# Estágio 2

Chegamos aos 512 bytes e precisamos de mais espaço (ainda tem espaço nos 512 bytes, mas vamos só seguir por hora). A solução é usarmos o estágio 1 pra pedir ao BIOS que leia mais setores do disco pra memória, e depois pular pra esse código que foi lido.

O serviço de leitura de disco do BIOS é a interrupção `int 0x13`, função `0x02`. Assim como o `int 0x10` usava o `AH` e `AL` como parâmetros, o `int 0x13` espera vários registradores configurados:

| Registrador | Valor | Significado |
| --- | --- | --- |
| `AH` | `0x02` | Função: ler setores |
| `AL` | quantidade | Quantos setores ler (cada setor tem 512 bytes) |
| `CH` | `0x00` | Cilindro 0 |
| `CL` | `0x02` | Setor 2 (o setor 1 é o boot sector, começa em 1 e não 0)
| `DH` | `0x00` | Cabeça 0 |
| `DL` | drive | N° do drive (o BIOS já deixa isso em `DL` quando entrega o controle)
| `ES:BX` | endereço | Onde na memória color

O endereçamento CHS (Cylinder-Head-Sector) é o sistema de endereçamento que os discos usavam antigamente. É confuso pela nomenclatura, mas pro nosso caso é simples: estamos no cilindro 0, cabeça 0, e queremos ler a partir do setor 2 (o setor 1 é o boot sector, que já está carregado).

O `DL` é um detalhe (bem elegante, inclusive): quando a BIOS pula pro boot sector, ela deixa o n° do drive de boot em `DL`, então a gente não precisa adivinhar se é `0x00` (disquete A), `0x80` (primeiro HD), etc. Basta usar o valor que já está lá.

## Stack

Até agora a gente não configurou a stack, e deu certo porque não fizemos nada que precisasse dela. Mas agora que vamos fazer chamadas mais complexas, é essencial configurarmos a stack, setando dois registradores:

- `SS` (Stack Segment) - O segmento base da stack
- `SP` (Stack Pointer) - O topo da stack (offset)

A stack em x86 cresce **pra baixo** (de endereços altos pra baixos). Então, se colocarmos `SP` em `0x7C00`, a stack vai crescer de `0x7C00` pra baixo (`0x7BFF`, por exemplo). É uma convenção comum em bootloaders.

Além disso, as partes novas que usamos foram:

- `call` e `ret`: até agora usamos `jmp` pra pular. O `call` faz a mesma coisa, mas antes ele empurra o endereçamento de retorno na stack, já `ret` pega esse endereço da stack e pula de volta. É literalkmente uma chamada de função.
- `.done`: o ponto antes do nome denota uma label local, vinculado ao label pai (`print`). É como um escopo, você pode escrever várias labels com o mesmo nome em diferentes funções, e elas não vão conflitar.
- `13, 10` logo depois da string: são o `\r` e `\n` (carriage return + line feed). Em modo real, o BIOS precisa dos dois pra pular de linha.
- `jc disk_error`: o `jc` é o `jump if carry`. O `int 0x13` seta o carry flag se a leitura falhou, equivale a verificar um código de erro depois de uma chamada de função.

## Por que sair do modo real?

Modo real é o modo de compatibilidade com o 8086 de 1978. As limitações são sérias:

- **1 MB de memória endereçável**: o esquema de segmento * 16 + offset só alcança 20 bits de endereço, o que dá 1 MB. Mesmo que você tenha 8 GB de RAM, não consegue acessar.
- **Sem proteção de memória**: qualquer código pode ler e escrever em qualquer endereço, incluindo sobreescrever o próprio kernel. Um programa bugado destrói tudo.
- **16 bits**: registradores de 16 bits, operações de 16 bits. Limitante pra qualquer coisa mais complexa.

Modo protegido, introduzido pelo 386, resolve tudo isso: registradores de 32 bits, até 4 GB de memória endereçável, e um sistema de proteção onde a CPU impede que código de usuário acesse memória do kernel. É por isso que se chama "protegido", a CPU protege regiões de memória baseado em níveis de privilégio.

### O que precisa acontecer pra trocarmos pro modo protegido?

A transição tem 3 passos, e todos precisam acontecer numa sequência específica:

**1. Desabilitar interrupções**: durante a troca, a CPU está num modo inconsistente. Se uma interrupção chegar no meio do processo, ele "crasha". A instrução `cli` (Clear Interrupt Flag) desabilita as interrupções.
**2. Configurar a GDT (Global Descriptor Table)**: é uma tabela na memória que descreve os segmentos de memória que o processador pode acessar. Em modo protegido, toda referência à memória passa pela GDT. A CPU precisa saber onde essa tabela está antes de trocar de modo.
**3. Setar o bit PE**: o registro `CR0` (Control Register 0) da CPU tem um bit chamado PE (Protection Enable). Quando você seta esse bit pra 1, a CPU entra em modo protegido. Depois disso, o modo real acaba: as interrupções da BIOS param de funcionar, e o "mundo" muda completamente.

## Global Descriptor Table (GDT)

> Essa é a parte mais densa dessa etapa, então vou tentar me aprofundar um pouco mais pra garantir que eu entendi.

Em modo real, os segmentos eram simples: `segmento * 16 + offset = endereço físico`. Em modo protegido, os registrados de segmento (`CS`, `DS`, `SS`) não guardam mais um endereço base: eles guardam um **seletor**, que é basicamente um índice pra uma entrada na GDT. A CPU pega esse índice, vai na GDT, lê a entrada correspondente, e ali vai encontrar: o endereço base do segmento, o tamanho (limit), e as permissões (leitura, escrita, execução, nível de privilégio).

Nós precisaremos de uma GDT mínima com pelo menos 3 entradas:

| Índice | Tipo | Descrição |
| Entrada 0 | Null Descriptor | A especificação Intel exige que a primeira entrada seja toda zeros. Se algum registrador de segmento apontar pro índice 0 por acidente, a CPU gera uma exceção em vez de acessar memória inválida aleatória. |
| Entrada 1 | Code Segment | Descreve o segmento onde código executável vive. Base = `0x00000000` (8x0s), limit = `0xFFFFF` com granularidade de 4KB = cobre os 4GB inteiros. Permissão: executável e legível |
| Entrada 2 | Data Segment | Descreve o segmento de dados (stack, variáveis, tudo que não é código). Mesma base e limit, mas permissão: legível, gravável, não executável. |

Repare que ambos cobrem os 4GB inteiros e começam em zero. Isso se chama **flat memory model**: os segmentos se sobrepõem completamente, e na prática o endereço lógico é igual ao endereço físico. A segmentação existe porque a CPU obriga, mas a gente neutraliza ela com uma GDT simples. A proteção real de memória vai vir depois com paginação, que é um mecanismo muito mais flexível.

> ⚠️ Os descritores da GDT têm um formato binário bem específico: 8 bytes por entrada, com campos de bits distribuídos de forma não intuitiva (o limit e a base são divididos em pedaços não contíguos). Isso é herança da evolução do x86 e é um dos pontos mais confusos. Preciso pesquisar por "GDT entry format osdev" pra entender melhhor. Talvez na OSDev Wiki tenha algo.

# Construindo um sistema operacional

> Olha... audacioso eu diria (?)

A tentação quando se cria um sistema operacional é pensar nele como "a interface gráfica do computador", mas a essência é outra (obrigado professor da faculdade, aprendi bastante sobre isso). Um OS é uma camada de **abstração e multiplexação de hardware**. Ele resolve dois problemas fundamentais:

1. Esconde a complexidade (bizarra) do hardware atrás de interfaces limpas (você não precisa programar o controaldor de disco toda vez que quer gravar um arquivo, por exemplo).
2. Permite que múltiplos programas compartilhem os mesmos recursos físicos (CPU, memória, disco), sem se destruírem mutuamente.

Pra isso, o OS precisa de pelo menos:

- Gerenciamento de memória (quem pode acessar o quê)
- Escalonamento de processos (quem roda quando)
- Tratamento de interrupções (como o hardware avisa que algo aconteceu)
- Alguma interface I/O (teclado, tela, disco, etc)

Que parcialmente se conecta com o que já fizemos de projeto de estudos brincando antes, como o [Marselo](https://github.com/caiodomingues/marselo) (uma VM gerenciando uma call stack e alocando memória, uma simplificação do que o kernel faz pra cada processo) e com o [OrlonDB](https://github.com/caiodomingues/orlondb) (onde a gente lida com páginas e serialização, basicamente o que o filesystem faz).

Nos outros projetos, usei TypeScript por ser o que estava sendo comum no dia a dia, então estava mais fresco na cabeça, mas aqui não tem como fugir: o core do OS é **C + Assembly** (é bom que eu revejo excelentes conceitos de C), e tem motivo: nos primeiros estágios do boot, precisamos configurar registradores do processador, definir a Global Descriptor Table (GDT), trocar de modo de execução e manipular endereços de memória diretamente. Não temos esse runtime de TypeScript e nem sequer existe um alocador de memória. Assembly é necessário para o bootstrap inicial e para operações que o C não consegue expressar (como carregar registradores de segmento ou executar o `iret` para retornar de uma interrupção). C entra em seguida como a principal linguagem do kernel (poderia ter sido Rust também), porque te dá controle total sobre memória, mas sem muita cerimônia (como o Assembly); e nem vamos usar tanto Assembly assim.

Pra simplificar um pouco as coisas (e ter mais facilidade em achar conteúdo de pesquisa e material de apoio), vamos fazer um OS x86 32-bit, sem lidar com aquela camada extra de complexidade que o x86_64 adiciona sem um ganho didático proporcional (ou eu só to com preguiça mesmo), mas no fim estou seguindo a mesma lógica que o OrlonDB -> um DB KV pra não ter que fazer algo muito grande até ter um DB relacional etc.

Vamos fazer um bootloader próprio ao invés de usar o GRUB*, um código que a BIOS carrega do disco (o setor de boot são os primeiros 512 bytes), sobe de modo real 16-bit pra modo protegido 32-bit, carrega o kernel do disco pra memória e pula pra ele.

>*Mas Caio, não era pra simplificar?
>É, mas ai perderíamos muito conteúdo de boot, modo protegido, modo real e etc. Qualquer coisa a gente muda pro GRUB depois (spoiler: provavelmente não vai hahahaha)

A vantagem é que vamos entender cada byte do procesos de boot, a desvantagem é que é um projeto dentro do projeto e que tem muita particularidade chata: limite de 512 bytes no primeiro setor, lidar com A20 line, ler setores do discor via interrupções de BIOS em modo real.

## Ambiente de Desenvolvimento

Precisamos de uma toolchain, no TypeScript era simples como usar o `node`, `tsc`, `npm` e etc, mas aqui precisamos de algo para **cross-compilation**: gerar binários que vão rodar num x86 bare-metal, sem OS por baixo. Então temos um pequeno quebra-cabeças:

1. **Cross-compiler (GCC)**: Precisamos de um GCC configurado para gerar código pra um target genérico, geralmente `i686-elf` (ELF = Executable and Linkable Format, padrão de sistemas Unix-like). O motivo de não usarmos o GCC "normal" da máquina é que ele vem configurado pra linkar com a libc do OS, incluir os headers do sistema e etc, mas não existe nada disso no PoppyOS. O cross-compiler gera código "freestanding" (sem dependência de nenhum OS).
2. **Assembler (NASM)**: NASM (Netwide Assembler) é o assembler mais usado pra hobby OS em x86, existem alternativas como o GAS (assembler do GNU, FASM), mas o NASM tem a sintaxe mais legível pra quem está aprendendo (como eu), usando a sintaxe da Intel (`mov eax 1`) em vez da sintaxe da AT&T (`movl $1 %eax`), que notoriamente é menos intuitiva.
3. **Linker(ld)**: Vem junto com o cross-compiler GCC (como `i686-elf-ld`). O linker junta todos os arquivos objeto (`.o`) num binário final, e vamos precisar escrever um linker script (um arquivo que diz exatamente _onde_ na memória cada seção do kernel vai ficar).
4. **Emulador (QEMU)**: QEMU vai emular um PC x86 completo. A gente roda `qemu-system-i386` apontando pro binário e ele simula um boot inteiro: BIOS, carregamento do disco e etc. A grande vantagem é um ciclo de feedback rápido e menos frustrante do que ficar queimando uma máquina real toda hora (e é mais seguro também).

O que temos até agora é:

| Tópico | Escolha |
| --- | --- |
| Arquitetura | x86 32-bit (i386) |
| Assembler | NASM (sintaxe Intel) |
| Compilador C | GCC cross-compiler (`i686-elf`) |
| Emulador | QEMU (`qemu-system-i386`) |
| Bootloader | Próprio (pelo menos no começo) |

## O que a CPU encontra quando liga

Quando a energia chega (literal mesmo, no sentido físico), o processador x86 sempre começa no mesmo estado: ele acorda em **real mode** (modo real), que é o modo de operação original do Intel 8086 (iAPX 86) de 1978. Nesse modo, a CPU se comporta como se fosse um processador de 16 bits com acesso a apenas 1MB de memória. Não importa se você tem um i9 com 64GB de RAM, nos primeiros milissegundos você "está em 1978".

A primeira coisa que a CPU faz é buscar uma instrução no endereço `0xFFFFFFF0` (são 7 Fs), ou seja, o topo da memória endereçável. Esse endereço aponta pra uma ROM na placa-mãe que contém o BIOS (Basic Input/Output System) ou o UEFI (Unified Extensible Firmware Interface) em máquinas mais modernas. No nosso caso, vamos trabalhar com BIOS porque o QEMU emula perfeitamente (e eu to achando conteúdo mais fácil também).

O BIOS faz o trabalho sujo inicial: testa a RAM, detecta dispositivos, inicializa o básico. Depois ele precisa encontrar algo pra bootar, e aqui temos um contrato simples e antigo.

### O contrato do boot: o MBR

O BIOS olha pros dispositivos do boot (disco rígido, USB, etc) e lê os primeiros 512 bytes do disco. Esse setor é o MBR (Master Boot Record). O BIOS verifica uma coisa só: os dois últimos bytes desse setor precisam ser `0x55` e `0xAA`. Essa é a assinatura de boot, se estiver lá, o BIOS assume que é código executável, carrega esses 512 bytes no endereço `0x7C00` da memória e pula a execução pra lá.

Isso significa que os primeiros 512 bytes do discos são o programa. Não tem loader, não tem runtime, não tem nada. A CPU começa a executar o que estiver ali, em modo real 16-bit, e o que acontecer a partir daí, bom... é problema seu xD

512 bytes é quase nada, deve dar umas 100~200 instruções Assembly no máximo e olhe lá. Por isso a maioria dos OS usa um modelo de dois estágios:

1. O primeiro estágio cabe nos 512 bytes iniciais e a única função dele é carregar o próximo estágio do disco pra memória.
2. O segundo estágio, por sua vez, tem espaço pra fazer o trabalho pesado que for preciso: configurar o modo protegido, carregar o kernel, etc.

## O mapa da memória em modo real

Quando o BIOS te entrega o controle, a memória está organizada assim (de forma BEM simplificada):

```plaintext
0x00000 ~ 0x003FF   Interrupt Vector Table (IVT), tabela de interrupções do BIOS
0x00400 ~ 0x004FF   BIOS Data Area
0x00500 ~ 0x07BFF   Livre, você pode usar
0x07C00 ~ 0x07DFF   Seu bootloader (512 bytes, carregado pelo BIOS)
0x07E00 ~ 0x7FFFF   Livre, você pode usar
0x80000 ~ 0x9FFFF   Parcialmente livre (depende do hardware)
0xA0000 ~ 0xBFFFF   Memória de vídeo (VGA)
0xC0000 ~ 0xFFFFF   ROMs do BIOS e de dispositivos
```

Note que o IVT (Interrupt Vector Table) fica lá no começo. É uma tabela de 256 entradas com endereços de handlers de interrupção, tanto de hardware (teclado, timer) quanto de software que o BIOS oferece como serviço. Em modo real, você usa essas interrupções do BIOS pra fazer coisas como ler setores do disco (`int 0x13`) e imprimir caracteres na tela (`int 0x10`). Quando a gente sair do modo real pra modo protegido, a IVT deixa de funcionar e perdemos o acesso a esses serviços (outra razão pra fazer certas coisas, como carregar o kernel do disco ainda em modo real).

## Registradores que importam agora

Em modo real 16-bit, os registradores principais que vamos usar são:

- **Registradores gerais (16-bit)**: `AX`, `BX`, `CX` e `DX`, cada um pode ser dividido em parte alta e baixa (`AH`/`AL`, etc). Usados pra dados, contadores, parâmetros de interrupções do BIOS.
- **Registradores de segmento**: `CS` (Code Segment), `DS` (Data Segment), `SS` (Stack Segment), `ES` (Extra Segment). Em modo real, a memória é acessada como `segment * 16 + offset`. Quando o BIOS pula pro `0x7C00`, o `CS` pode estar em `0x0000` com offset `0x7C00`, ou `0x07C0` com offset `0x0000`, o que dá o mesmo endereço físico. Pegadinha clássica de bootloader, inclusive.
- **Registrador de pilha**: `SP` (Stack Pointer), ponteiro da stack. Precisamos configurar isso logo no início, porque sem stack não dá pra chamar funções nem usar `push`/`pop`. `IP` (Instruction Pointer), endereo da próxima instrução. Não vamos escrever nele diretamente, ele avança sozinho ou muda com `jmp`/`call`.

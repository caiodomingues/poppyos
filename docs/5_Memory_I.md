# Gerenciamento de Memória

No contexto atual, nosso OS usa um **flat memory model**: qualquer código pode acessar qualquer endereço de 0 a 4GB. Não tem proteção nenhuma, se o shell tiver um bug e escrever no endereço errado, pode sobreescrever a IDT, a GDT, ou o próprio kernel.

A paginação resolve isso, a ideia é que a CPU mantenha uma tabela que traduz endereços virtuais (o que o código vê) pra endereços físicos (o que realmente existe na RAM). Quando o código acessa o endereço `0x8000`, a CPU consulta a tabela e descobre que esse endereço virtual coreresponde a um endereço físico (como `0x50000` ou outro qualquer). Isso nos dá uns poderes legais:

- **Isolamento**: cada processo pode ter seu próprio mapeamento. O processo A acha que tem memória de `0x0` a `0xFFFFFFFF` todinha pra ele, mas na real está mapeado pra um pedaço pequeno da RAM. O processo B tem outro mapeamento. Nenhum dos dois consegue ver a memória do outro.
- **Proteção de memória**: cada página pode ter permissões: leitura, escrita, execução, e nível de privilégio (kernel vs user). Se código do usuário tentar escrever numa página do kernel, a CPU gera uma Page Fault (exceção 14, veja na IDT).
- **Memória virtual**: podemos mapear mais memória do que fisicamente existe, usando swap pra disco. Mas isso é um pouco mais avançado, não vamos lá agora.

## Como funciona no x86

O x86 usa um esquema de dois níveis:

- **Page Directory**: tem 1024 entradas, cada uma aponta pra uma Page Table e fica num bloco de 4096 bytes alinhado.
- **Page Table**: cada uma tem 1024 entradas, cada entrada aponta pra uma página física de 4096 bytes.

A matemática é simples: 1024 entradas na Page Directory \* 1024 entradas na Page Table \* 4096 bytes por página = 4GB de espaço virtual. Cobre todo o espaço de endereçamento de 32 bits.

Quando a CPU precisa traduzir um endereço virtual, ela divide ele em três partes:

```plaintext
Endereço virtual: 0xABCDE123
  
  Bits 31-22 (10 bits) → índice no Page Directory  = 0x2AF
  Bits 21-12 (10 bits) → índice na Page Table      = 0xDE
  Bits 11-0  (12 bits) → offset dentro da página   = 0x123
```

A CPU vai no Page Directory, pega a entrada `0x2AF`, que aponta pra uma Page Table. Nessa Page Table, pega a entrada `0xDE`, que aponta pro endereço físico da página. Soma o offset `0x123` e tem o endereço físico final.

O registrador CR3 guarda o endereço físico do Page Directory. Quando quisermos trocar o mapeamento (ex: context switch entre processos), basta mudar o CR3.

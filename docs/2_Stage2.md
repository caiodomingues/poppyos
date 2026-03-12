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

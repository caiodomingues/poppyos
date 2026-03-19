# Sistema de arquivos: ATA PIO, leitura e escrita de setores

O controlador ATA (o chip que gerencia HDs) é acessado via portas de I/O, igual o PIC e o PIT. O modo PIO (Programmed I/O) é o mais simples: a CPU transfere os dados byte a byte. Lento, mas perfeito pro nosso caso.

Vamos usar um disco separado pro filesystem (a floppy é o boot). O QEMU aceita múltiplos drives.

## Driver ATA

A comunicação com o controlador ATA acontece através de 8 portas de I/O (0x1F0 a 0x1F7). O fluxo pra ler um setor:

1. Espera o drive ficar pronto (bit busy do status register limpar)
2. Envia o endereço do setor em formato LBA (Logical Block Addressing) nas portas de endereço
3. Envia o comando de leitura (0x20) na porta de comando
4. Espera os dados ficarem prontos (bit DRQ do status register setar)
5. Lê 256 words de 16 bits (= 512 bytes = 1 setor) da porta de dados

A escrita é o processo inverso: envia o comando 0x30, espera DRQ, escreve 256 words, e depois envia um comando de flush (0xE7) pra garantir que os dados saíram do cache do controlador pro disco.

Um detalhe importante: o ATA transfere dados em words de 16 bits, não em bytes individuais. Por isso as funções `port_in16`/`port_out16` são necessárias, usando as instruções x86 `inw`/`outw` em vez de `inb`/`outb`.

## Filesystem

O layout no disco é intencionalmente simples:

- **Setor 0**: superblock (magic number, número de arquivos)
- **Setores 1-2**: file table (16 entries de 64 bytes = 1024 bytes = 2 setores)
- **Setores 3-18**: dados dos arquivos (1 setor por arquivo, 512 bytes max)

### Superblock

O superblock é o primeiro setor do disco e contém metadados do filesystem. O campo mais importante é o **magic number** (`0x504F5059`, que é "POPY" em ASCII). Na inicialização, o kernel lê o superblock e checa o magic: se bater, o disco já tem um filesystem válido e os dados são carregados. Se não bater, o disco é considerado não formatado e um filesystem novo é criado.

Essa é a mesma lógica que o OrlonDB usa no header do arquivo de dados: um magic number pra identificar se o formato é válido.

### File Table

A file table é um array de 16 entries, cada uma com 64 bytes: 32 bytes pro nome do arquivo, 4 bytes pro tamanho, 1 byte de flag (usado/livre), e 27 bytes de padding pra alinhar em 64. O índice da entry na tabela também define qual setor do disco guarda os dados daquele arquivo (setor 3 + índice).

### Operações

- **write \<name\> \<content\>**: encontra um slot livre na file table, copia o nome e os dados, escreve o setor de dados no disco, e atualiza o superblock.
- **cat \<name\>**: busca o arquivo pelo nome na file table, lê o setor de dados correspondente, e imprime na tela.
- **rm \<name\>**: marca o slot como livre e decrementa o contador de arquivos no superblock.
- **ls**: percorre a file table e imprime os nomes dos arquivos que estão marcados como usados.

Todas as operações de escrita (create, delete) chamam `fs_save()` no final, que serializa o superblock e a file table de volta pro disco. Isso garante que se o sistema for reiniciado, os dados permanecem. É durabilidade no nível mais básico, sem WAL, sem journaling, sem transações, mas funcional.

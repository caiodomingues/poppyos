# Kernel

> Chegamos no ponto onde começamos a escrever C. 

O que precisa acontecer agora, no fluxo do boot, é:

```plaintext
BIOS -> Stage 1 (ASM) -> Stage 2 (ASM) -> kernel_main() (C)
```

O stage 2 em Assembly faz o setup que C não consegue fazer (GDT, modo protegido), e depois chama uma função C. A partir daí, Assembly só vai aparecer quando for estritamente necessário. Mas pra isso funcionar, vamos precisar de 3 coisas novas:

1. **Um entry point em Assembly que chama o C**: vamos chamar de `kernel_entry.asm`. Ele é mínimo, só chama o `kernel_main` e trava se a função retornar.
2. **O kernel em C**: `kernel.c` com a função `kernel_main()`.
3. **Um linker script**: diz pro linker como organizar o binário final: onde o código começa, onde os dados ficam, qual é o entrypoint e etc.

## Linker Script

Resumo simples do que é o `linker.ld`:

- `ENTRY(kernel_main)`: define o ponto de entrada do binário.
- `. = 0x1000`: diz que o código começa no endereço `0x1000` da memória, o `.` é o "location counter", a posição atual na memória.
- `.text`: seção de código executável (funções)
- `.rodata`: dados somente de leitura (strings, constantes)
- `.data`: dados inicializados (variáveis globais com valor)
- `.bss`: dados não inicializados (variáveis globais zeradas). A CPU sabe que essa seção deve ser zerada na inicialização.

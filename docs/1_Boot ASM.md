# Iniciando com o boot.asm

Assembly bare-metal é um mundo diferente pra mim, então vamos devagar:

Se a BIOS carrega 512 bytes e pula pro início, então o nosso executável precisa estar no começo do arquivo (obviamente), que é a primeira coisa que a CPU vai ler e executar. E a assinatura `0x55AA` precisa estar nos bytes 511 e 512 (os dois últimos), porque é lá que o BIOS confere. Então a estrutura é basicamente:

```plaintext
[ nosso código ][ espaço vazio preenchido com zeros se necessário ][ 0x55 0xAA]
```

O espaço vazio existe só pra garantir que o arquivo tenha exatamente 512 bytes, se o nosso código for menor que isso. Se o código for maior, ai você tem um problema :/

Em NASM, os conceitos que te permitem mostrar isso são 3:

1. `org` - Diz ao assembler que o código vai ser carregado no endereço `0x7C00`. Ele não move nada, só ajusta os cálculos de endereço internos pra levar isso em conta.
2. `times` - Repete uma instrução N vezes. É como preenchemos o espaço vazio.
3. `db`/`dw` - "define byte" e "define word". É como escrevemos dados brutos no arquivo (incluindo a assinatura).

E pra imprimir algo na tela estando em modo real, usamos a interrupção `int 0x10` do BIOS, especificamente a função `0x0E` do serviço de vídeo, que imprime um caractere. O setup das coisas é: coloca `0x0E` em `AH` (indica a função), o caractere ASCII em `AL`, e chama `int 0x10`.

## Assembly (NASM)

Pra comparar com o que tá mais recente na minha cabeça por hora (TypeScript), quando escrevemos `let x = 1 + 2`, o compilador decide onde guardar x, como somar e etc. Em Assembly, a gente tem que decidir tudo, então eu preciso me situar:

```nasm
mov ax, 42      ; ax agora vale 42
mov bx, ax      ; bx agora vale 42 (copiou de ax)
```

O formato é sempre `mov destino, origem`: ao contrário do que é mais comum e intuitivo, o destino vem primeiro.

- Instrução `int` dispara uma interrupção de software, é como chamar uma função do BIOS:

```nasm
int 0x10        ; chama o serviço de vídeo do BIOS
```

O BIOS sabe o que fazer baseado nos valores que colocarmos nos registradores antes de chamar o `int`. É como se os registradores fossem os parâmetros da função.

- Instrução `hlt` para o processador:

```nasm
hlt             ; para a CPU, nada mais acontece depois disso
```

- Instrução `jmp` é como um `goto`, pula pra um endereço específico:

```nasm
jmp $           ; pula pra si mesmo, criando um loop infinito
```

O `$` em NASM significa "o endereço da instrução atual", então `jmp $` é um loop infinito porque sempre pula pra si mesmo, prendendo a CPU. Útil quando quisermos parar o sistema sem correr o risco do processador sair executando lixo da memória depois do `hlt` (porque uma interrupção pode acordar a CPU de um `hlt`).

### Diretivas

Não são instruções, são comandos pro assembler:

```nasm
org 0x7C00      ; esse código vai ser carregado em 0x7C00

db 0x55         ; escreve o byte 0x55 no arquivo
dw 0xAA55       ; escreve a word (2 bytes) 0xAA55

times 10 db 0   ; escreve 10 bytes de valor 0

; $ = endereço atual
; $$ = endereço do começo da seção
; então ($ - $$) = quantos bytes de código você já escreveu
```

### Labels

São nomes que marcam posições no código, como um bookmark:

```nasm
inicio:
    mov ax, 42
    jmp inicio   ; volta pro mov ax, 42
```

### Como imprimir um caractere

O serviço `int 0x10` com função `0x0E` imprime um caractere no modo "teletype" (avança o cursor automaticamente), assim:

```nasm
mov ah, 0x0E    ; função 0x0E = teletype output
mov al, 'P'     ; caractere 'P' em ASCII
int 0x10        ; chama o serviço de vídeo do BIOS (P aparece na tela)
```

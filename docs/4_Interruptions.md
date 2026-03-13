# Interrupções

> Esse passo também é grande, mas é bem importante pois cria a primeira interação usuário <> sistema

No momento atual, o sistema é passivo, o PoppyOS imprime texto e trava. Ele não reage a nada, apertar uma tecla não resulta em nenhuma ação ou efeito e o kernel sequer sabe o que está acontecendo no mundo externo (eu queria ser como o kernel às vezes).

Interrupções resolvem isso. São sinais que chegam na CPU dizendo "para o que está fazendo e trata esse evento". Existem três tipos:

1. **Exceções**: geradas pela própria CPU quando algo dá errado. Divisão por zero, acesso a memória inválida, instrução ilegal.
2. **Interrupções de hardware (IRQs)**: geradas por dispositivos externos. O teclado manda uma IRQ quando uma tecla é pressionada. O timer do sistema manda uma IRQ a cada poucos milissegundos. O disco manda uma IRQ quando termina uma leitura.
3. **Interrupções de software**: geradas intencionalmente pelo código com a instrução `int`. É como as interrupções da BIOS (`int 0x10`, `int 0x13`) que você usou no bootloader (as syscalls do PoppyOS vão funcionar assim)

## O mecanismo

Em modo protegido, quando uma interrupção chega, a CPU faz o seguinte:

1. Para a instrução atual
2. Salva o estado (registradores, flags, endereço de retorno) na stack
3. Consulta a IDT (Interrupt Descriptor Table) pra encontrar o handler daquela interrupção
4. Pula pro handler
5. Quando o handler termina (com `iret`), a CPU restaura o estado e continua de onde parou

A IDT é pra interrupções o que a GDT é pra segmentos: uma tabela na memória que a CPU consulta. Ela tem 256 entradas (uma pra cada número de interrupção, de 0 a 255). Cada entrada aponta pro endereço de uma função handler.

## O PIC (Programmable Interrupt Controller)

Tem um detalhe de hardware importante. Os dispositivos não mandam interrupções direto pra CPU, eles passam pelo PIC, um chip que fica entre os dispositivos e o processador. O PC original tinha um PIC, o IBM AT adicionou um segundo, totalizando 15 linhas de IRQ. No setup padrão da BIOS:

- IRQ 0-7: interrupções 8-15 (PIC master)
- IRQ 8-15: interrupções 70-77 (PIC slave)

O problema é: as interrupções 8-15 conflitam com as exceções da CPU (que usam os números 0-31). Divisão por zero é a exceção 0, page fault é a 14 e o timer seria a IRQ 0 mapeada na interrupção 8, que conflita com a exceção 8 (Double Fault). Então a primeira coisa que precisamos fazer é remapear o PIC pra mover as IRQs pra fora da faixa 0-31. A convenção comum é mapear:

- IRQ 0-7: interrupções 32-39
- IRQ 8-15: interrupções 40-47

## A estrutura de arquivos

- `idt.h` e `idt.c`: define a estrutura da IDT e a função pra registrar handlers
- `isr.h` e `isr.c`: handlers para as exceções (0-31)
- `isr.asm`: stubs em Assembly que salvam os registradores e chamam o handler em C (a CPU não salva todos os registradores automaticamente, então precisamos fazer isso manualmente com um wrapper)
- `pic.h` e `pic.c`: remapeamento e controle do PIC.

> Pro idt, vamos precisar de uma instrução "especial" pra carregar a IDT na CPU: `asm volatile("lidt %0" : : "m"(idt_ptr));`. Essa sintaxe é inline assembly do GCC, o `"lidt %0"` é a instrução, o `: : "m"(idt_ptr)` diz que "o operando `%0` é a variável `idt_ptr` na memória. O `volatile` impede o compilador de otimizar a instrução fora do lugar, o que é importante porque ela tem efeitos colaterais que o compilador não conhece, além de evitar remoções acidentais pela otimização.

## Uma parte meio chata: ISR stubs em Assembly

O problema é o seguinte: quando a CPU chama um handler de interrupção, ela salva na stack apenas `EIP`, `CS` e `EFLAGS`. Os registradores gerais (`EAX`, `EBX`, `ECX`, etc) **não são salvos**. Se o handler em C usar esses registradores (e vai), o código que foi interrompido volta com registradores corrompidos.

A solução é um wrapper em Assembly pra cada interrupção que faz `pusha` (salva todos os registradores), chama o handle C, faz `popa` (restaura), e retorna com `iret`.

> Um stub de ISR em Assembly atua como uma "ponte" ou intermediário entre o hardware (que gerou a interrupção) e o manipulador principal da interrupção (geralmente a linguagem de alto nível, como C no nosso caso)

Os **Macros NASM** (`%macro`/`%endmacro`) gera código reptitivo. `ISR_NO_ERR_0` expande pra uma função `isr0` que empurra um error code falso (0) e o número da interrupção (0) na stack. Algumas exceções (como page fault, double fault) já empurram um error code automaticamente, por isso tem duas macros.

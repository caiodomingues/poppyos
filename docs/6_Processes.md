# Processos e Context Switch

Até agora o PoppyOS roda uma única thread de execução: o `kernel_main` que cai no loop `hlt`. Tudo é sequencial, mas um OS real precisa rodar múltiplos programas "ao mesmo tempo", e faz isso alternando rapidamente entre eles, dezenas de vezes por segundo.

Cada processo precisa de:

- **Estado da CPU**: os registradores (EAX, EBX, etc., EIP, ESP, EFLAGS). Quando você pausa um processo pra rodar outro, precisa salvar todos esses valores. Quando volta, restaura. Isso é o context switch.
- **Stack própria**: cada processo precisa da sua própria stack, senão um sobreescreve os dados do outro.
- **Metadados**: um ID, o estado (rodando, pronto, bloqueado), ponteiro pra stack.

A struct que guarda tudo isso se chama **PCB (Process Control Block)**, ou em kernels menores, simplesmente "task".

## Context Switch (o conceito)

Imagina dois processos A e B. O processo A está rodando. O timer (IRQ 0) dispara. O handler:

1. Salva os registradores de A na stack de A
2. Salva o ponteiro da stack de A no PCB de A
3. Pega o ponteiro da stack de B do PCB de B
4. Restaura os registradores de B da stack de B
5. Retorna, e agora B está rodando

Do ponto de vista de cada processo, nada aconteceu — ele "dormiu" e "acordou" com tudo intacto. A troca é invisível.

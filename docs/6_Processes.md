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

Do ponto de vista de cada processo, nada aconteceu, ele "dormiu" e "acordou" com tudo intacto. A troca é invisível.

## Cooperative vs Preemptive

O PoppyOS implementa os dois modelos:

**Cooperative scheduling**: cada task chama `task_yield()` voluntariamente pra dar vez à próxima. O problema é que se uma task travar num loop infinito sem chamar yield, o sistema inteiro congela. É simples de implementar, mas frágil.

**Preemptive scheduling**: o timer (IRQ 0) chama `schedule()` automaticamente a cada tick. Mesmo que uma task entre num loop infinito, o timer interrompe ela e troca pra próxima. É o modelo que OSes reais usam. O PoppyOS roda o scheduler a 100 Hz (100 trocas potenciais por segundo).

## O Context Switch em Assembly

O coração do multitasking são 6 instruções em Assembly (`switch.asm`):

```asm
context_switch:
    pusha               ; salva todos os registradores na stack atual
    mov eax, [esp + 36] ; pega o ponteiro pra old_esp
    mov [eax], esp      ; salva o ESP atual
    mov eax, [esp + 40] ; pega o new_esp
    mov esp, eax        ; TROCA DE STACK
    popa                ; restaura registradores da nova stack
    ret                 ; pula pro EIP da nova stack
```

O `pusha` salva 8 registradores (32 bytes). O `popa` restaura. A troca do `esp` é o momento em que a CPU muda de uma task pra outra. Quando `ret` executa, ele pega o endereço de retorno da nova stack, que na primeira execução de uma task é o `entry` (ponto de entrada da função), e nas vezes seguintes é o ponto onde `task_yield` ou o timer interrompeu.

## Criação de Tasks

Quando `task_create(entry)` é chamado, o kernel monta a stack da nova task como se ela já tivesse sido interrompida por um context switch. Isso significa empilhar valores iniciais pra todos os registradores (zeros) e o endereço de `entry` como endereço de retorno. Quando o scheduler escolher essa task pela primeira vez, o `popa` restaura os zeros e o `ret` pula direto pro `entry`.

## Round-Robin

O scheduler usa o algoritmo mais simples possível: round-robin. Percorre o array de tasks circularmente e escolhe a próxima que estiver com status `TASK_READY`. Não tem prioridade, não tem quantum variável. Cada task recebe a mesma fatia de tempo (1 tick do timer = 10ms a 100 Hz). Simples, justo, e suficiente pro PoppyOS.

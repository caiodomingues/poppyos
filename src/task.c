#include "task.h"

static struct task tasks[MAX_TASKS];
static int current_task = 0;
static int num_tasks = 0;

// Defined at switch.asm
extern void context_switch(uint32_t *old_esp, uint32_t new_esp);

void task_init(void)
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].status = TASK_UNUSED;
    }

    // Task 0 is kernel task, always running
    tasks[0].id = 0;
    tasks[0].status = TASK_RUNNING;
    current_task = 0;
    num_tasks = 1;
}

int task_create(void (*entry)(void))
{
    // Find a free slot
    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].status == TASK_UNUSED)
        {
            slot = i;
            break;
        }
    }

    if (slot == -1)
        return -1; // no slots available

    tasks[slot].id = slot;
    tasks[slot].status = TASK_READY;

    // Set up the stack to look like context_switch saved it.
    // When popped, it will restore zeros in all registers.
    // When ret is executed, it will jump to entry.
    uint32_t *sp = (uint32_t *)((uint32_t)tasks[slot].stack + TASK_STACK_SIZE);

    *(--sp) = (uint32_t)entry; // return address (where ret will jump)
    *(--sp) = 0;               // EAX
    *(--sp) = 0;               // ECX
    *(--sp) = 0;               // EDX
    *(--sp) = 0;               // EBX
    *(--sp) = 0;               // ESP (ignored by popa)
    *(--sp) = 0;               // EBP
    *(--sp) = 0;               // ESI
    *(--sp) = 0;               // EDI

    tasks[slot].esp = (uint32_t)sp;
    num_tasks++;

    return slot;
}

void task_yield(void)
{
    int prev = current_task;

    // Round-robin: find the next READY task
    int next = current_task;
    for (int i = 0; i < MAX_TASKS; i++)
    {
        next = (next + 1) % MAX_TASKS;
        if (tasks[next].status == TASK_READY)
            break;
    }

    if (next == current_task)
        return; // no task to switch to

    tasks[prev].status = TASK_READY;
    tasks[next].status = TASK_RUNNING;
    current_task = next;

    context_switch(&tasks[prev].esp, tasks[next].esp);
}

void schedule(void)
{
    int prev = current_task;

    int next = current_task;
    for (int i = 0; i < MAX_TASKS; i++)
    {
        next = (next + 1) % MAX_TASKS;
        if (tasks[next].status == TASK_READY)
            break;
    }

    if (next == current_task)
        return;

    tasks[prev].status = TASK_READY;
    tasks[next].status = TASK_RUNNING;
    current_task = next;

    context_switch(&tasks[prev].esp, tasks[next].esp);
}

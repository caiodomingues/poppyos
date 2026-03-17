#ifndef TASK_H
#define TASK_H

#include "types.h"

#define MAX_TASKS 8
#define TASK_STACK_SIZE 4096

enum task_status
{
    TASK_UNUSED,
    TASK_READY,
    TASK_RUNNING,
};

struct task
{
    int id;
    enum task_status status;
    uint32_t esp;                   // stack pointer stored
    uint8_t stack[TASK_STACK_SIZE]; // proprietary stack
};

void task_init(void);
int task_create(void (*entry)(void));
void task_yield(void);
void schedule(void);

#endif
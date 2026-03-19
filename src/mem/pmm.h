// pmm stands for Physical Memory Manager
#ifndef PMM_H
#define PMM_H

#include "types.h"

void pmm_init(void);
uint32_t pmm_alloc(void);          // Returns the physical address of a free page
void pmm_free(uint32_t address);   // Marks the page as free

#endif
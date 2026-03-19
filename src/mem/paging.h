#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGE_SIZE 4096
#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024

// flags (bits 0-2)
#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4

void paging_init(void);

#endif
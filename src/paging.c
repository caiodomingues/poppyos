#include "paging.h"

static uint32_t page_directory[TABLES_PER_DIR] __attribute__((aligned(PAGE_SIZE)));
static uint32_t first_table[PAGES_PER_TABLE] __attribute__((aligned(PAGE_SIZE)));

void paging_init(void)
{
    // Clears the page directory
    for (uint32_t i = 0; i < TABLES_PER_DIR; i++)
    {
        page_directory[i] = 0; // Flags as not present
    }

    // Identity map of the first 4 MB (1024 pages of 4 KB)
    for (uint32_t i = 0; i < PAGES_PER_TABLE; i++)
    {
        first_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITE;
    }

    // First entry of the directory points to first_table
    page_directory[0] = (uint32_t)first_table | PAGE_PRESENT | PAGE_WRITE;

    // Load into CR3
    asm volatile("mov %0, %%cr3" : : "r"(page_directory));

    // Enables paging (bit 31 of CR0)
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}
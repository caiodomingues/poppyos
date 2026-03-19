#include "heap.h"
#include "pmm.h"
#include "paging.h"

struct block_header
{
    uint32_t size;             // block size (without counting the header)
    uint8_t is_free;           // 1 = free, 0 = in use
    struct block_header *next; // next block in the list
};

static struct block_header *free_list = NULL;

void heap_init(void)
{
    free_list = (struct block_header *)pmm_alloc();
    free_list->size = PAGE_SIZE - sizeof(struct block_header);
    free_list->is_free = 1;
    free_list->next = NULL;
}

void *kmalloc(uint32_t size)
{
    struct block_header *current = free_list;
    struct block_header *previous = NULL;

    while (current != NULL)
    {
        if (current->is_free && current->size >= size)
        {
            // Found a suitable block
            if (current->size > size + sizeof(struct block_header))
            {
                // Split the block
                struct block_header *new_block = (struct block_header *)((uint8_t *)current + sizeof(struct block_header) + size);
                new_block->size = current->size - size - sizeof(struct block_header);
                new_block->is_free = 1;
                new_block->next = current->next;

                current->size = size;
                current->is_free = 0;
                current->next = new_block;
            }
            else
            {
                // Use the entire block
                current->is_free = 0;
            }
            return (void *)(current + 1); // Return pointer to the data area
        }
        previous = current;
        current = current->next;
    }
    return NULL; // No suitable block found
}

void kfree(void *ptr)
{
    if (ptr == NULL)
        return;

    struct block_header *block = (struct block_header *)ptr - 1; // Get the block header
    block->is_free = 1;                                          // Mark the block as free

    // Coalesce adjacent free blocks
    struct block_header *current = free_list;
    while (current != NULL)
    {
        if (current->is_free && current->next != NULL && current->next->is_free)
        {
            current->size += sizeof(struct block_header) + current->next->size; // Merge with next block
            current->next = current->next->next;                                // Skip the next block
        }
        else
        {
            current = current->next;
        }
    }
}

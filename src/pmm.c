#include "pmm.h"
#include "paging.h"

static uint8_t bitmap[128] = {0}; // Bitmap to track used/free pages (1024 pages / 8 bits per byte)

void pmm_init(void)
{
    for (int i = 0; i < sizeof(bitmap); i++)
    {
        bitmap[i] = 0;
    }

    // Mark the first 64 pages as used (256 KB)
    // Covers: IVT, BIOS data, bootloader, kernel, page tables, stack
    for (int i = 0; i < 64; i++)
    {
        bitmap[i / 8] |= (1 << (i % 8));
    }
}

uint32_t pmm_alloc(void)
{
    for (int i = 0; i < sizeof(bitmap); i++)
    {
        if (bitmap[i] != 0xFF) // If not all bits are set, there is a free page
        {
            for (int j = 0; j < 8; j++)
            {
                if ((bitmap[i] & (1 << j)) == 0) // Check if the bit is free
                {
                    bitmap[i] |= (1 << j);          // Mark the page as used
                    return (i * 8 + j) * PAGE_SIZE; // Return the physical address of the allocated page
                }
            }
        }
    }
    return 0; // No free pages available
}

void pmm_free(uint32_t address)
{
    uint32_t page_index = address / PAGE_SIZE; // Calculate the page index
    uint32_t byte_index = page_index / 8;      // Calculate the byte index in the bitmap
    uint32_t bit_index = page_index % 8;       // Calculate the bit index within the byte

    if (byte_index < sizeof(bitmap))
    {
        bitmap[byte_index] &= ~(1 << bit_index); // Mark the page as free
    }
}

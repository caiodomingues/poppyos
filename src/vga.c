#include "vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static int cursor_row = 0;
static int cursor_col = 0;
static char *video = (char *)VGA_MEMORY;

// Initialize the VGA text mode
void vga_init(void)
{
    cursor_row = 0;
    cursor_col = 0;
    video = (char *)VGA_MEMORY;
}

// Clear the screen by filling it with spaces and default color
void vga_clear(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        video[i * 2] = ' ';
        // Bit shifting: it shifts the background color (VGA_BLACK) to the left by 4 bits and combines it with the
        // foreground color (VGA_LIGHT_GREY) using a bitwise OR operation.
        video[i * 2 + 1] = (VGA_BLACK << 4) | VGA_LIGHT_GREY;
    }
    cursor_row = 0;
    cursor_col = 0;
}

// Print a string to the screen at the current cursor position
void vga_print(const char *str)
{
    while (*str)
    {
        vga_print_char(*str++);
    }
}

// Print a string with specified foreground and background colors
void vga_print_color(const char *str, enum vga_color fg, enum vga_color bg)
{
    while (*str)
    {
        // Check for newline character to handle line breaks
        if (*str == '\n')
        {
            cursor_col = 0;
            cursor_row++;
            if (cursor_row >= VGA_HEIGHT)
            {
                cursor_row = 0;
            }
            str++;
            continue;
        }

        video[(cursor_row * VGA_WIDTH + cursor_col) * 2] = *str++;
        video[(cursor_row * VGA_WIDTH + cursor_col) * 2 + 1] = (bg << 4) | fg;
        cursor_col++;
        if (cursor_col >= VGA_WIDTH)
        {
            cursor_col = 0;
            cursor_row++;
            if (cursor_row >= VGA_HEIGHT)
            {
                cursor_row = 0; // Scroll or reset to top
            }
        }
    }
}

// Print a single character to the screen at the current cursor position
void vga_print_char(char c)
{
    // Check for newline character to handle line breaks
    if (c == '\n')
    {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_HEIGHT)
        {
            cursor_row = 0; // Scroll or reset to top
        }
    }
    else if (c == '\b')
    {
        if (cursor_col > 0)
        {
            cursor_col--;
            video[(cursor_row * VGA_WIDTH + cursor_col) * 2] = ' ';
            video[(cursor_row * VGA_WIDTH + cursor_col) * 2 + 1] = (VGA_BLACK << 4) | VGA_LIGHT_GREY;
        }
    }
    else
    {
        video[(cursor_row * VGA_WIDTH + cursor_col) * 2] = c;
        video[(cursor_row * VGA_WIDTH + cursor_col) * 2 + 1] = (VGA_BLACK << 4) | VGA_LIGHT_GREY;
        cursor_col++;
        if (cursor_col >= VGA_WIDTH)
        {
            cursor_col = 0;
            cursor_row++;
            if (cursor_row >= VGA_HEIGHT)
            {
                cursor_row = 0; // Scroll or reset to top
            }
        }
    }
}

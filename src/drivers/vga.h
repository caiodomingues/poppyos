#ifndef VGA_H
#define VGA_H

// VGA colors (4 bits each, foreground + background)
enum vga_color
{
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_WHITE = 7,
    VGA_LIGHT_GREY = 8,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_YELLOW = 14,
    VGA_BRIGHT_WHITE = 15,
};

void vga_init(void);
void vga_clear(void);
void vga_print(const char *str);
void vga_print_color(const char *str, enum vga_color fg, enum vga_color bg);
void vga_print_char(char c);

#endif

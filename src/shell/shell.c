#include "shell.h"
#include "vga.h"
#include "timer.h"
#include "types.h"
#include "fs.h"

static char input_buffer[256];
static int buffer_position = 0;

static int str_equal(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static int starts_with(const char *str, const char *prefix)
{
    while (*prefix)
    {
        if (*str != *prefix)
            return 0;
        str++;
        prefix++;
    }
    return 1;
}

static void int_to_str(uint32_t num, char *buf)
{
    if (num == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char tmp[12];
    int i = 0;

    while (num > 0)
    {
        tmp[i++] = '0' + (num % 10);
        num /= 10;
    }

    for (int j = 0; j < i; j++)
    {
        buf[j] = tmp[i - 1 - j];
    }
    buf[i] = '\0';
}

static int find_space(const char *str)
{
    int i = 0;
    while (str[i])
    {
        if (str[i] == ' ')
            return i;
        i++;
    }
    return -1;
}

void shell_init(void)
{
    vga_print_char('>');
}

void shell_handle_key(char c)
{
    if (c == '\n')
    {
        input_buffer[buffer_position] = '\0';
        vga_print_char('\n');

        if (str_equal(input_buffer, "help"))
        {
            vga_print("Commands: help, clear, ticks, about,\n");
            vga_print("          ls, cat, write, rm\n");
        }
        else if (str_equal(input_buffer, "clear"))
        {
            vga_clear();
        }
        else if (str_equal(input_buffer, "ticks"))
        {
            char buf[12];
            int_to_str(timer_get_ticks(), buf);
            vga_print("Ticks: ");
            vga_print(buf);
            vga_print("\n");
        }
        else if (str_equal(input_buffer, "about"))
        {
            vga_print_color("PoppyOS v0.1\n", VGA_LIGHT_GREEN, VGA_BLACK);
            vga_print("A hobby OS built from scratch.\n");
            vga_print("Architecture: x86 (i386)\n");
            vga_print("Features: VGA, IDT, PIC, keyboard, timer,\n");
            vga_print("          paging, heap, multitasking\n");
        }
        else if (str_equal(input_buffer, "ls"))
        {
            fs_list();
        }
        else if (starts_with(input_buffer, "cat "))
        {
            char buf[512];
            if (fs_read(&input_buffer[4], buf) >= 0)
                vga_print(buf);
            else
                vga_print("File not found.\n");
            vga_print("\n");
        }
        else if (starts_with(input_buffer, "write "))
        {
            char *args = &input_buffer[6];
            int sp = find_space(args);
            if (sp > 0)
            {
                args[sp] = '\0';
                char *name = args;
                char *data = &args[sp + 1];
                if (fs_create(name, data) == 0)
                    vga_print("File created.\n");
                else
                    vga_print("Error: file exists or no space.\n");
            }
            else
            {
                vga_print("Usage: write <name> <content>\n");
            }
        }
        else if (starts_with(input_buffer, "rm "))
        {
            if (fs_delete(&input_buffer[3]) == 0)
                vga_print("File deleted.\n");
            else
                vga_print("File not found.\n");
        }
        else if (input_buffer[0] != '\0')
        {
            vga_print("Unknown command: ");
            vga_print(input_buffer);
            vga_print("\n");
        }

        buffer_position = 0;
        input_buffer[0] = '\0';
        vga_print_char('>');
    }
    else if (c == '\b')
    {
        if (buffer_position > 0)
        {
            buffer_position--;
            vga_print_char('\b');
        }
    }
    else if (buffer_position < sizeof(input_buffer) - 1)
    {
        input_buffer[buffer_position++] = c;
        vga_print_char(c);
    }
}
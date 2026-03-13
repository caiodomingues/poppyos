#include "shell.h"
#include "vga.h"

static char input_buffer[256];  // Buffer to store user input
static int buffer_position = 0; // Current position in the input buffer

int str_equal(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0; // Strings are not equal
        a++;
        b++;
    }
    return *a == *b; // Both strings should end at the same time
}

void shell_init(void)
{
    vga_print_char('>'); // Print the shell prompt
}

void shell_handle_key(char c)
{
    if (c == '\n') // If the user presses Enter
    {
        // Null-terminate the input buffer and process the command (prevents buffer overflow with garbage)
        input_buffer[buffer_position] = '\0';
        vga_print_char('\n');

        // Temp dumb command processing: just check for "help" and "clear"
        if (str_equal(input_buffer, "help"))
            vga_print("Commands: help, clear\n");
        else if (str_equal(input_buffer, "clear"))
            vga_clear();
        else
        {
            vga_print("Unknown command:\n");
            vga_print(input_buffer);
            vga_print("\n");
        }

        buffer_position = 0;    // Reset the buffer position for the next command
        input_buffer[0] = '\0'; // Clear the input buffer
        vga_print_char('>');    // Print the shell prompt
    }
    else if (c == '\b') // Handle backspace
    {
        if (buffer_position > 0)
        {
            buffer_position--;
            vga_print_char('\b');
        }
    }
    else if (buffer_position < sizeof(input_buffer) - 1) // Ensure we don't overflow the buffer
    {
        input_buffer[buffer_position++] = c; // Add character to buffer and move position
        vga_print_char(c);                   // Echo the character back to the screen
    }
}

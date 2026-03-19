#include "keyboard.h"
#include "ports.h"
#include "vga.h"
#include "shell.h"

// US QWERTY
// 0 = scancode
// 1 = Escape
// 2 = "1"
// ...
static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '};

void keyboard_handler(void)
{
    // Read the scancode from the keyboard controller (port 0x60)
    unsigned char scancode = port_in(0x60);

    // Ignore key releases (scancode with bit 7 set, > 0x80)
    if (scancode & 0x80)
    {
        return;
    }

    // Convert scancode to ASCII character with a lookup table
    if (scancode < sizeof(scancode_to_ascii))
    {
        char ascii = scancode_to_ascii[scancode];
        if (ascii)
        {
            shell_handle_key(ascii);
        }
    }
}
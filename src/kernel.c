void kernel_main()
{
    // VGA text buffer pointer
    char *video = (char *)0xB8000;

    // Clears the screen (80 cols, 25 rows, 2 bytes per character)
    for (int i = 0; i < 80 * 25 * 2; i += 2)
    {
        video[i] = ' ';      // ASCII space character
        video[i + 1] = 0x07; // White on black background
    }

    // Write "PoppyOS" to the top-left corner
    char *msg = "PoppyOS";
    int offset = 0;
    for (int i = 0; msg[i] != '\0'; i++)
    {
        video[offset] = msg[i];
        video[offset + 1] = 0x0A; // Green on black background
        offset += 2;
    }
}

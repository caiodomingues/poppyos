#include "fs.h"
#include "ata.h"
#include "vga.h"

#define FS_MAGIC 0x504F5059 // "POPY" in ASCII
#define SUPERBLOCK_LBA 0
#define TABLE_LBA 1 // sectors 1-2
#define DATA_LBA 3  // data starts at sector 3

struct superblock
{
    uint32_t magic;
    uint32_t file_count;
    uint8_t padding[504]; // completes 512 bytes
};

struct file_entry
{
    char name[MAX_FILENAME]; // 32 bytes
    uint32_t size;           // 4 bytes
    uint8_t used;            // 1 byte
    uint8_t padding[27];     // completes 64 bytes
};

static struct superblock sb;
static struct file_entry file_table[MAX_FILES];

static void str_copy(char *dest, const char *src)
{
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
}

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

static void fs_save(void)
{
    // Saves superblock
    ata_write_sector(SUPERBLOCK_LBA, (uint8_t *)&sb);

    // Saves file table (2 sectors)
    ata_write_sector(TABLE_LBA, (uint8_t *)&file_table[0]);
    ata_write_sector(TABLE_LBA + 1, (uint8_t *)&file_table[8]);
}

static void fs_load(void)
{
    ata_read_sector(SUPERBLOCK_LBA, (uint8_t *)&sb);
    ata_read_sector(TABLE_LBA, (uint8_t *)&file_table[0]);
    ata_read_sector(TABLE_LBA + 1, (uint8_t *)&file_table[8]);
}

void fs_init(void)
{
    fs_load();

    if (sb.magic != FS_MAGIC)
    {
        // Disk not formatted -> initialize
        sb.magic = FS_MAGIC;
        sb.file_count = 0;

        for (int i = 0; i < MAX_FILES; i++)
            file_table[i].used = 0;

        fs_save();
        vga_print("Filesystem created.\n");
    }
    else
    {
        vga_print("Filesystem loaded.\n");
    }
}

int fs_create(const char *name, const char *data)
{
    // Checks if the file already exists
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (file_table[i].used && str_equal(file_table[i].name, name))
            return -1; // already exists
    }

    // Finds a free slot
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (!file_table[i].used)
        {
            str_copy(file_table[i].name, name);
            file_table[i].used = 1;
            file_table[i].size = 0;

            // Calculates size and copies to buffer
            uint8_t sector_buf[512] = {0};
            int len = 0;
            while (data[len] && len < MAX_FILE_SIZE)
            {
                sector_buf[len] = data[len];
                len++;
            }
            file_table[i].size = len;

            // Writes data to the file's sector
            ata_write_sector(DATA_LBA + i, sector_buf);

            sb.file_count++;
            fs_save();
            return 0;
        }
    }
    return -2; // no space
}

int fs_read(const char *name, char *buf)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (file_table[i].used && str_equal(file_table[i].name, name))
        {
            uint8_t sector_buf[512];
            ata_read_sector(DATA_LBA + i, sector_buf);

            for (uint32_t j = 0; j < file_table[i].size; j++)
                buf[j] = sector_buf[j];
            buf[file_table[i].size] = '\0';

            return file_table[i].size;
        }
    }
    return -1; // not found
}

int fs_delete(const char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (file_table[i].used && str_equal(file_table[i].name, name))
        {
            file_table[i].used = 0;
            sb.file_count--;
            fs_save();
            return 0;
        }
    }
    return -1;
}

void fs_list(void)
{
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (file_table[i].used)
        {
            vga_print("  ");
            vga_print(file_table[i].name);
            vga_print("\n");
            found++;
        }
    }
    if (!found)
        vga_print("  (no files)\n");
}
#include "ata.h"
#include "ports.h"

#define ATA_DATA 0x1F0
#define ATA_ERROR 0x1F1
#define ATA_SECT_COUNT 0x1F2
#define ATA_LBA_LOW 0x1F3
#define ATA_LBA_MID 0x1F4
#define ATA_LBA_HIGH 0x1F5
#define ATA_DRIVE_HEAD 0x1F6
#define ATA_CMD_STATUS 0x1F7

#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_FLUSH 0xE7

static void ata_wait(void)
{
    // Waits for the drive to be ready (bit 7 = busy, bit 3 = data ready)
    while (port_in(ATA_CMD_STATUS) & 0x80)
        ; // Waits for busy to clear
}

static void ata_wait_data(void)
{
    ata_wait();
    while (!(port_in(ATA_CMD_STATUS) & 0x08))
        ; // Waits for DRQ to set
}

void ata_read_sector(uint32_t lba, uint8_t *buf)
{
    ata_wait();

    port_out(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F)); // drive 0, LBA mode
    port_out(ATA_SECT_COUNT, 1);                           // 1 sector
    port_out(ATA_LBA_LOW, lba & 0xFF);
    port_out(ATA_LBA_MID, (lba >> 8) & 0xFF);
    port_out(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    port_out(ATA_CMD_STATUS, ATA_CMD_READ);

    ata_wait_data();

    // Reads 256 words (512 bytes)
    for (int i = 0; i < 256; i++)
    {
        uint16_t word = port_in16(ATA_DATA);
        buf[i * 2] = word & 0xFF;
        buf[i * 2 + 1] = (word >> 8) & 0xFF;
    }
}

void ata_write_sector(uint32_t lba, const uint8_t *buf)
{
    ata_wait();

    port_out(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    port_out(ATA_SECT_COUNT, 1);
    port_out(ATA_LBA_LOW, lba & 0xFF);
    port_out(ATA_LBA_MID, (lba >> 8) & 0xFF);
    port_out(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    port_out(ATA_CMD_STATUS, ATA_CMD_WRITE);

    ata_wait_data();

    // Writes 256 words (512 bytes)
    for (int i = 0; i < 256; i++)
    {
        uint16_t word = buf[i * 2] | (buf[i * 2 + 1] << 8);
        port_out16(ATA_DATA, word);
    }

    // Flush cache
    port_out(ATA_CMD_STATUS, ATA_CMD_FLUSH);
    ata_wait();
}
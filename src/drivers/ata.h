#ifndef ATA_H
#define ATA_H

#include "types.h"

void ata_read_sector(uint32_t lba, uint8_t* buf);
void ata_write_sector(uint32_t lba, const uint8_t* buf);

#endif
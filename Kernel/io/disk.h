#pragma once
#include <stdint.h>

void ata_lba_read(void *dest, uint32_t lba, uint8_t count);
void ata_lba_write(const void *buf, uint32_t lba, uint8_t count);
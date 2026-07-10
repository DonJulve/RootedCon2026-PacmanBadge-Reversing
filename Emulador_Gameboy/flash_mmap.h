#ifndef FLASH_MMAP_H
#define FLASH_MMAP_H

#include "esp_partition.h"
#include <Arduino.h>

const uint8_t* mappedROM_init(const char* filepath);

#endif

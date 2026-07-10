#include "flash_mmap.h"
#include <LittleFS.h>

struct PartitionHeader {
    uint32_t magic;
    uint32_t crc32; // Storing fileSize here for simplicity, plus checking first bytes.
};
static constexpr uint32_t PART_MAGIC = 0x47422121u; // 'GB!!'

const uint8_t* mappedROM_init(const char* filepath) {
    const esp_partition_t* ptr_partition =
        esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "gbrom");
    if (!ptr_partition) {
        Serial.println("[flash_mmap] 'gbrom' partition not found.");
        return nullptr;
    }

    File file = LittleFS.open(filepath, FILE_READ);
    if (!file) {
        Serial.printf("[flash_mmap] Failed to open %s\n", filepath);
        return nullptr;
    }

    uint32_t fileSize = file.size();
    if (sizeof(PartitionHeader) + fileSize > ptr_partition->size) {
        Serial.printf("[flash_mmap] ROM (%u B) exceeds partition (%u B)\n", (unsigned)fileSize, (unsigned)ptr_partition->size);
        file.close();
        return nullptr;
    }

    PartitionHeader stored = {};
    esp_partition_read(ptr_partition, 0, &stored, sizeof(stored));
    
    bool needsWrite = true;
    if (stored.magic == PART_MAGIC && stored.crc32 == fileSize) {
        uint8_t firstBytes[16];
        uint8_t storedFirstBytes[16];
        file.seek(0);
        file.read(firstBytes, 16);
        esp_partition_read(ptr_partition, sizeof(PartitionHeader), storedFirstBytes, 16);
        if (memcmp(firstBytes, storedFirstBytes, 16) == 0) {
            needsWrite = false;
        }
    }

    if (needsWrite) {
        Serial.println("[flash_mmap] Writing ROM to flash partition...");
        if (esp_partition_erase_range(ptr_partition, 0, ptr_partition->size) != ESP_OK) {
            Serial.println("[flash_mmap] Erase failed.");
            file.close();
            return nullptr;
        }

        uint8_t buf[4096];
        PartitionHeader header = { PART_MAGIC, fileSize };
        esp_partition_write(ptr_partition, 0, &header, sizeof(header));

        file.seek(0);
        uint32_t offset = sizeof(PartitionHeader);
        while (file.available()) {
            size_t bytesRead = file.read(buf, sizeof(buf));
            esp_partition_write(ptr_partition, offset, buf, bytesRead);
            offset += bytesRead;
        }
        Serial.println("[flash_mmap] Flash write complete.");
    } else {
        Serial.println("[flash_mmap] ROM already in flash, skipping write.");
    }
    file.close();

    const void* mapped;
    esp_partition_mmap_handle_t mmap_handle;
    esp_err_t err = esp_partition_mmap(ptr_partition, 0, fileSize + sizeof(PartitionHeader),
                                       ESP_PARTITION_MMAP_DATA, &mapped, &mmap_handle);
    if (err != ESP_OK) {
        Serial.printf("[flash_mmap] mmap failed: %s\n", esp_err_to_name(err));
        return nullptr;
    }

    const uint8_t* prg_base = ((const uint8_t*)mapped) + sizeof(PartitionHeader);
    Serial.printf("[flash_mmap] mmap successful, base: %p\n", prg_base);
    return prg_base;
}

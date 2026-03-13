#include "data_buffer.h"
#include <string.h>

static DataBuffer_t buffer;
extern W25Q_t flash;

#define FLASH_START_ADDRESS 0x000000
#define FLASH_SECTOR_SIZE   4096
//#define SAMPLE_SIZE 10
#define SAMPLE_SIZE         sizeof(DataSample_t)
#define MAX_SAMPLES         ((4UL * 1024UL * 1024UL) / SAMPLE_SIZE)

void Buffer_Init(void) {
    memset(&buffer, 0, sizeof(DataBuffer_t));
    buffer.count = 0;
    buffer.total_samples_saved = 0;
}

void Buffer_AddSample(float temp, float pressure, uint8_t humidity,uint8_t battery_percent, DS3231_Time_t *time) {
    if (buffer.count < BUFFER_SIZE) {
        buffer.samples[buffer.count]. temperature = temp;
        buffer.samples[buffer.count].pressure = pressure;
        buffer.samples[buffer.count].humidity = humidity;
        buffer.samples[buffer.count].battery_percent = battery_percent; // <--- NOVO!
        memcpy(&buffer.samples[buffer.count].timestamp, time, sizeof(DS3231_Time_t));
        buffer.count++;
    }
}

uint8_t Buffer_IsFull(void) {
    return (buffer.count >= BUFFER_SIZE);
}

void Buffer_SaveToFlash(void) {
    if (buffer.count == 0) return;

    if (buffer.total_samples_saved >= MAX_SAMPLES) {
        Buffer_Clear();
        return;
    }

    // ✅ GRAVAR CADA AMOSTRA NA FLASH
    for (uint8_t i = 0; i < buffer.count; i++) {

        // Converter estrutura para bytes
        uint8_t *data = (uint8_t *)&buffer.samples[i];

        // Gravar usando a função W25Q (ela usa endereço interno)
        W25Q_WriteDataSample(&flash, data, SAMPLE_SIZE);

        buffer.total_samples_saved++;

        if (buffer.total_samples_saved >= MAX_SAMPLES) {
            break;
        }
    }

    Buffer_Clear();
}

void Buffer_Clear(void) {
    buffer.count = 0;
}

uint32_t Buffer_GetTotalSamples(void) {
    return buffer.total_samples_saved;
}

uint32_t Buffer_GetFlashCapacity(void) {
    return MAX_SAMPLES;
}

float Buffer_GetFlashUsagePercent(void) {
    return ((float)buffer.total_samples_saved / (float)MAX_SAMPLES) * 100.0f;
}

void Buffer_EraseFlash(void) {
    // Apagar toda a Flash (ou primeiro setor)
    W25Q_EraseSector(&flash, FLASH_START_ADDRESS);

    // Resetar contadores da W25Q também
    W25Q_Reset(&flash);

    buffer.total_samples_saved = 0;
    buffer.count = 0;
}

#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include "stm32f4xx_hal.h"
#include "ds3231.h"
#include "w25qxx.h"

#define BUFFER_SIZE 10

// Estrutura principal de amostra, agora com battery_percent incluído
typedef struct {
    float temperature;
    float pressure;
    uint8_t humidity;
    uint8_t battery_percent;     // Percentual de carga da bateria (%)
    DS3231_Time_t timestamp;
} DataSample_t;

typedef struct {
    DataSample_t samples[BUFFER_SIZE];
    uint8_t count;
    uint32_t total_samples_saved;
} DataBuffer_t;

// Inicializa o buffer circular de amostras
void Buffer_Init(void);

// Adiciona uma amostra ao buffer e agora recebe também percent. bateria!
// (ATUALIZE sua implementação para receber esse novo argumento)
void Buffer_AddSample(float temp, float pressure, uint8_t humidity, uint8_t battery_percent, DS3231_Time_t *time);

uint8_t Buffer_IsFull(void);
void Buffer_SaveToFlash(void);
void Buffer_Clear(void);

// Informações sobre uso do buffer e flash
uint32_t Buffer_GetTotalSamples(void);
uint32_t Buffer_GetFlashCapacity(void);
float Buffer_GetFlashUsagePercent(void);
void Buffer_EraseFlash(void);

#endif

#ifndef USB_DATALOGGER_H
#define USB_DATALOGGER_H

#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "ds3231.h"
#include <stdbool.h>

// Estados da máquina de estados
typedef enum {
    USB_STATE_IDLE,
    USB_STATE_DETECTING,
    USB_STATE_MOUNTING,
    USB_STATE_READY,
    USB_STATE_WRITING,
    USB_STATE_ERROR
} USB_State_t;

// Erros
typedef enum {
    USB_ERROR_NONE,
    USB_ERROR_NOT_CONNECTED,
    USB_ERROR_MOUNT_FAILED,
    USB_ERROR_FILE_OPEN_FAILED,
    USB_ERROR_WRITE_FAILED
} USB_Error_t;

// Estrutura do DataLogger
typedef struct {
    USB_State_t state;
    USB_Error_t last_error;
    FIL file;
    char filename[32];
    char write_buffer[128];
    uint32_t last_write_time;
    uint32_t write_interval;
    uint32_t records_written;
    bool connected;
    bool mounted;
    bool file_open;
} USB_DataLogger_t;

// Funções públicas
void USB_DataLogger_Init(USB_DataLogger_t *logger, uint32_t write_interval_ms);
void USB_DataLogger_Process(USB_DataLogger_t *logger);
bool USB_DataLogger_WriteData(USB_DataLogger_t *logger, DS3231_Time_t *time, float temp, float pressure, uint8_t humidity);
bool USB_DataLogger_IsReady(USB_DataLogger_t *logger);
const char* USB_DataLogger_GetStateString(USB_State_t state);
const char* USB_DataLogger_GetErrorString(USB_Error_t error);

// Exportar Flash → CSV
uint8_t USB_DataLogger_ExportFlashToCSV(USB_DataLogger_t *logger);
uint8_t USB_DataLogger_ExportTestFlashToCSV(USB_DataLogger_t *logger);

#endif

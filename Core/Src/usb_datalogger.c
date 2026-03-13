#include "usb_datalogger.h"
#include "usb_host.h"
#include <string.h>
#include <stdio.h>
#include "w25qxx.h"
#include "data_buffer.h"
#include "ds3231.h"

// Variáveis externas
extern ApplicationTypeDef Appli_state;
extern FATFS USBHFatFS;
extern char USBHPath[4];

// RTC para nome do arquivo normal
extern I2C_HandleTypeDef hi2c1;

// Protótipos
static void State_Idle(USB_DataLogger_t *logger);
static void State_Detecting(USB_DataLogger_t *logger);
static void State_Mounting(USB_DataLogger_t *logger);
static void State_Ready(USB_DataLogger_t *logger);
static void State_Writing(USB_DataLogger_t *logger);
static void State_Error(USB_DataLogger_t *logger);

void USB_DataLogger_Init(USB_DataLogger_t *logger, uint32_t write_interval_ms) {
    logger->state = USB_STATE_IDLE;
    logger->last_error = USB_ERROR_NONE;
    logger->write_interval = write_interval_ms;
    logger->last_write_time = 0;
    logger->records_written = 0;
    logger->connected = false;
    logger->mounted = false;
    logger->file_open = false;
    memset(logger->filename, 0, sizeof(logger->filename));
    memset(logger->write_buffer, 0, sizeof(logger->write_buffer));
}

void USB_DataLogger_Process(USB_DataLogger_t *logger) {
    switch(logger->state) {
        case USB_STATE_IDLE:       State_Idle(logger); break;
        case USB_STATE_DETECTING:  State_Detecting(logger); break;
        case USB_STATE_MOUNTING:   State_Mounting(logger); break;
        case USB_STATE_READY:      State_Ready(logger); break;
        case USB_STATE_WRITING:    State_Writing(logger); break;
        case USB_STATE_ERROR:      State_Error(logger); break;
    }
}

bool USB_DataLogger_WriteData(USB_DataLogger_t *logger, DS3231_Time_t *time, float temp, float pressure, uint8_t humidity) {
    if (logger->state != USB_STATE_READY) return false;

    int temp_int = (int)temp;
    int temp_dec = (int)((temp - temp_int) * 100);
    if (temp_dec < 0) temp_dec = -temp_dec;

    int press_int = (int)pressure;
    int press_dec = (int)((pressure - press_int) * 100);
    if (press_dec < 0) press_dec = -press_dec;

    snprintf(logger->write_buffer, sizeof(logger->write_buffer),
             "%04d-%02d-%02d %02d:%02d:%02d,%d.%02d,%d.%02d,%d\r\n",
             time->year, time->month, time->date,
             time->hours, time->minutes, time->seconds,
             temp_int, temp_dec, press_int, press_dec, humidity);

    logger->state = USB_STATE_WRITING;
    return true;
}

bool USB_DataLogger_IsReady(USB_DataLogger_t *logger) {
    return (logger->state == USB_STATE_READY);
}

const char* USB_DataLogger_GetStateString(USB_State_t state) {
    switch(state) {
        case USB_STATE_IDLE: return "IDLE";
        case USB_STATE_DETECTING: return "DETECTING";
        case USB_STATE_MOUNTING: return "MOUNTING";
        case USB_STATE_READY: return "READY";
        case USB_STATE_WRITING: return "WRITING";
        case USB_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

const char* USB_DataLogger_GetErrorString(USB_Error_t error) {
    switch(error) {
        case USB_ERROR_NONE: return "NONE";
        case USB_ERROR_NOT_CONNECTED: return "NOT_CONNECTED";
        case USB_ERROR_MOUNT_FAILED: return "MOUNT_FAILED";
        case USB_ERROR_FILE_OPEN_FAILED: return "FILE_OPEN_FAILED";
        case USB_ERROR_WRITE_FAILED: return "WRITE_FAILED";
        default: return "UNKNOWN";
    }
}

// ====== Estados ======
static void State_Idle(USB_DataLogger_t *logger) {
    logger->connected = false;
    logger->mounted = false;

    if (Appli_state == APPLICATION_READY) {
        logger->state = USB_STATE_DETECTING;
    }
}

static void State_Detecting(USB_DataLogger_t *logger) {
    if (Appli_state == APPLICATION_READY) {
        logger->connected = true;
        logger->state = USB_STATE_MOUNTING;
    } else if (Appli_state == APPLICATION_DISCONNECT) {
        logger->state = USB_STATE_IDLE;
    }
}

static void State_Mounting(USB_DataLogger_t *logger) {
    if (f_mount(&USBHFatFS, USBHPath, 0) == FR_OK) {
        logger->mounted = true;
        logger->state = USB_STATE_READY;
    } else {
        logger->last_error = USB_ERROR_MOUNT_FAILED;
        logger->state = USB_STATE_ERROR;
    }
}

static void State_Ready(USB_DataLogger_t *logger) {
    if (Appli_state != APPLICATION_READY) {
        if (logger->file_open) {
            f_close(&logger->file);
            logger->file_open = false;
        }
        logger->state = USB_STATE_IDLE;
    }
}

static void State_Writing(USB_DataLogger_t *logger) {
    UINT bytes_written;

    if (f_write(&logger->file, logger->write_buffer, strlen(logger->write_buffer), &bytes_written) == FR_OK) {
        f_sync(&logger->file);
        logger->records_written++;
        logger->last_write_time = HAL_GetTick();
        logger->state = USB_STATE_READY;
    } else {
        logger->last_error = USB_ERROR_WRITE_FAILED;
        logger->state = USB_STATE_ERROR;
    }
}

static void State_Error(USB_DataLogger_t *logger) {
    if (logger->file_open) {
        f_close(&logger->file);
        logger->file_open = false;
    }

    HAL_Delay(2000);
    logger->state = USB_STATE_IDLE;
}

// ===== EXPORT NORMAL: LOG_YYYYMMDD_HHMMSS.CSV (não sobrescreve) =====
uint8_t USB_DataLogger_ExportFlashToCSV(USB_DataLogger_t *logger) {
    if (Appli_state != APPLICATION_READY) return 0;

    FIL file;
    FRESULT res;
    UINT bw;

    DS3231_Time_t t;
    DS3231_GetTime(&hi2c1, &t);

    char filename[64];
    snprintf(filename, sizeof(filename),
             "0:LOG_%04d%02d%02d_%02d%02d%02d.CSV",
             t.year, t.month, t.date, t.hours, t.minutes, t.seconds);

    res = f_open(&file, filename, FA_CREATE_NEW | FA_WRITE);
    if (res == FR_EXIST) {
        for (int i = 1; i <= 99 && res == FR_EXIST; i++) {
            snprintf(filename, sizeof(filename),
                     "0:LOG_%04d%02d%02d_%02d%02d%02d_%02d.CSV",
                     t.year, t.month, t.date, t.hours, t.minutes, t.seconds, i);
            res = f_open(&file, filename, FA_CREATE_NEW | FA_WRITE);
        }
    }
    if (res != FR_OK) return 0;

    const char *header = "Date,Time,Temperature_C,Pressure_hPa,Humidity_Percent,Battery_Percent\r\n";
    f_write(&file, header, strlen(header), &bw);

    extern W25Q_t flash;
    uint32_t total = W25Q_GetTotalSamples(&flash);
    if (total == 0) { f_close(&file); return 1; }

    DataSample_t s;
    char line[160];
    uint16_t sz = sizeof(DataSample_t);

    for (uint32_t i = 0; i < total; i++) {
        W25Q_ReadData(&flash, i * sz, (uint8_t*)&s, sz);

        snprintf(line, sizeof(line),
                 "%02d/%02d/%04d,%02d:%02d:%02d,%.2f,%.2f,%d,%d\r\n",
                 s.timestamp.date, s.timestamp.month, s.timestamp.year,
                 s.timestamp.hours, s.timestamp.minutes, s.timestamp.seconds,
                 s.temperature, s.pressure, s.humidity, s.battery_percent);

        res = f_write(&file, line, strlen(line), &bw);
        if (res != FR_OK) { f_close(&file); return 0; }
    }

    f_close(&file);
    return 1;
}

// ===== EXPORT TESTE: TESTE.CSV (sobrescreve sempre) =====
uint8_t USB_DataLogger_ExportTestFlashToCSV(USB_DataLogger_t *logger) {
    if (Appli_state != APPLICATION_READY) return 0;

    FIL file;
    FRESULT res;
    UINT bw;

    res = f_open(&file, "0:TEST.CSV", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) return 0;

    const char *header = "Date,Time,Temperature_C,Pressure_hPa,Humidity_Percent,Battery_Percent\r\n";
    f_write(&file, header, strlen(header), &bw);

    extern W25Q_t flash;
    uint32_t total = W25Q_GetTestTotalSamples(&flash);
    if (total == 0) { f_close(&file); return 1; }

    DataSample_t s;
    char line[160];
    uint16_t sz = sizeof(DataSample_t);

    for (uint32_t i = 0; i < total; i++) {
        uint32_t addr = W25Q_TEST_DATA_ADDR + (i * sz);
        if (addr + sz > W25Q_TEST_STATE_ADDR) break;

        W25Q_ReadData(&flash, addr, (uint8_t*)&s, sz);

        snprintf(line, sizeof(line),
                 "%02d/%02d/%04d,%02d:%02d:%02d,%.2f,%.2f,%d,%d\r\n",
                 s.timestamp.date, s.timestamp.month, s.timestamp.year,
                 s.timestamp.hours, s.timestamp.minutes, s.timestamp.seconds,
                 s.temperature, s.pressure, s.humidity, s.battery_percent);

        res = f_write(&file, line, strlen(line), &bw);
        if (res != FR_OK) { f_close(&file); return 0; }
    }

    f_close(&file);
    return 1;
}

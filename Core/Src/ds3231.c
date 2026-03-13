/*
 * ds3231.c
 *
 *  Created on: Dec 27, 2025
 *      Author: igor
 */


#include "ds3231.h"

// Registradores do DS3231
#define DS3231_REG_SECONDS   0x00
#define DS3231_REG_MINUTES   0x01
#define DS3231_REG_HOURS     0x02
#define DS3231_REG_DAY       0x03
#define DS3231_REG_DATE      0x04
#define DS3231_REG_MONTH     0x05
#define DS3231_REG_YEAR      0x06
#define DS3231_REG_TEMP_MSB  0x11

// Converter BCD para decimal
static uint8_t BCD2DEC(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Converter decimal para BCD
static uint8_t DEC2BCD(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// Inicializar DS3231
bool DS3231_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t test;
    // Tentar ler registrador de segundos
    if (HAL_I2C_Mem_Read(hi2c, DS3231_ADDR, DS3231_REG_SECONDS, 1, &test, 1, 100) == HAL_OK) {
        return true;
    }
    return false;
}

// Configurar data/hora
bool DS3231_SetTime(I2C_HandleTypeDef *hi2c, DS3231_Time_t *time) {
    uint8_t data[7];

    data[0] = DEC2BCD(time->seconds);
    data[1] = DEC2BCD(time->minutes);
    data[2] = DEC2BCD(time->hours);
    data[3] = DEC2BCD(time->day);
    data[4] = DEC2BCD(time->date);
    data[5] = DEC2BCD(time->month);
    data[6] = DEC2BCD(time->year - 2000);

    if (HAL_I2C_Mem_Write(hi2c, DS3231_ADDR, DS3231_REG_SECONDS, 1, data, 7, 100) == HAL_OK) {
        return true;
    }
    return false;
}

// Ler data/hora
bool DS3231_GetTime(I2C_HandleTypeDef *hi2c, DS3231_Time_t *time) {
    uint8_t data[7];

    if (HAL_I2C_Mem_Read(hi2c, DS3231_ADDR, DS3231_REG_SECONDS, 1, data, 7, 100) == HAL_OK) {
        time->seconds = BCD2DEC(data[0]);
        time->minutes = BCD2DEC(data[1]);
        time->hours = BCD2DEC(data[2] & 0x3F);  // Modo 24h
        time->day = BCD2DEC(data[3]);
        time->date = BCD2DEC(data[4]);
        time->month = BCD2DEC(data[5] & 0x1F);
        time->year = BCD2DEC(data[6]) + 2000;
        return true;
    }
    return false;
}

// Ler temperatura interna do DS3231
float DS3231_GetTemperature(I2C_HandleTypeDef *hi2c) {
    uint8_t data[2];

    if (HAL_I2C_Mem_Read(hi2c, DS3231_ADDR, DS3231_REG_TEMP_MSB, 1, data, 2, 100) == HAL_OK) {
        int16_t temp = (data[0] << 8) | data[1];
        return (float)temp / 256.0f;
    }
    return 0.0f;
}

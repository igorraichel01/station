/*
 * ds3231.h
 *
 *  Created on: Dec 27, 2025
 *      Author: igor
 */

#ifndef DS3231_H
#define DS3231_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

#define DS3231_ADDR  0xD0  // Endereço I2C (0x68 << 1)

// Estrutura de data/hora
typedef struct {
    uint8_t seconds;    // 0-59
    uint8_t minutes;    // 0-59
    uint8_t hours;      // 0-23
    uint8_t day;        // 1-7 (dia da semana)
    uint8_t date;       // 1-31
    uint8_t month;      // 1-12
    uint16_t year;      // 2000-2099
} DS3231_Time_t;

// Funções públicas
bool DS3231_Init(I2C_HandleTypeDef *hi2c);
bool DS3231_SetTime(I2C_HandleTypeDef *hi2c, DS3231_Time_t *time);
bool DS3231_GetTime(I2C_HandleTypeDef *hi2c, DS3231_Time_t *time);
float DS3231_GetTemperature(I2C_HandleTypeDef *hi2c);

#endif

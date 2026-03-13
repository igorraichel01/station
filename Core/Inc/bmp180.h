#ifndef BMP180_H
#define BMP180_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// Endereço I2C do BMP180
#define BMP180_ADDRESS 0xEE

// Registradores
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6
#define BMP180_REG_CAL_AC1 0xAA

// Comandos
#define BMP180_CMD_TEMPERATURE 0x2E
#define BMP180_CMD_PRESSURE_0 0x34
#define BMP180_CMD_PRESSURE_1 0x74
#define BMP180_CMD_PRESSURE_2 0xB4
#define BMP180_CMD_PRESSURE_3 0xF4

// Estrutura de calibração
typedef struct {
    int16_t AC1;
    int16_t AC2;
    int16_t AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t B1;
    int16_t B2;
    int16_t MB;
    int16_t MC;
    int16_t MD;
} BMP180_CalibData;

// Estrutura do sensor
typedef struct {
    I2C_HandleTypeDef *hi2c;
    BMP180_CalibData calib;
    uint8_t oversampling;
} BMP180_t;

// Funções
uint8_t BMP180_Init(BMP180_t *bmp, I2C_HandleTypeDef *hi2c, uint8_t oversampling);
float BMP180_ReadTemperature(BMP180_t *bmp);
float BMP180_ReadPressure(BMP180_t *bmp);
float BMP180_ReadAltitude(BMP180_t *bmp, float seaLevelPressure);

#endif

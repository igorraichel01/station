#include "bmp180.h"
#include <math.h>

static int32_t B5; // Usado para cálculo de temperatura e pressão

// Função auxiliar para ler dados do I2C
static HAL_StatusTypeDef BMP180_ReadReg(BMP180_t *bmp, uint8_t reg, uint8_t *data, uint16_t len) {
    return HAL_I2C_Mem_Read(bmp->hi2c, BMP180_ADDRESS, reg, 1, data, len, HAL_MAX_DELAY);
}

// Função auxiliar para escrever no I2C
static HAL_StatusTypeDef BMP180_WriteReg(BMP180_t *bmp, uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(bmp->hi2c, BMP180_ADDRESS, reg, 1, &data, 1, HAL_MAX_DELAY);
}

// Inicialização do BMP180
uint8_t BMP180_Init(BMP180_t *bmp, I2C_HandleTypeDef *hi2c, uint8_t oversampling) {
    bmp->hi2c = hi2c;
    bmp->oversampling = oversampling;

    uint8_t buffer[22];

    // Ler dados de calibração
    if (BMP180_ReadReg(bmp, BMP180_REG_CAL_AC1, buffer, 22) != HAL_OK) {
        return 0;
    }

    // Processar dados de calibração
    bmp->calib.AC1 = (int16_t)(buffer[0] << 8 | buffer[1]);
    bmp->calib.AC2 = (int16_t)(buffer[2] << 8 | buffer[3]);
    bmp->calib.AC3 = (int16_t)(buffer[4] << 8 | buffer[5]);
    bmp->calib.AC4 = (uint16_t)(buffer[6] << 8 | buffer[7]);
    bmp->calib.AC5 = (uint16_t)(buffer[8] << 8 | buffer[9]);
    bmp->calib. AC6 = (uint16_t)(buffer[10] << 8 | buffer[11]);
    bmp->calib.B1 = (int16_t)(buffer[12] << 8 | buffer[13]);
    bmp->calib.B2 = (int16_t)(buffer[14] << 8 | buffer[15]);
    bmp->calib.MB = (int16_t)(buffer[16] << 8 | buffer[17]);
    bmp->calib.MC = (int16_t)(buffer[18] << 8 | buffer[19]);
    bmp->calib.MD = (int16_t)(buffer[20] << 8 | buffer[21]);

    return 1;
}

// Ler temperatura
float BMP180_ReadTemperature(BMP180_t *bmp) {
    uint8_t data[2];
    int32_t UT;
    int32_t X1, X2;

    // Iniciar medição de temperatura
    BMP180_WriteReg(bmp, BMP180_REG_CONTROL, BMP180_CMD_TEMPERATURE);
    HAL_Delay(5);

    // Ler resultado
    BMP180_ReadReg(bmp, BMP180_REG_RESULT, data, 2);
    UT = (int32_t)(data[0] << 8 | data[1]);

    // Calcular temperatura real
    X1 = ((UT - bmp->calib. AC6) * bmp->calib.AC5) >> 15;
    X2 = (bmp->calib. MC << 11) / (X1 + bmp->calib.MD);
    B5 = X1 + X2;

    return ((B5 + 8) >> 4) / 10.0f;
}

// Ler pressão
float BMP180_ReadPressure(BMP180_t *bmp) {
    uint8_t data[3];
    int32_t UP;
    int32_t B3, B6;
    uint32_t B4, B7;
    int32_t X1, X2, X3;
    int32_t pressure;
    uint8_t cmd;
    uint8_t delay_ms;

    // Definir comando e delay baseado no oversampling
    switch(bmp->oversampling) {
        case 0:  cmd = BMP180_CMD_PRESSURE_0; delay_ms = 5; break;
        case 1: cmd = BMP180_CMD_PRESSURE_1; delay_ms = 8; break;
        case 2: cmd = BMP180_CMD_PRESSURE_2; delay_ms = 14; break;
        case 3: cmd = BMP180_CMD_PRESSURE_3; delay_ms = 26; break;
        default: cmd = BMP180_CMD_PRESSURE_0; delay_ms = 5; break;
    }

    // Iniciar medição de pressão
    BMP180_WriteReg(bmp, BMP180_REG_CONTROL, cmd);
    HAL_Delay(delay_ms);

    // Ler resultado
    BMP180_ReadReg(bmp, BMP180_REG_RESULT, data, 3);
    UP = ((int32_t)data[0] << 16 | (int32_t)data[1] << 8 | (int32_t)data[2]) >> (8 - bmp->oversampling);

    // Calcular pressão real
    B6 = B5 - 4000;
    X1 = (bmp->calib.B2 * ((B6 * B6) >> 12)) >> 11;
    X2 = (bmp->calib.AC2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = (((bmp->calib.AC1 * 4 + X3) << bmp->oversampling) + 2) / 4;
    X1 = (bmp->calib.AC3 * B6) >> 13;
    X2 = (bmp->calib.B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = (bmp->calib.AC4 * (uint32_t)(X3 + 32768)) >> 15;
    B7 = ((uint32_t)UP - B3) * (50000 >> bmp->oversampling);

    if (B7 < 0x80000000) {
        pressure = (B7 * 2) / B4;
    } else {
        pressure = (B7 / B4) * 2;
    }

    X1 = (pressure >> 8) * (pressure >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * pressure) >> 16;
    pressure = pressure + ((X1 + X2 + 3791) >> 4);

    return pressure / 100.0f; // Retorna em hPa
}

// Calcular altitude
float BMP180_ReadAltitude(BMP180_t *bmp, float seaLevelPressure) {
    float pressure = BMP180_ReadPressure(bmp);
    return 44330.0f * (1.0f - powf(pressure / seaLevelPressure, 0.1903f));
}

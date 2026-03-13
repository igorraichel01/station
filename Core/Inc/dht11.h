/*
 * dht11.h
 *
 *  Created on: Dec 26, 2025
 *      Author: igor
 */

#ifndef DHT11_H
#define DHT11_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// Estrutura do DHT11
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t temperature;
    uint8_t humidity;
    bool data_valid;
} DHT11_t;

// Funções públicas
void DHT11_Init(DHT11_t *dht, GPIO_TypeDef *port, uint16_t pin);
bool DHT11_Read(DHT11_t *dht);
uint8_t DHT11_GetTemperature(DHT11_t *dht);
uint8_t DHT11_GetHumidity(DHT11_t *dht);

#endif

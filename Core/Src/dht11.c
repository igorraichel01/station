/*
 * dht11.c
 *
 *  Created on: Dec 26, 2025
 *      Author: igor
 */


#include "dht11.h"

// Funções privadas
static void DHT11_SetPinOutput(DHT11_t *dht);
static void DHT11_SetPinInput(DHT11_t *dht);
static void DHT11_WritePin(DHT11_t *dht, GPIO_PinState state);
static GPIO_PinState DHT11_ReadPin(DHT11_t *dht);
static void delay_us(uint32_t us);

// Inicialização
void DHT11_Init(DHT11_t *dht, GPIO_TypeDef *port, uint16_t pin) {
    dht->port = port;
    dht->pin = pin;
    dht->temperature = 0;
    dht->humidity = 0;
    dht->data_valid = false;
}

// Leitura do DHT11
bool DHT11_Read(DHT11_t *dht) {
    uint8_t data[5] = {0};
    uint8_t bit_count = 0;
    uint8_t byte_count = 0;

    // Sinal de START:  LOW por 18ms
    DHT11_SetPinOutput(dht);
    DHT11_WritePin(dht, GPIO_PIN_RESET);
    HAL_Delay(18);

    // Liberar linha
    DHT11_WritePin(dht, GPIO_PIN_SET);
    delay_us(30);

    // Configurar como entrada
    DHT11_SetPinInput(dht);

    // Aguardar resposta do DHT11 (LOW por 80us, HIGH por 80us)
    uint32_t timeout = 1000;
    while(DHT11_ReadPin(dht) == GPIO_PIN_SET && timeout--);
    if(timeout == 0) return false;

    timeout = 1000;
    while(DHT11_ReadPin(dht) == GPIO_PIN_RESET && timeout--);
    if(timeout == 0) return false;

    timeout = 1000;
    while(DHT11_ReadPin(dht) == GPIO_PIN_SET && timeout--);
    if(timeout == 0) return false;

    // Ler 40 bits (5 bytes)
    for(byte_count = 0; byte_count < 5; byte_count++) {
        for(bit_count = 0; bit_count < 8; bit_count++) {
            // Aguardar início do bit (LOW por 50us)
            timeout = 1000;
            while(DHT11_ReadPin(dht) == GPIO_PIN_RESET && timeout--);
            if(timeout == 0) return false;

            // Aguardar 40us e verificar se ainda está HIGH
            delay_us(40);

            if(DHT11_ReadPin(dht) == GPIO_PIN_SET) {
                // Bit 1 (HIGH por 70us)
                data[byte_count] |= (1 << (7 - bit_count));
            }
            // Bit 0 (HIGH por 26-28us) - não faz nada

            // Aguardar fim do bit
            timeout = 1000;
            while(DHT11_ReadPin(dht) == GPIO_PIN_SET && timeout--);
            if(timeout == 0) return false;
        }
    }

    // Verificar checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if(checksum != data[4]) {
        dht->data_valid = false;
        return false;
    }

    // Dados válidos
    dht->humidity = data[0];
    dht->temperature = data[2];
    dht->data_valid = true;

    return true;
}

// Obter temperatura
uint8_t DHT11_GetTemperature(DHT11_t *dht) {
    return dht->temperature;
}

// Obter umidade
uint8_t DHT11_GetHumidity(DHT11_t *dht) {
    return dht->humidity;
}

// Configurar pino como saída
static void DHT11_SetPinOutput(DHT11_t *dht) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dht->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(dht->port, &GPIO_InitStruct);
}

// Configurar pino como entrada
static void DHT11_SetPinInput(DHT11_t *dht) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct. Pin = dht->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(dht->port, &GPIO_InitStruct);
}

// Escrever no pino
static void DHT11_WritePin(DHT11_t *dht, GPIO_PinState state) {
    HAL_GPIO_WritePin(dht->port, dht->pin, state);
}

// Ler do pino
static GPIO_PinState DHT11_ReadPin(DHT11_t *dht) {
    return HAL_GPIO_ReadPin(dht->port, dht->pin);
}

// Delay em microsegundos (aproximado)
static void delay_us(uint32_t us) {
    uint32_t cycles = (SystemCoreClock / 1000000) * us / 5;
    while(cycles--) {
        __NOP();
    }
}

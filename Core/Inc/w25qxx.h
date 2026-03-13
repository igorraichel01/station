/*
 * w25qxx.h
 *
 *  Created on:  Jan 2, 2026
 *      Author: igor
 */
#ifndef W25QXX_H
#define W25QXX_H

#include "main.h"
#include <stdint.h>

// Comandos W25Q32
#define W25Q_WRITE_ENABLE       0x06
#define W25Q_WRITE_DISABLE      0x04
#define W25Q_READ_STATUS_REG1   0x05
#define W25Q_READ_STATUS_REG2   0x35
#define W25Q_WRITE_STATUS_REG   0x01
#define W25Q_PAGE_PROGRAM       0x02
#define W25Q_QUAD_PAGE_PROGRAM  0x32
#define W25Q_BLOCK_ERASE_64KB   0xD8
#define W25Q_BLOCK_ERASE_32KB   0x52
#define W25Q_SECTOR_ERASE_4KB   0x20
#define W25Q_CHIP_ERASE         0xC7
#define W25Q_READ_DATA          0x03
#define W25Q_FAST_READ          0x0B
#define W25Q_POWER_DOWN         0xB9
#define W25Q_RELEASE_POWER_DOWN 0xAB
#define W25Q_DEVICE_ID          0xAB
#define W25Q_MANUFACTURER_ID    0x90
#define W25Q_JEDEC_ID           0x9F

// Tamanhos W25Q32 (4MB)
#define W25Q32_PAGE_SIZE        256
#define W25Q32_SECTOR_SIZE      4096
#define W25Q32_BLOCK_SIZE       65536
#define W25Q32_CAPACITY         4194304UL  // 4MB

// Status register bits
#define W25Q_STATUS_BUSY        0x01
#define W25Q_STATUS_WEL         0x02

// =====================
// Layout na flash
// =====================

// Área de teste começa na metade da flash
#define W25Q_TEST_DATA_ADDR     (W25Q32_CAPACITY / 2) // 0x200000

// Metadata (2 setores no final)
#define W25Q_LOG_STATE_MAGIC    0x45535441u  // 'ESTA'
#define W25Q_LOG_STATE_VERSION  1u

#define W25Q_LOG_STATE_ADDR     (W25Q32_CAPACITY - 3*W25Q32_SECTOR_SIZE) // 0x3FD000
#define W25Q_TEST_STATE_ADDR    (W25Q32_CAPACITY - 2*W25Q32_SECTOR_SIZE) // 0x3FE000
// 0x3FF000 fica reservado para CONFIG (main.c)
// Estrutura do driver (2 logs separados)
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;

    // NORMAL
    uint32_t current_address;
    uint32_t total_samples;

    // TESTE
    uint32_t test_current_address;
    uint32_t test_total_samples;
} W25Q_t;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t current_address;
    uint32_t total_samples;
    uint32_t crc32; // XOR simples
} W25Q_LogState_t;

// Funções públicas
void W25Q_Init(W25Q_t *dev, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);
uint8_t W25Q_ReadID(W25Q_t *dev);
void W25Q_EraseSector(W25Q_t *dev, uint32_t sector_address);
void W25Q_EraseChip(W25Q_t *dev);
void W25Q_WritePage(W25Q_t *dev, uint32_t address, uint8_t *data, uint16_t size);
void W25Q_ReadData(W25Q_t *dev, uint32_t address, uint8_t *buffer, uint16_t size);
uint8_t W25Q_IsBusy(W25Q_t *dev);
void W25Q_WaitBusy(W25Q_t *dev);

// Log normal
void W25Q_WriteDataSample(W25Q_t *dev, uint8_t *data, uint16_t size);
uint32_t W25Q_GetCurrentAddress(W25Q_t *dev);
uint32_t W25Q_GetTotalSamples(W25Q_t *dev);


void W25Q_LogReset(W25Q_t *dev);

// Log teste
void W25Q_WriteTestSample(W25Q_t *dev, uint8_t *data, uint16_t size);
uint32_t W25Q_GetTestCurrentAddress(W25Q_t *dev);
uint32_t W25Q_GetTestTotalSamples(W25Q_t *dev);
void W25Q_TestReset(W25Q_t *dev);

void W25Q_Reset(W25Q_t *dev);

// Persistência
uint8_t W25Q_LogState_Load(W25Q_t *dev);
void W25Q_LogState_Save(W25Q_t *dev);

uint8_t W25Q_TestState_Load(W25Q_t *dev);
void W25Q_TestState_Save(W25Q_t *dev);

#endif

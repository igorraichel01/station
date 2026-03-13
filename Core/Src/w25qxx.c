/*
 * w25qxx. c
 *
 *  Created on: Jan 2, 2026
 *      Author:  igor
 */

#include "w25qxx.h"
#include <string.h>

// Funções privadas
static void W25Q_CS_Low(W25Q_t *dev);
static void W25Q_CS_High(W25Q_t *dev);
static void W25Q_WriteEnable(W25Q_t *dev);
static uint8_t W25Q_ReadStatusReg(W25Q_t *dev);

static uint32_t W25Q_SimpleCrc_Xor(const uint8_t *data, uint32_t len) {
    uint32_t c = 0;
    for (uint32_t i = 0; i < len; i++) c ^= data[i];
    return c;
}

// ===== FUNÇÕES PRIVADAS =====

static void W25Q_CS_Low(W25Q_t *dev) {
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
}

static void W25Q_CS_High(W25Q_t *dev) {
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

static void W25Q_WriteEnable(W25Q_t *dev) {
    uint8_t cmd = W25Q_WRITE_ENABLE;
    W25Q_CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, &cmd, 1, 100);
    W25Q_CS_High(dev);
}

static uint8_t W25Q_ReadStatusReg(W25Q_t *dev) {
    uint8_t cmd = W25Q_READ_STATUS_REG1;
    uint8_t status;

    W25Q_CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, &cmd, 1, 100);
    HAL_SPI_Receive(dev->hspi, &status, 1, 100);
    W25Q_CS_High(dev);

    return status;
}


// Adicione esta função em w25qxx.c (por exemplo, após W25Q_LogState_Save)

void W25Q_LogReset(W25Q_t *dev) {
    // Apaga só até onde já foi escrito (bem mais rápido que apagar 2MB inteiro)
    uint32_t end = dev->current_address;

    // Se nunca escreveu, apaga 1 setor para garantir "limpo"
    if (end == 0) end = W25Q32_SECTOR_SIZE;

    // arredonda para cima no limite de setor
    uint32_t end_rounded = (end + (W25Q32_SECTOR_SIZE - 1)) & ~(W25Q32_SECTOR_SIZE - 1);

    // limite de segurança: nunca passar para a área de teste
    if (end_rounded > W25Q_TEST_DATA_ADDR) end_rounded = W25Q_TEST_DATA_ADDR;

    for (uint32_t addr = 0; addr < end_rounded; addr += W25Q32_SECTOR_SIZE) {
        W25Q_EraseSector(dev, addr);
    }

    dev->current_address = 0;
    dev->total_samples = 0;

    // Salva o state (preservando as configs em 0x3FF000)
    W25Q_LogState_Save(dev);
}
// ===== FUNÇÕES PÚBLICAS =====

void W25Q_Init(W25Q_t *dev, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin) {
    dev->hspi = hspi;
    dev->cs_port = cs_port;
    dev->cs_pin = cs_pin;

    dev->current_address = 0;
    dev->total_samples = 0;

    dev->test_current_address = W25Q_TEST_DATA_ADDR;
    dev->test_total_samples = 0;

    W25Q_CS_High(dev);
    HAL_Delay(100);
}

uint8_t W25Q_ReadID(W25Q_t *dev) {
    uint8_t cmd[4] = {W25Q_MANUFACTURER_ID, 0x00, 0x00, 0x00};
    uint8_t id[2];

    W25Q_CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, cmd, 4, 100);
    HAL_SPI_Receive(dev->hspi, id, 2, 100);
    W25Q_CS_High(dev);

    return id[1];  // Device ID
}

void W25Q_WaitBusy(W25Q_t *dev) {
    while (W25Q_ReadStatusReg(dev) & W25Q_STATUS_BUSY) {
        HAL_Delay(1);
    }
}

uint8_t W25Q_IsBusy(W25Q_t *dev) {
    return (W25Q_ReadStatusReg(dev) & W25Q_STATUS_BUSY) ? 1 : 0;
}

void W25Q_EraseSector(W25Q_t *dev, uint32_t sector_address) {
    W25Q_WriteEnable(dev);

    uint8_t cmd[4];
    cmd[0] = W25Q_SECTOR_ERASE_4KB;
    cmd[1] = (sector_address >> 16) & 0xFF;
    cmd[2] = (sector_address >> 8) & 0xFF;
    cmd[3] = sector_address & 0xFF;

    W25Q_CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, cmd, 4, 100);
    W25Q_CS_High(dev);

    W25Q_WaitBusy(dev);
}

void W25Q_EraseChip(W25Q_t *dev) {
    W25Q_WriteEnable(dev);

    uint8_t cmd = W25Q_CHIP_ERASE;
    W25Q_CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, &cmd, 1, 100);
    W25Q_CS_High(dev);

    W25Q_WaitBusy(dev);
}

void W25Q_WritePage(W25Q_t *dev, uint32_t address, uint8_t *data, uint16_t size) {
    if (size > W25Q32_PAGE_SIZE) {
        size = W25Q32_PAGE_SIZE;
    }

    W25Q_WriteEnable(dev);

    uint8_t cmd[4];
    cmd[0] = W25Q_PAGE_PROGRAM;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    W25Q_CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, cmd, 4, 100);
    HAL_SPI_Transmit(dev->hspi, data, size, 1000);
    W25Q_CS_High(dev);

    W25Q_WaitBusy(dev);
}

void W25Q_ReadData(W25Q_t *dev, uint32_t address, uint8_t *buffer, uint16_t size) {
    uint8_t cmd[4];
    cmd[0] = W25Q_READ_DATA;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    W25Q_CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, cmd, 4, 100);
    HAL_SPI_Receive(dev->hspi, buffer, size, 1000);
    W25Q_CS_High(dev);
}

static void W25Q_WriteSampleToRegion(W25Q_t *dev,
                                    uint32_t region_start,
                                    uint32_t region_end_exclusive,
                                    uint32_t *address_io,
                                    uint32_t *total_samples_io,
                                    uint8_t *data,
                                    uint16_t size) {
    // apaga setor se está no começo do setor
    if (((*address_io) % W25Q32_SECTOR_SIZE) == 0) {
        W25Q_EraseSector(dev, *address_io);
    }

    uint16_t remaining = size;
    uint16_t offset = 0;

    while (remaining > 0) {
        uint16_t page_offset = (*address_io) % W25Q32_PAGE_SIZE;
        uint16_t bytes_to_write = W25Q32_PAGE_SIZE - page_offset;

        if (bytes_to_write > remaining) {
            bytes_to_write = remaining;
        }

        W25Q_WritePage(dev, *address_io, data + offset, bytes_to_write);

        *address_io += bytes_to_write;
        offset += bytes_to_write;
        remaining -= bytes_to_write;

        // wrap dentro da região
        if (*address_io >= region_end_exclusive) {
            *address_io = region_start;
        }
    }

    (*total_samples_io)++;
}

// ===== LOG NORMAL =====
void W25Q_WriteDataSample(W25Q_t *dev, uint8_t *data, uint16_t size) {
    // Normal: 0 .. W25Q_TEST_DATA_ADDR
    W25Q_WriteSampleToRegion(dev,
                            0,
                            W25Q_TEST_DATA_ADDR,
                            &dev->current_address,
                            &dev->total_samples,
                            data,
                            size);
}

uint32_t W25Q_GetCurrentAddress(W25Q_t *dev) {
    return dev->current_address;
}

uint32_t W25Q_GetTotalSamples(W25Q_t *dev) {
    return dev->total_samples;
}

// ===== LOG TESTE =====
void W25Q_WriteTestSample(W25Q_t *dev, uint8_t *data, uint16_t size) {
    // Teste: W25Q_TEST_DATA_ADDR .. W25Q_TEST_STATE_ADDR
    W25Q_WriteSampleToRegion(dev,
                            W25Q_TEST_DATA_ADDR,
                            W25Q_TEST_STATE_ADDR,
                            &dev->test_current_address,
                            &dev->test_total_samples,
                            data,
                            size);
}

uint32_t W25Q_GetTestCurrentAddress(W25Q_t *dev) {
    return dev->test_current_address;
}

uint32_t W25Q_GetTestTotalSamples(W25Q_t *dev) {
    return dev->test_total_samples;
}

void W25Q_Reset(W25Q_t *dev) {
    dev->current_address = 0;
    dev->total_samples = 0;

    dev->test_current_address = W25Q_TEST_DATA_ADDR;
    dev->test_total_samples = 0;
}
void W25Q_TestReset(W25Q_t *dev) {
    dev->test_current_address = W25Q_TEST_DATA_ADDR;
    dev->test_total_samples = 0;
    W25Q_TestState_Save(dev);
    W25Q_EraseSector(dev, W25Q_TEST_DATA_ADDR);
    // Opcional: apagar o primeiro setor da área de teste para garantir "limpo"
    // (Como você regrava 10 amostras, esse erase já vai acontecer automaticamente
    //  quando test_current_address estiver no início do setor. Mas eu deixo explícito.)

}

// ===== Persistência NORMAL =====
uint8_t W25Q_LogState_Load(W25Q_t *dev) {
    W25Q_LogState_t st;
    W25Q_ReadData(dev, W25Q_LOG_STATE_ADDR, (uint8_t*)&st, sizeof(st));

    if (st.magic != W25Q_LOG_STATE_MAGIC) return 0;
    if (st.version != W25Q_LOG_STATE_VERSION) return 0;

    uint32_t crc_calc = W25Q_SimpleCrc_Xor((uint8_t*)&st, sizeof(st) - sizeof(st.crc32));
    if (st.crc32 != crc_calc) return 0;

    // normal não pode apontar pra área teste
    if (st.current_address >= W25Q_TEST_DATA_ADDR) return 0;

    dev->current_address = st.current_address;
    dev->total_samples   = st.total_samples;
    return 1;
}

// IMPORTANTE (necessário para não apagar config):
// Troque sua função W25Q_LogState_Save atual por esta versão que preserva o setor inteiro.

void W25Q_LogState_Save(W25Q_t *dev) {
    W25Q_LogState_t st;
    st.magic = W25Q_LOG_STATE_MAGIC;
    st.version = W25Q_LOG_STATE_VERSION;
    st.reserved = 0;
    st.current_address = dev->current_address;
    st.total_samples = dev->total_samples;
    st.crc32 = W25Q_SimpleCrc_Xor((uint8_t*)&st, sizeof(st) - sizeof(st.crc32));

    // Lê setor inteiro (onde também estão as CONFIGs em 0x3FF000)
    uint8_t sector[W25Q32_SECTOR_SIZE];
    W25Q_ReadData(dev, W25Q_LOG_STATE_ADDR, sector, W25Q32_SECTOR_SIZE);

    // Atualiza apenas o começo do setor com o LogState
    memcpy(&sector[0], &st, sizeof(st));

    // Apaga e regrava o setor inteiro
    W25Q_EraseSector(dev, W25Q_LOG_STATE_ADDR);
    for (uint32_t off = 0; off < W25Q32_SECTOR_SIZE; off += W25Q32_PAGE_SIZE) {
        W25Q_WritePage(dev, W25Q_LOG_STATE_ADDR + off, &sector[off], W25Q32_PAGE_SIZE);
    }
}
// ===== Persistência TESTE =====
uint8_t W25Q_TestState_Load(W25Q_t *dev) {
    W25Q_LogState_t st;
    W25Q_ReadData(dev, W25Q_TEST_STATE_ADDR, (uint8_t*)&st, sizeof(st));

    if (st.magic != W25Q_LOG_STATE_MAGIC) return 0;
    if (st.version != W25Q_LOG_STATE_VERSION) return 0;

    uint32_t crc_calc = W25Q_SimpleCrc_Xor((uint8_t*)&st, sizeof(st) - sizeof(st.crc32));
    if (st.crc32 != crc_calc) return 0;

    if (st.current_address < W25Q_TEST_DATA_ADDR) return 0;
    if (st.current_address >= W25Q_TEST_STATE_ADDR) return 0;

    dev->test_current_address = st.current_address;
    dev->test_total_samples   = st.total_samples;
    return 1;

}

void W25Q_TestState_Save(W25Q_t *dev) {
    W25Q_LogState_t st;
    st.magic = W25Q_LOG_STATE_MAGIC;
    st.version = W25Q_LOG_STATE_VERSION;
    st.reserved = 0;
    st.current_address = dev->test_current_address;
    st.total_samples = dev->test_total_samples;
    st.crc32 = W25Q_SimpleCrc_Xor((uint8_t*)&st, sizeof(st) - sizeof(st.crc32));

    W25Q_EraseSector(dev, W25Q_TEST_STATE_ADDR);
    W25Q_WritePage(dev, W25Q_TEST_STATE_ADDR, (uint8_t*)&st, sizeof(st));
}

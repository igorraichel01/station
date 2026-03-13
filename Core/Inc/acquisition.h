/*
 * acquisition.h
 *
 *  Created on: Dec 30, 2025
 *      Author: igor
 */

#ifndef ACQUISITION_H
#define ACQUISITION_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// Índices dos intervalos
typedef enum {
    ACQ_2SEC = 0,
    ACQ_5SEC,
    ACQ_10SEC,
    ACQ_30SEC,
    ACQ_1MIN,
    ACQ_5MIN,
    ACQ_10MIN,
    ACQ_30MIN,
    ACQ_1HOUR,
    ACQ_INTERVAL_COUNT
} AcqIntervalIndex_t;

typedef struct {
    uint32_t interval_ms;           // Intervalo atual em milissegundos
    uint32_t next_save_time;        // Timestamp do próximo salvamento
    AcqIntervalIndex_t interval_index;  // Índice do intervalo (0-8)
    bool enabled;                   // Se aquisição está habilitada
} Acquisition_t;

// Funções públicas
void Acquisition_Init(Acquisition_t *acq);
bool Acquisition_IsTimeToSave(Acquisition_t *acq);
void Acquisition_UpdateNextTime(Acquisition_t *acq);
void Acquisition_SetInterval(Acquisition_t *acq, AcqIntervalIndex_t index);
uint32_t Acquisition_GetIntervalMs(AcqIntervalIndex_t index);
const char* Acquisition_GetIntervalString(AcqIntervalIndex_t index);

#endif

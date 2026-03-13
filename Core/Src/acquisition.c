/*
 * acquisition.c
 *
 *  Created on: Dec 30, 2025
 *      Author: igor
 */


#include "acquisition.h"
#include <string.h>

// Tabela de intervalos em milissegundos
static const uint32_t interval_table[ACQ_INTERVAL_COUNT] = {
    2000,      // 2 segundos
    5000,      // 5 segundos (default)
    10000,     // 10 segundos
    30000,     // 30 segundos
    60000,     // 1 minuto
    300000,    // 5 minutos
    600000,    // 10 minutos
    1800000,   // 30 minutos
    3600000    // 1 hora
};

// Tabela de strings para exibição
static const char* interval_strings[ACQ_INTERVAL_COUNT] = {
    " 2 seg ",
    " 5 seg ",
    "10 seg ",
    "30 seg ",
    " 1 min ",
    " 5 min ",
    "10 min ",
    "30 min ",
    " 1 hora"
};

void Acquisition_Init(Acquisition_t *acq) {
    acq->interval_index = ACQ_5SEC;  // Default:  5 segundos
    acq->interval_ms = interval_table[ACQ_5SEC];
    acq->next_save_time = HAL_GetTick() + acq->interval_ms;
    acq->enabled = true;
}

bool Acquisition_IsTimeToSave(Acquisition_t *acq) {
    if (!acq->enabled) {
        return false;
    }

    uint32_t now = HAL_GetTick();

    // Verifica se chegou a hora
    if (now >= acq->next_save_time) {
        return true;
    }

    return false;
}

void Acquisition_UpdateNextTime(Acquisition_t *acq) {
    uint32_t now = HAL_GetTick();

    // Calcula próximo salvamento (evita acúmulo de erro)
    acq->next_save_time += acq->interval_ms;

    // Proteção contra atraso excessivo
    // Se atrasou mais que um intervalo, reseta
    if ((now - acq->next_save_time) > acq->interval_ms) {
        acq->next_save_time = now + acq->interval_ms;
    }
}

void Acquisition_SetInterval(Acquisition_t *acq, AcqIntervalIndex_t index) {
    if (index >= ACQ_INTERVAL_COUNT) {
        return;
    }

    acq->interval_index = index;
    acq->interval_ms = interval_table[index];

    // Recalcula próximo salvamento
    acq->next_save_time = HAL_GetTick() + acq->interval_ms;
}

uint32_t Acquisition_GetIntervalMs(AcqIntervalIndex_t index) {
    if (index >= ACQ_INTERVAL_COUNT) {
        return interval_table[ACQ_5SEC];  // Default
    }
    return interval_table[index];
}

const char* Acquisition_GetIntervalString(AcqIntervalIndex_t index) {
    if (index >= ACQ_INTERVAL_COUNT) {
        return interval_strings[ACQ_5SEC];  // Default
    }
    return interval_strings[index];
}

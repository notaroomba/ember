/**
 ******************************************************************************
 * @file    thermocouple.c
 * @brief   MAX6675 thermocouple driver interface (HAL wrapper)
 * @date    December 16, 2025
 ******************************************************************************
 */

#include "thermocouple.h"
#include "driver_max6675_basic.h"
#include "main.h"

// ---- HAL SPI2 and NSS pin (hardware NSS) ----
extern SPI_HandleTypeDef hspi2;

static volatile float tc_last_temp = 0.0f;
static volatile uint16_t tc_last_raw = 0;
static volatile uint8_t tc_last_status = 1; // 0 = OK, 1 = error

void Thermocouple_Init(void) {
    tc_last_status = max6675_basic_init();
}

float Thermocouple_ReadCelsius(void) {
    float temp = 0.0f;
    uint16_t raw = 0;
    if (max6675_basic_read(&raw, &temp) == 0) {
        tc_last_temp = temp;
        tc_last_raw = raw;
        tc_last_status = 0;
        return temp;
    } else {
        tc_last_status = 1;
        return -1000.0f;
    }
}

uint16_t Thermocouple_GetRaw(void) {
    return tc_last_raw;
}

bool Thermocouple_Ok(void) {
    return (tc_last_status == 0);
}

void Thermocouple_Deinit(void) {
    max6675_basic_deinit();
    tc_last_status = 1;
}

/**
 ******************************************************************************
 * @file    thermocouple.h
 * @brief   MAX6675 thermocouple driver interface (HAL wrapper)
 * @date    December 16, 2025
 ******************************************************************************
 */

#ifndef THERMOCOUPLE_H
#define THERMOCOUPLE_H

#include <stdint.h>
#include <stdbool.h>

void Thermocouple_Init(void);
float Thermocouple_ReadCelsius(void);
uint16_t Thermocouple_GetRaw(void);
bool Thermocouple_Ok(void);
void Thermocouple_Deinit(void);

#endif // THERMOCOUPLE_H

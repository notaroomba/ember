/**
 ******************************************************************************
 * @file    heater.h
 * @brief   Heater PWM control driver
 * @date    December 16, 2025
 ******************************************************************************
 */

#ifndef HEATER_H
#define HEATER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

/**
 * @brief Initialize heater PWM control
 * @note  Call after MX_TIM2_Init()
 */
void Heater_Init(void);

/**
 * @brief Set heater duty cycle
 * @param duty Duty cycle (0-1000, where 1000 = 100%)
 */
void Heater_SetDuty(uint16_t duty);

/**
 * @brief Set heater power as percentage
 * @param percent Power level (0.0 to 100.0)
 */
void Heater_SetPercent(float percent);

/**
 * @brief Turn heater completely off
 */
void Heater_Off(void);

/**
 * @brief Turn heater to full power
 */
void Heater_Full(void);

/**
 * @brief Get current heater duty cycle
 * @return Current duty cycle (0-1000)
 */
uint16_t Heater_GetDuty(void);

/**
 * @brief Get current heater power as percentage
 * @return Current power level (0.0 to 100.0)
 */
float Heater_GetPercent(void);

/**
 * @brief Check if heater is currently on
 * @return true if duty cycle > 0
 */
bool Heater_IsOn(void);

#ifdef __cplusplus
}
#endif

#endif /* HEATER_H */

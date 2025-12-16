/**
 ******************************************************************************
 * @file    heater.c
 * @brief   Heater PWM control driver
 * @date    December 16, 2025
 ******************************************************************************
 */

#include "heater.h"
#include "main.h"
#include "config.h"

/* External timer handle from main.c */
extern TIM_HandleTypeDef htim2;

/* Private variables */
static volatile uint16_t heater_duty = 0;

void Heater_Init(void) {
    // Start PWM on TIM2 CH1
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    
    // Start with heater off (inverted: off = full compare value)
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, HEATER_PWM_RESOLUTION);
    heater_duty = 0;
}

void Heater_SetDuty(uint16_t duty) {
    if (duty > HEATER_PWM_RESOLUTION) {
        duty = HEATER_PWM_RESOLUTION;
    }
    heater_duty = duty;
    
    // PWM is inverted (low = heater on), so we invert the compare value
    // 0% duty -> compare = 1000 (always high, heater off)
    // 100% duty -> compare = 0 (always low, heater on)
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, HEATER_PWM_RESOLUTION - duty);
}

void Heater_SetPercent(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    Heater_SetDuty((uint16_t)(percent * (HEATER_PWM_RESOLUTION / 100.0f)));
}

void Heater_Off(void) {
    Heater_SetDuty(0);
}

void Heater_Full(void) {
    Heater_SetDuty(HEATER_PWM_RESOLUTION);
}

uint16_t Heater_GetDuty(void) {
    return heater_duty;
}

float Heater_GetPercent(void) {
    return (float)heater_duty * 100.0f / HEATER_PWM_RESOLUTION;
}

bool Heater_IsOn(void) {
    return heater_duty > 0;
}

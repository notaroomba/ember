#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32wbxx.h"
#include "stm32wbxx_hal.h"

typedef enum {
    LED_STATUS_PD = 0,
    LED_STATUS_PREHEAT,
    LED_STATUS_SOAK,
    LED_STATUS_REFLOW,
    LED_STATUS_COOL,
    LED_STATUS_STATUS,
    LED_STATUS_GOOD,
    LED_STATUS_ERROR
} LEDs_Type_t;


typedef enum {
    OFF = 0,
    ON,
    BLINKING
} LED_Status_t;

typedef struct {
    LED_Status_t PD_LED;
    LED_Status_t Preheat_LED;
    LED_Status_t Soak_LED;
    LED_Status_t Reflow_LED;
    LED_Status_t Cool_LED;
    LED_Status_t Status_LED;
    LED_Status_t Good_LED;
    LED_Status_t Error_LED;
} LEDState_t;

/* Global LED status - defined in utils.c */
extern LEDState_t LED_Status;
    





void print(const char *format, ...);

void Set_LED_Status(LEDs_Type_t type, LED_Status_t status);
void Update_LEDs(void);
/**
 * @brief Register a GPIO port and pin for a given LED type
 * @param type: LED identifier (LEDs_Type_t)
 * @param port: GPIO port (pass as (void*)GPIOx, e.g., (void*)GPIOA)
 * @param pin: GPIO pin mask (GPIO_PIN_x)
 *
 * Call this from application code (e.g., in main) to associate hardware pins
 * with the logical LEDs used by the utils LED helpers. This avoids including
 * board-specific headers in utils.c.
 */
void LEDs_RegisterPin(LEDs_Type_t type, void* port, uint16_t pin);
/**
 * @brief Set the global blink interval (ms) used for BLINKING LEDs
 * @param ms Blink period in milliseconds; set to 0 to disable blinking
 */
void LEDs_SetBlinkInterval(uint32_t ms);

/**
 * @brief Get current blink interval in milliseconds
 * @return blink interval in ms
 */
uint32_t LEDs_GetBlinkInterval(void);
void LEDs_On(void);
void LEDs_Blinking(void);
void LEDs_Off(void);
/**
 * @brief Scan an I2C bus for devices and print found addresses via USB CDC
 * @param hi2c Pointer to the I2C handle (e.g., &hi2c3)
 * @return number of devices found
 */
int I2C_ScanBus(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */

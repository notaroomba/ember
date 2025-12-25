#include "utils.h"
#include "usbd_cdc_if.h"
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "stm32wbxx.h"
#include "stm32wbxx_hal.h"

void print(const char* format, ...) {
    char buffer[256];
    va_list args;
    
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (len > 0 && len < (int)sizeof(buffer)) {
        CDC_Transmit_FS((uint8_t *)buffer, len);
    }
    HAL_Delay(10); // Small delay to ensure transmission
}

void Set_LED_Status(LEDs_Type_t type, LED_Status_t status) {
    switch (type) {
        case LED_STATUS_PD:
            LED_Status.PD_LED = status;
            break;
        case LED_STATUS_PREHEAT:
            LED_Status.Preheat_LED = status;
            break;
        case LED_STATUS_SOAK:
            LED_Status.Soak_LED = status;
            break;
        case LED_STATUS_REFLOW:
            LED_Status.Reflow_LED = status;
            break;
        case LED_STATUS_COOL:
            LED_Status.Cool_LED = status;
            break;
        case LED_STATUS_STATUS:
            LED_Status.Status_LED = status;
            break;
        case LED_STATUS_GOOD:
            LED_Status.Good_LED = status;
            break;
        case LED_STATUS_ERROR:
            LED_Status.Error_LED = status;
            break;
        default:
            break;
    }
    Update_LEDs();
}

/* File-scope blinking state (non-blocking) */
static volatile uint32_t led_blink_interval_ms = 500; /* default */
static volatile uint32_t led_next_blink_toggle = 0;
static volatile bool led_blink_state = false;

/* Definition of global LED status (single instance) */
LEDState_t LED_Status = { .PD_LED = OFF, .Preheat_LED = OFF, .Soak_LED = OFF,
                         .Reflow_LED = OFF, .Cool_LED = OFF, .Status_LED = OFF,
                         .Good_LED = OFF, .Error_LED = OFF };

/* Static storage for registered LED pins */
/* store port as void* to keep header free of HAL types */
static void* led_gpio_ports[8] = { NULL };
static uint16_t led_gpio_pins[8] = { 0 };

void LEDs_SetBlinkInterval(uint32_t ms) {
    led_blink_interval_ms = ms;
    uint32_t now = HAL_GetTick();
    led_next_blink_toggle = (ms == 0) ? 0U : (now + ms);
    if (ms == 0) {
        led_blink_state = false; /* ensure off while disabled */
    }
}

uint32_t LEDs_GetBlinkInterval(void) {
    return led_blink_interval_ms;
}

void LEDs_RegisterPin(LEDs_Type_t type, void* port, uint16_t pin) {
    if (type >= LED_STATUS_PD && type <= LED_STATUS_ERROR) {
        led_gpio_ports[type] = port;
        led_gpio_pins[type] = pin;
    }
}

void Update_LEDs(void) {
    uint32_t now = HAL_GetTick();

    /* handle blink state toggling non-blocking */
    if (led_blink_interval_ms != 0) {
        if (led_next_blink_toggle == 0) {
            /* first-time init */
            led_next_blink_toggle = now + led_blink_interval_ms;
        } else if ((int32_t)(now - led_next_blink_toggle) >= 0) {
            led_blink_state = !led_blink_state;
            led_next_blink_toggle = now + led_blink_interval_ms;
        }
    }

    /* Update each LED based on its logical status and the blink state */
    for (int t = LED_STATUS_PD; t <= LED_STATUS_ERROR; ++t) {
        GPIO_TypeDef* port = led_gpio_ports[t];
        uint16_t pin = led_gpio_pins[t];
        if (port == NULL || pin == 0) {
            continue; /* not registered */
        }

        LED_Status_t s;
        switch (t) {
            case LED_STATUS_PD: s = LED_Status.PD_LED; break;
            case LED_STATUS_PREHEAT: s = LED_Status.Preheat_LED; break;
            case LED_STATUS_SOAK: s = LED_Status.Soak_LED; break;
            case LED_STATUS_REFLOW: s = LED_Status.Reflow_LED; break;
            case LED_STATUS_COOL: s = LED_Status.Cool_LED; break;
            case LED_STATUS_STATUS: s = LED_Status.Status_LED; break;
            case LED_STATUS_GOOD: s = LED_Status.Good_LED; break;
            case LED_STATUS_ERROR: s = LED_Status.Error_LED; break;
            default: s = OFF; break;
        }

        if (s == ON) {
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
        } else if (s == OFF) {
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        } else if (s == BLINKING) {
            /* blink disabled -> treat as OFF */
            if (led_blink_interval_ms == 0) {
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
            } else {
                HAL_GPIO_WritePin(port, pin, led_blink_state ? GPIO_PIN_RESET : GPIO_PIN_SET);
            }
        }
    }
}

void LEDs_On(void) {
    Set_LED_Status(LED_STATUS_PD, ON);
    Set_LED_Status(LED_STATUS_PREHEAT, ON);
    Set_LED_Status(LED_STATUS_SOAK, ON);
    Set_LED_Status(LED_STATUS_REFLOW, ON);
    Set_LED_Status(LED_STATUS_COOL, ON);
    Set_LED_Status(LED_STATUS_STATUS, ON);
    Set_LED_Status(LED_STATUS_GOOD, ON);
    Set_LED_Status(LED_STATUS_ERROR, ON);
}

void LEDs_Blinking(void) {
    Set_LED_Status(LED_STATUS_PD, BLINKING);
    Set_LED_Status(LED_STATUS_PREHEAT, BLINKING);
    Set_LED_Status(LED_STATUS_SOAK, BLINKING);
    Set_LED_Status(LED_STATUS_REFLOW, BLINKING);
    Set_LED_Status(LED_STATUS_COOL, BLINKING);
    Set_LED_Status(LED_STATUS_STATUS, BLINKING);
    Set_LED_Status(LED_STATUS_GOOD, BLINKING);
    Set_LED_Status(LED_STATUS_ERROR, BLINKING);
}

void LEDs_Off(void) {
    Set_LED_Status(LED_STATUS_PD, OFF);
    Set_LED_Status(LED_STATUS_PREHEAT, OFF);
    Set_LED_Status(LED_STATUS_SOAK, OFF);
    Set_LED_Status(LED_STATUS_REFLOW, OFF);
    Set_LED_Status(LED_STATUS_COOL, OFF);
    Set_LED_Status(LED_STATUS_STATUS, OFF);
    Set_LED_Status(LED_STATUS_GOOD, OFF);
    Set_LED_Status(LED_STATUS_ERROR, OFF);
}

/**
 * @brief Scan an I2C bus for devices and print their 7-bit addresses
 * @param hi2c Pointer to I2C handle (e.g., &hi2c3)
 * @return Number of devices found
 */
int I2C_ScanBus(I2C_HandleTypeDef *hi2c)
{
    int found = 0;
    print("I2C scan starting...\r\n");

    /* Probe all 7-bit addresses 0x01..0x7F */
    for (uint16_t addr = 1; addr < 0xFF; ++addr) {
        HAL_StatusTypeDef res = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(addr << 1), 1, 10);
        if (res == HAL_OK) {
            print("  Device at 0x%02X\r\n", (uint8_t)addr);
            ++found;
        }
    }

    print("I2C scan complete, %d device(s) found\r\n", found);
    return found;
}
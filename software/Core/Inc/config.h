/**
 ******************************************************************************
 * @file    config.h
 * @brief   Centralized configuration for Ember hotplate
 * @date    December 16, 2025
 ******************************************************************************
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * System Configuration
 * ============================================================================ */
#define SYSTEM_CLOCK_HZ         64000000    // 64 MHz system clock

/* ============================================================================
 * Heater PWM Configuration (TIM2 CH1)
 * ============================================================================ */
#define HEATER_TIM_CLOCK_HZ     64000000    // Timer clock frequency
#define HEATER_PWM_FREQ_HZ      10          // PWM frequency (10 Hz for SSR/MOSFET)
#define HEATER_PWM_RESOLUTION   1000        // 0-1000 = 0.0% to 100.0% duty cycle

// Calculated timer values (do not modify)
#define HEATER_TIM_PRESCALER    ((HEATER_TIM_CLOCK_HZ / (HEATER_PWM_FREQ_HZ * HEATER_PWM_RESOLUTION)) - 1)
#define HEATER_TIM_PERIOD       (HEATER_PWM_RESOLUTION - 1)

/* ============================================================================
 * Speaker PWM Configuration (TIM1 CH1)
 * ============================================================================ */
#define SPEAKER_TIM_CLOCK_HZ    64000000    // Timer clock frequency

/* ============================================================================
 * Encoder Configuration (LPTIM1)
 * ============================================================================ */
#define ENCODER_COUNTS_PER_DETENT   2       // Encoder counts per click/detent

/* ============================================================================
 * Button Configuration
 * ============================================================================ */
#define BUTTON_DEBOUNCE_MS      30          // Button debounce time (ms)
#define BUTTON_LONG_PRESS_MS    500         // Long press threshold (ms)

/* ============================================================================
 * I2C Device Addresses
 * ============================================================================ */
#define TPS25730_I2C_ADDRESS    0x20        // USB-PD controller
#define TMP116_I2C_ADDR         0x48        // Temperature sensor (ADD0 = GND)
#define NFC_I2C_ADDRESS         0x55        // NT3H2111 NFC tag (7-bit address, byte=0x08)
#define OLED_I2C_ADDRESS        0xC3        // OLED display (7-bit address, byte=0xC3)

/* ============================================================================
 * NFC Configuration
 * ============================================================================ */
#define NFC_I2C_TIMEOUT_MS      100         // I2C timeout for NFC operations
#define NFC_POLL_INTERVAL_MS    100         // NFC polling interval (ms)

/* ============================================================================
 * Temperature Limits
 * ============================================================================ */
#define TEMP_MAX_CELSIUS        300.0f      // Maximum allowed temperature
#define TEMP_MIN_CELSIUS        0.0f        // Minimum operating temperature
#define TEMP_READ_INTERVAL_MS   500         // Temperature polling interval (ms)

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */

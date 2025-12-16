/**
 ******************************************************************************
 * @file           : tmp116.h
 * @brief          : TMP116/TMP117/TMP119 Temperature Sensor Driver
 * @author         : Lawrence Stanton (C++ original), converted to C
 * @note           : Compatible with TMP116, TMP117, and TMP119
 ******************************************************************************
 */

#ifndef TMP116_H
#define TMP116_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32wbxx.h"
#include "stm32wbxx_hal.h"

/* Device Address - ADD0 pin connected to GND */
#define TMP116_I2C_ADDRESS      (0x48 << 1)  /* 7-bit address 0x48, left-shifted for HAL */

/* Register Addresses */
#define TMP116_REG_TEMP         0x00  /* Temperature Register */
#define TMP116_REG_CONFIG       0x01  /* Configuration Register */
#define TMP116_REG_HIGH_LIM     0x02  /* High Limit Register */
#define TMP116_REG_LOW_LIM      0x03  /* Low Limit Register */
#define TMP116_REG_EEPROM_UL    0x04  /* EEPROM Unlock Register */
#define TMP116_REG_EEPROM1      0x05  /* EEPROM 1 Register */
#define TMP116_REG_EEPROM2      0x06  /* EEPROM 2 Register */
#define TMP116_REG_EEPROM3      0x07  /* EEPROM 3 Register */
#define TMP116_REG_EEPROM4      0x08  /* EEPROM 4 Register */
#define TMP116_REG_DEVICE_ID    0x0F  /* Device ID Register */

/* Expected Device IDs (full 16-bit register value including revision) */
#define TMP116_DEVICE_ID        0x1116  /* TMP116 */
#define TMP117_DEVICE_ID        0x2117  /* TMP117 */
#define TMP119_DEVICE_ID        0x2117  /* TMP119 (same as TMP117) */

/* Temperature Resolution: 0.0078125°C per LSB (same for all variants) */
#define TMP116_TEMP_RESOLUTION  0.0078125f

/* Configuration Register Bit Masks */
#define TMP116_CFG_HIGH_ALERT   0x8000  /* High Alert Flag (RO) */
#define TMP116_CFG_LOW_ALERT    0x4000  /* Low Alert Flag (RO) */
#define TMP116_CFG_DATA_READY   0x2000  /* Data Ready Flag (RO) */
#define TMP116_CFG_EEPROM_BUSY  0x1000  /* EEPROM Busy Flag (RO) */

/* Averaging Mode */
typedef enum {
    TMP116_AVG_1  = 0x0000,
    TMP116_AVG_8  = 0x0020,
    TMP116_AVG_32 = 0x0040,
    TMP116_AVG_64 = 0x0060,
} TMP116_Averages_t;

/* Temperature Conversion Mode */
typedef enum {
    TMP116_MODE_CONTINUOUS = 0x0000,
    TMP116_MODE_SHUTDOWN   = 0x0400,
    TMP116_MODE_ONESHOT    = 0x0C00,
} TMP116_ConversionMode_t;

/* Conversion Cycle Time */
typedef enum {
    TMP116_CONV_15_5MS   = 0x0000,  /* Period increased when Averages > 1 */
    TMP116_CONV_125MS    = 0x0080,  /* Period increased when Averages > 8 */
    TMP116_CONV_250MS    = 0x0100,  /* Period increased when Averages > 8 */
    TMP116_CONV_500MS    = 0x0180,  /* Period increased when Averages > 32 */
    TMP116_CONV_1000MS   = 0x0200,
    TMP116_CONV_4000MS   = 0x0280,
    TMP116_CONV_8000MS   = 0x0300,
    TMP116_CONV_16000MS  = 0x0380,
} TMP116_ConversionCycle_t;

/* Thermal Alert Mode Select */
typedef enum {
    TMP116_ALERT_MODE  = 0x0000,  /* Alert Mode */
    TMP116_THERM_MODE  = 0x0010,  /* Temperature Mode (Therm) */
} TMP116_AlertMode_t;

/* Alert Polarity */
typedef enum {
    TMP116_ALERT_ACTIVE_LOW  = 0x0000,
    TMP116_ALERT_ACTIVE_HIGH = 0x0008,
} TMP116_AlertPolarity_t;

/* Data Ready / Alert Pin Select */
typedef enum {
    TMP116_PIN_ALERT      = 0x0000,  /* ALERT pin reflects Alert Flags */
    TMP116_PIN_DATA_READY = 0x0004,  /* ALERT pin reflects Data Ready Flag */
} TMP116_PinSelect_t;

/* Configuration Structure */
typedef struct {
    /* Read-only status flags */
    bool high_alert_flag;
    bool low_alert_flag;
    bool data_ready_flag;
    bool eeprom_busy_flag;
    
    /* Configurable settings */
    TMP116_ConversionMode_t  conversion_mode;
    TMP116_ConversionCycle_t conversion_cycle;
    TMP116_Averages_t        averages;
    TMP116_AlertMode_t       alert_mode;
    TMP116_AlertPolarity_t   alert_polarity;
    TMP116_PinSelect_t       pin_select;
} TMP116_Config_t;

/* Device Handle */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t            address;
} TMP116_Handle_t;

/* Status codes */
typedef enum {
    TMP116_OK       = 0,
    TMP116_ERROR    = 1,
    TMP116_TIMEOUT  = 2,
    TMP116_BUSY     = 3,
} TMP116_Status_t;

/**
 * @brief Initialize the TMP116 driver
 * @param handle Pointer to TMP116 handle structure
 * @param hi2c Pointer to I2C handle (e.g., &hi2c1)
 * @param address 7-bit I2C address (typically TMP116_I2C_ADDRESS)
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_Init(TMP116_Handle_t *handle, I2C_HandleTypeDef *hi2c, uint8_t address);

/**
 * @brief Read the Device ID
 * @param handle Pointer to TMP116 handle
 * @param device_id Pointer to store device ID (should be 0x1116)
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_GetDeviceID(TMP116_Handle_t *handle, uint16_t *device_id);

/**
 * @brief Read temperature in degrees Celsius
 * @param handle Pointer to TMP116 handle
 * @param temperature Pointer to store temperature value
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_GetTemperature(TMP116_Handle_t *handle, float *temperature);

/**
 * @brief Read raw temperature register value
 * @param handle Pointer to TMP116 handle
 * @param raw_temp Pointer to store raw 16-bit value
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_GetTemperatureRaw(TMP116_Handle_t *handle, int16_t *raw_temp);

/**
 * @brief Read the configuration register
 * @param handle Pointer to TMP116 handle
 * @param config Pointer to configuration structure to populate
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_GetConfig(TMP116_Handle_t *handle, TMP116_Config_t *config);

/**
 * @brief Write configuration to the TMP116
 * @param handle Pointer to TMP116 handle
 * @param config Pointer to configuration structure
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_SetConfig(TMP116_Handle_t *handle, const TMP116_Config_t *config);

/**
 * @brief Check if new temperature data is ready
 * @param handle Pointer to TMP116 handle
 * @param ready Pointer to store result (true if data ready)
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_IsDataReady(TMP116_Handle_t *handle, bool *ready);

/**
 * @brief Set the high temperature limit for alerts
 * @param handle Pointer to TMP116 handle
 * @param temperature High limit in degrees Celsius
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_SetHighLimit(TMP116_Handle_t *handle, float temperature);

/**
 * @brief Set the low temperature limit for alerts
 * @param handle Pointer to TMP116 handle
 * @param temperature Low limit in degrees Celsius
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_SetLowLimit(TMP116_Handle_t *handle, float temperature);

/**
 * @brief Get the high temperature limit
 * @param handle Pointer to TMP116 handle
 * @param temperature Pointer to store high limit in degrees Celsius
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_GetHighLimit(TMP116_Handle_t *handle, float *temperature);

/**
 * @brief Get the low temperature limit
 * @param handle Pointer to TMP116 handle
 * @param temperature Pointer to store low limit in degrees Celsius
 * @return TMP116_OK on success
 */
TMP116_Status_t TMP116_GetLowLimit(TMP116_Handle_t *handle, float *temperature);

/**
 * @brief Trigger a one-shot temperature conversion
 * @param handle Pointer to TMP116 handle
 * @return TMP116_OK on success
 * @note Device must be in shutdown mode for one-shot to work
 */
TMP116_Status_t TMP116_TriggerOneShot(TMP116_Handle_t *handle);

/**
 * @brief Convert raw register value to temperature in Celsius
 * @param raw_value Raw 16-bit register value
 * @return Temperature in degrees Celsius
 */
float TMP116_RawToTemperature(int16_t raw_value);

/**
 * @brief Convert temperature in Celsius to raw register value
 * @param temperature Temperature in degrees Celsius
 * @return Raw 16-bit register value
 */
int16_t TMP116_TemperatureToRaw(float temperature);

#endif /* TMP116_H */

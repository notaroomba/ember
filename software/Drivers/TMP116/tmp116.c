/**
 ******************************************************************************
 * @file           : tmp116.c
 * @brief          : TMP116 Temperature Sensor Driver Implementation
 * @author         : Lawrence Stanton (C++ original), converted to C
 ******************************************************************************
 */

#include "tmp116.h"

/* I2C timeout in milliseconds */
#define TMP116_I2C_TIMEOUT  100

/**
 * @brief Read a 16-bit register from the TMP116
 */
static TMP116_Status_t TMP116_ReadRegister(TMP116_Handle_t *handle, uint8_t reg, uint16_t *value)
{
    uint8_t data[2];
    HAL_StatusTypeDef status;
    
    status = HAL_I2C_Mem_Read(handle->hi2c, handle->address, reg, 
                              I2C_MEMADD_SIZE_8BIT, data, 2, TMP116_I2C_TIMEOUT);
    
    if (status != HAL_OK) {
        return TMP116_ERROR;
    }
    
    /* TMP116 is big-endian */
    *value = ((uint16_t)data[0] << 8) | data[1];
    
    return TMP116_OK;
}

/**
 * @brief Write a 16-bit register to the TMP116
 */
static TMP116_Status_t TMP116_WriteRegister(TMP116_Handle_t *handle, uint8_t reg, uint16_t value)
{
    uint8_t data[2];
    HAL_StatusTypeDef status;
    
    /* TMP116 is big-endian */
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)(value & 0xFF);
    
    status = HAL_I2C_Mem_Write(handle->hi2c, handle->address, reg,
                               I2C_MEMADD_SIZE_8BIT, data, 2, TMP116_I2C_TIMEOUT);
    
    if (status != HAL_OK) {
        return TMP116_ERROR;
    }
    
    return TMP116_OK;
}

TMP116_Status_t TMP116_Init(TMP116_Handle_t *handle, I2C_HandleTypeDef *hi2c, uint8_t address)
{
    if (handle == NULL || hi2c == NULL) {
        return TMP116_ERROR;
    }
    
    handle->hi2c = hi2c;
    handle->address = address;
    
    return TMP116_OK;
}

TMP116_Status_t TMP116_GetDeviceID(TMP116_Handle_t *handle, uint16_t *device_id)
{
    if (handle == NULL || device_id == NULL) {
        return TMP116_ERROR;
    }
    
    return TMP116_ReadRegister(handle, TMP116_REG_DEVICE_ID, device_id);
}

float TMP116_RawToTemperature(int16_t raw_value)
{
    return (float)raw_value * TMP116_TEMP_RESOLUTION;
}

int16_t TMP116_TemperatureToRaw(float temperature)
{
    return (int16_t)(temperature / TMP116_TEMP_RESOLUTION);
}

TMP116_Status_t TMP116_GetTemperature(TMP116_Handle_t *handle, float *temperature)
{
    uint16_t raw;
    TMP116_Status_t status;
    
    if (handle == NULL || temperature == NULL) {
        return TMP116_ERROR;
    }
    
    status = TMP116_ReadRegister(handle, TMP116_REG_TEMP, &raw);
    
    if (status != TMP116_OK) {
        return status;
    }
    
    *temperature = TMP116_RawToTemperature((int16_t)raw);
    
    return TMP116_OK;
}

TMP116_Status_t TMP116_GetTemperatureRaw(TMP116_Handle_t *handle, int16_t *raw_temp)
{
    uint16_t raw;
    TMP116_Status_t status;
    
    if (handle == NULL || raw_temp == NULL) {
        return TMP116_ERROR;
    }
    
    status = TMP116_ReadRegister(handle, TMP116_REG_TEMP, &raw);
    
    if (status != TMP116_OK) {
        return status;
    }
    
    *raw_temp = (int16_t)raw;
    
    return TMP116_OK;
}

/**
 * @brief Parse a configuration register value into a config structure
 */
static void TMP116_ParseConfig(uint16_t reg_value, TMP116_Config_t *config)
{
    config->high_alert_flag  = (reg_value & TMP116_CFG_HIGH_ALERT) != 0;
    config->low_alert_flag   = (reg_value & TMP116_CFG_LOW_ALERT) != 0;
    config->data_ready_flag  = (reg_value & TMP116_CFG_DATA_READY) != 0;
    config->eeprom_busy_flag = (reg_value & TMP116_CFG_EEPROM_BUSY) != 0;
    
    /* Handle special case: Mode 0b10 maps to 0b00 (both continuous) */
    uint16_t mode_bits = reg_value & 0x0C00;
    if (mode_bits == 0x0800) {
        mode_bits = 0x0000;
    }
    config->conversion_mode  = (TMP116_ConversionMode_t)mode_bits;
    
    config->conversion_cycle = (TMP116_ConversionCycle_t)(reg_value & 0x0380);
    config->averages         = (TMP116_Averages_t)(reg_value & 0x0060);
    config->alert_mode       = (TMP116_AlertMode_t)(reg_value & 0x0010);
    config->alert_polarity   = (TMP116_AlertPolarity_t)(reg_value & 0x0008);
    config->pin_select       = (TMP116_PinSelect_t)(reg_value & 0x0004);
}

/**
 * @brief Build a configuration register value from a config structure
 */
static uint16_t TMP116_BuildConfig(const TMP116_Config_t *config)
{
    uint16_t reg_value = 0;
    
    /* Flags are read-only, so we don't include them */
    reg_value |= (uint16_t)config->conversion_mode;
    reg_value |= (uint16_t)config->conversion_cycle;
    reg_value |= (uint16_t)config->averages;
    reg_value |= (uint16_t)config->alert_mode;
    reg_value |= (uint16_t)config->alert_polarity;
    reg_value |= (uint16_t)config->pin_select;
    
    return reg_value;
}

TMP116_Status_t TMP116_GetConfig(TMP116_Handle_t *handle, TMP116_Config_t *config)
{
    uint16_t reg_value;
    TMP116_Status_t status;
    
    if (handle == NULL || config == NULL) {
        return TMP116_ERROR;
    }
    
    status = TMP116_ReadRegister(handle, TMP116_REG_CONFIG, &reg_value);
    
    if (status != TMP116_OK) {
        return status;
    }
    
    TMP116_ParseConfig(reg_value, config);
    
    return TMP116_OK;
}

TMP116_Status_t TMP116_SetConfig(TMP116_Handle_t *handle, const TMP116_Config_t *config)
{
    uint16_t reg_value;
    
    if (handle == NULL || config == NULL) {
        return TMP116_ERROR;
    }
    
    reg_value = TMP116_BuildConfig(config);
    
    return TMP116_WriteRegister(handle, TMP116_REG_CONFIG, reg_value);
}

TMP116_Status_t TMP116_IsDataReady(TMP116_Handle_t *handle, bool *ready)
{
    TMP116_Config_t config;
    TMP116_Status_t status;
    
    if (handle == NULL || ready == NULL) {
        return TMP116_ERROR;
    }
    
    status = TMP116_GetConfig(handle, &config);
    
    if (status != TMP116_OK) {
        return status;
    }
    
    *ready = config.data_ready_flag;
    
    return TMP116_OK;
}

TMP116_Status_t TMP116_SetHighLimit(TMP116_Handle_t *handle, float temperature)
{
    int16_t raw;
    
    if (handle == NULL) {
        return TMP116_ERROR;
    }
    
    raw = TMP116_TemperatureToRaw(temperature);
    
    return TMP116_WriteRegister(handle, TMP116_REG_HIGH_LIM, (uint16_t)raw);
}

TMP116_Status_t TMP116_SetLowLimit(TMP116_Handle_t *handle, float temperature)
{
    int16_t raw;
    
    if (handle == NULL) {
        return TMP116_ERROR;
    }
    
    raw = TMP116_TemperatureToRaw(temperature);
    
    return TMP116_WriteRegister(handle, TMP116_REG_LOW_LIM, (uint16_t)raw);
}

TMP116_Status_t TMP116_GetHighLimit(TMP116_Handle_t *handle, float *temperature)
{
    uint16_t raw;
    TMP116_Status_t status;
    
    if (handle == NULL || temperature == NULL) {
        return TMP116_ERROR;
    }
    
    status = TMP116_ReadRegister(handle, TMP116_REG_HIGH_LIM, &raw);
    
    if (status != TMP116_OK) {
        return status;
    }
    
    *temperature = TMP116_RawToTemperature((int16_t)raw);
    
    return TMP116_OK;
}

TMP116_Status_t TMP116_GetLowLimit(TMP116_Handle_t *handle, float *temperature)
{
    uint16_t raw;
    TMP116_Status_t status;
    
    if (handle == NULL || temperature == NULL) {
        return TMP116_ERROR;
    }
    
    status = TMP116_ReadRegister(handle, TMP116_REG_LOW_LIM, &raw);
    
    if (status != TMP116_OK) {
        return status;
    }
    
    *temperature = TMP116_RawToTemperature((int16_t)raw);
    
    return TMP116_OK;
}

TMP116_Status_t TMP116_TriggerOneShot(TMP116_Handle_t *handle)
{
    TMP116_Config_t config;
    TMP116_Status_t status;
    
    if (handle == NULL) {
        return TMP116_ERROR;
    }
    
    /* Read current config */
    status = TMP116_GetConfig(handle, &config);
    
    if (status != TMP116_OK) {
        return status;
    }
    
    /* Set one-shot mode (this will trigger a conversion if in shutdown mode) */
    config.conversion_mode = TMP116_MODE_ONESHOT;
    
    return TMP116_SetConfig(handle, &config);
}

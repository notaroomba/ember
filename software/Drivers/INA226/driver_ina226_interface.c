/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_ina226_interface_template.c
 * @brief     driver ina226 interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2025-01-29
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2025/01/29  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ina226_interface.h"
#include "stm32wbxx_hal.h"
#include "usbd_cdc_if.h"
#include <stdarg.h>
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
uint8_t ina226_interface_iic_init(void)
{
    // Assume I2C1 is already initialized by CubeMX
    return 0;
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t ina226_interface_iic_deinit(void)
{
    // Not implemented (CubeMX handles deinit)
    return 0;
}

/**
 * @brief      interface iic bus read
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t ina226_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    // addr is 8-bit (already shifted)
    if (HAL_I2C_Mem_Read(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, 100) == HAL_OK) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief     interface iic bus write
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t ina226_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    // addr is 8-bit (already shifted)
    if (HAL_I2C_Mem_Write(&hi2c1, addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, 100) == HAL_OK) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void ina226_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void ina226_interface_debug_print(const char *const fmt, ...)
{
    char buffer[256];
    va_list args;
    
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    if (len > 0 && len < (int)sizeof(buffer)) {
        CDC_Transmit_FS((uint8_t *)buffer, len);
    }
    HAL_Delay(10); 
}

/**
 * @brief     interface receive callback
 * @param[in] type irq type
 * @note      none
 */
void ina226_interface_receive_callback(uint8_t type)
{
    switch (type)
    {
        case INA226_STATUS_SHUNT_VOLTAGE_OVER_VOLTAGE :
        {
            ina226_interface_debug_print("ina226: irq shunt voltage over voltage.\n");
            
            break;
        }
        case INA226_STATUS_SHUNT_VOLTAGE_UNDER_VOLTAGE :
        {
            ina226_interface_debug_print("ina226: irq shunt voltage under voltage.\n");
            
            break;
        }
        case INA226_STATUS_BUS_VOLTAGE_OVER_VOLTAGE :
        {
            ina226_interface_debug_print("ina226: irq bus voltage over voltage.\n");
            
            break;
        }
        case INA226_STATUS_BUS_VOLTAGE_UNDER_VOLTAGE :
        {
            ina226_interface_debug_print("ina226: irq bus voltage under voltage.\n");
            
            break;
        }
        case INA226_STATUS_POWER_OVER_LIMIT :
        {
            ina226_interface_debug_print("ina226: irq power over limit.\n");
            
            break;
        }
        default :
        {
            ina226_interface_debug_print("ina226: unknown code.\n");
            
            break;
        }
    }
}

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
 * @file      driver_max6675_interface_template.c
 * @brief     driver max6675 interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2022-11-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2022/11/30  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_max6675_interface.h"
#include "main.h"
#include "utils.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

extern SPI_HandleTypeDef hspi2;

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   configure SPI2 for 8-bit full-duplex with hardware NSS output
 */
uint8_t max6675_interface_spi_init(void)
{
    // /* Reconfigure SPI2 to ensure 8-bit, full-duplex operation suitable for MAX6675 */
    // if (HAL_SPI_DeInit(&hspi2) != HAL_OK) {
    //     /* continue even if deinit fails; try to re-init anyway */
    // }

    // hspi2.Instance = SPI2;
    // hspi2.Init.Mode = SPI_MODE_MASTER;
    // hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    // hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    // hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    // hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    // hspi2.Init.NSS = SPI_NSS_HARD_OUTPUT; /* use hardware NSS */
    // hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; /* keep fast */
    // hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    // hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    // hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    // hspi2.Init.CRCPolynomial = 7;
    // hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    // hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;

    // if (HAL_SPI_Init(&hspi2) != HAL_OK) {
    //     max6675_interface_debug_print("max6675: HAL_SPI_Init failed\n");
    //     return 1;
    // }

    // /* Ensure SPI is enabled */
    __HAL_SPI_ENABLE(&hspi2);

    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   deinitialize SPI2
 */
uint8_t max6675_interface_spi_deinit(void)
{
    // if (HAL_SPI_DeInit(&hspi2) != HAL_OK) {
    //     max6675_interface_debug_print("max6675: HAL_SPI_DeInit failed\n");
    //     return 1;
    // }

    return 0;
}

/**
 * @brief      interface spi bus read command
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       performs a single SPI transfer (keeps NSS low for the full transfer)
 */
uint8_t max6675_interface_spi_read_cmd(uint8_t *buf, uint16_t len)
{
    if (buf == NULL || len == 0) {
        return 1;
    }

    /* driver only needs 2 bytes for MAX6675; support up to 16 bytes here */
    if (len > 16) {
        return 1;
    }

    uint8_t tx[16];
    memset(tx, 0xFF, len); /* dummy bytes to clock data out */

    if (HAL_SPI_Receive(&hspi2, buf, len, 100) != HAL_OK) {
        max6675_interface_debug_print("max6675: SPI TxRx failed\n");
        return 1;
    }

    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      use HAL_Delay
 */
void max6675_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      formats to a buffer and forwards to the project's print() helper
 */
void max6675_interface_debug_print(const char *const fmt, ...)
{
    char buf[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    print("%s", buf);
}

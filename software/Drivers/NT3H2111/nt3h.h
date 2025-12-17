/*
 * MIT License
 * 
 * Copyright (c) 2019 Sean Farrelly
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
 * File        nt3h.h
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file nt3h.h
 * @brief Driver for NT3H2111 NFC device.
 */

#ifndef _NT3H_H_
#define _NT3H_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

#include "nt3h_defs.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


/* =========== TODO ============ */
/* ADD PROTECTION OF I2C ADDRESS */
/* ============================= */

/*!
 * @brief This API intialises NT3H NFC device.
 * 
 * @param[in] dev : Pointer to nt3h device structure.
 * 
 * @return API status code.
 */
nt3h_status_t nt3h_init(nt3h_dev_t *dev);

/*!
 * @brief This API de-intialises NT3H NFC device.
 * 
 * @param[in] dev : Pointer to nt3h device structure.
 * 
 * @return API status code.
 */
nt3h_status_t nt3h_deinit(nt3h_dev_t *dev);

/*!
 * @brief This API reads a number of bytes from NT3H memory.
 *
 * @note User must ensure the memory region does not include session registers.
 * 
 * @param[in]    dev : Pointer to device structure.
 * @param[in]   addr : Memory address (I2C side).
 * @param[in] offset : Byte offset within memory address.
 * @param[out]  data : Pointer to buffer in which to store bytes.
 * @param[in]    len : Number of bytes to read.
 * 
 * @return API status code.
 */
nt3h_status_t nt3h_read_bytes(nt3h_dev_t *dev, uint16_t addr, uint16_t offset, uint8_t *data, size_t len);

/*!
 * @brief This API write a number of bytes to NT3H memory.
 *
 * @note User must ensure the memory region does not include session registers.
 * 
 * @param[in]    dev : Pointer to device structure.
 * @param[in]   addr : Memory address (I2C side).
 * @param[in] offset : Byte offset within memory address.
 * @param[out]  data : Pointer to buffer containing data to write.
 * @param[in]    len : Number of bytes to write.
 * 
 * @return API status code.
 */
nt3h_status_t nt3h_write_bytes(nt3h_dev_t *dev, uint16_t addr, uint16_t offset, uint8_t *data, size_t len);

/*!
 * @brief This API erases a number of bytes in NT3H memory.
 *
 * @note User must ensure the memory region does not include session registers.
 * 
 * @param[in]    dev : Pointer to device structure.
 * @param[in]   addr : Memory address (I2C side).
 * @param[in] offset : Byte offset within memory address.
 * @param[in]    len : Number of bytes to erase.
 * 
 * @return API status code.
 */
nt3h_status_t nt3h_erase_bytes(nt3h_dev_t *dev, uint16_t addr, uint16_t offset, size_t len);

/*!
 * @brief This API reads the 1-byte value of a Session register within NT3H memory.
 *
 * @param[in]   dev : Pointer to device structure.
 * @param[in]   reg : Register to read.
 * @param[out] data : Pointer to memory in which to store register value.
 * 
 * @return API status code.
 */
nt3h_status_t nt3h_read_register(nt3h_dev_t *dev, uint8_t reg, uint8_t *data);

/*!
 * @brief This API writes the 1-byte value to a Session register within NT3H memory.
 *
 * @param[in]   dev : Pointer to device structure.
 * @param[in]   reg : Register to write to.
 * @param[in]  mask : Mask of current register value before write.
 * @param[out] data : Register value to write.
 * 
 * @return API status code.
 */
nt3h_status_t nt3h_write_register(nt3h_dev_t *dev, uint8_t reg, uint8_t mask, uint8_t data);

/*!
 * @brief This API reads the 1-byte value of a Configuration register within NT3H memory.
 *
 * @param[in]   dev : Pointer to device structure.
 * @param[in]   reg : Register to read.
 * @param[out] data : Pointer to memory in which to store register value.
 * 
 * @return API status code.
 */
nt3h_status_t nt3h_read_config(nt3h_dev_t *dev, uint8_t reg, uint8_t *data);

/*!
 * @brief This API writes the 1-byte value to a Configuration register within NT3H memory.
 *
 * @param[in]   dev : Pointer to device structure.
 * @param[in]   reg : Register to write to.
 * @param[in]  mask : Mask of current register value before write.
 * @param[out] data : Register value to write.
 * 
 * @return Result of API execution status.
 */
nt3h_status_t nt3h_write_config(nt3h_dev_t *dev, uint8_t reg, uint8_t mask, uint8_t data);

/*!
 * @brief This API reads the Capability Container memory region of the device.
 *
 * @param[in]   dev : Pointer to device structure.
 * @param[in]    cc : Pointer to capability container to store values.
 * 
 * @return Result of API execution status.
 */
nt3h_status_t nt3h_read_capability_cont(nt3h_dev_t *dev, capability_cont_t *cc);

/*!
 * @brief This API writes the Capability Container memory region of the device.
 *
 * @param[in]   dev : Pointer to device structure.
 * @param[in]    cc : Pointer to capability container containing values.
 * 
 * @return Result of API execution status.
 */
nt3h_status_t nt3h_write_capability_cont(nt3h_dev_t *dev, capability_cont_t *cc);

nt3h_status_t nt3h_write_addr(nt3h_dev_t *dev, uint8_t addr); /* Write 'Addr' (I2C Address) field */

/*!
 * @brief This API checks if there is currently an NFC field present on the NFC antenna.
 *
 * @note The functionality of the field present pin of the NT3H decice is configurable.
 * 
 * @param[in] dev : Pointer to NT3H device structure.
 * @param[out] is_present : Pointer to boolean to store result.
 * 
 * @return Result of API exeuction status.
 */
nt3h_status_t nt3h_is_field_present(nt3h_dev_t *dev, bool *is_field_present);


/*!
 * @brief This API checks the device is responding to I2C commands.
 *
 * @param[in]   dev : Pointer to device structure.
 * 
 * @return Result of API execution status.
 */
nt3h_status_t nt3h_check(nt3h_dev_t *dev);

/*!
    * @brief This API performs a factory reset of the NT3H device.
    * 
    * @param[in]   dev : Pointer to device structure.
    * 
    * @return Result of API execution status.
*/
nt3h_status_t nt3h_factory_reset(nt3h_dev_t *dev);




// /* Helpful toString log print functions */
// #if DEBUG_NFC == 1
// void NFC_PrintMemory(NFC_HandleTypeDef* nfc, uint16_t blockAddress, uint16_t noOfBlocks);
// void NFC_PrintDeviceProperties(NFC_HandleTypeDef *nfc);
// void NFC_PrintSessionRegisters(NFC_HandleTypeDef *hnfc);
// void NFC_PrintConfigRegisters(NFC_HandleTypeDef *hnfc);
// #endif
// /* ========================================== */


#ifdef __cplusplus
}
#endif /* End of CPP guard */
#endif /* NT3H_H_ */
/** @}*/
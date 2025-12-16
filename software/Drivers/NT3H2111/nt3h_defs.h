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
 * File        nt3h_defs.h
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file nt3h_defs.h
 * @brief Definitions file for NT3H2111 NFC device driver.
 */

#ifndef _NT3H_DEFS_H_
#define _NT3H_DEFS_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Include header files
 */
#include <stdint.h>
#include <stddef.h>


#define NTAG_1K
#define NTAG_2K

#define NT3H_DEFAULT_I2C_ADDRESS        0x40

#define NT3H_MEM_BLOCK_CONFIG_1K        0x3A
#define NT3H_MEM_BLOCK_SESSION_REGS_1K  0xFE

/* Factory default value of memory block 0 */
#define NT3H_FACTORY_VALUE_BLOCK_0  { 0x04, 0x00, 0x00, 0x00, \
                                      0x00, 0x00, 0x00, 0x00, \
                                      0x00, 0x00, 0x00, 0x00, \
                                      0xE1, 0x10, 0x6D, 0x00 }

#define NT3H_FACTORY_VALUE_BLOCK_56 { 0x00, 0x00, 0x00, 0x00, \
                                      0x00, 0x00, 0x00, 0x00, \
                                      0x00, 0x00, 0x00, 0x00, \
                                      0x00, 0x00, 0x00, 0xFF }

#define NT3H_FACTORY_VALUE_BLOCK_57 { 0x00, 0x00, 0x00, 0x00, \
                                      0xFF, 0xFF, 0xFF, 0xFF, \
                                      0x00, 0x00, 0x00, 0x00, \
                                      0x00, 0x00, 0x00, 0x00 }

#define NT3H_FACTORY_VALUE_BLOCK_58 { 0x01, 0x00, 0xF8, 0x48, \
                                      0x08, 0x01, 0x00, 0x00, \
                                      0x00, 0x00, 0x00, 0x00, \
                                      0x00, 0x00, 0x00, 0x00 }


/*!
 * @brief NT3H API status codes
 */
typedef enum {
    NT3H_OK,
    NT3H_E_NULL_PTR,
    NT3H_E_DEV_NOT_FOUND,
    NT3H_E_INVALID_ARGS,
} nt3h_status_t;

/*!
 * @brief Type declarations
 */
typedef nt3h_status_t (*nt3h_com_func_ptr_t)(uint8_t dev_id, uint8_t *data, size_t len);
typedef void          (*nt3h_delay_ms_func_ptr_t)(uint32_t period_ms);

// /*
//  * @brief Structure representation of Capability Container values.
//  */
// typedef struct {
    
//     /*  */
//     uint8_t magic_number;
    
//     /*  */
//     uint8_t version;
    
//     /*  */
//     uint8_t mlen;
    
//     /*  */
//     uint8_t access_control;

// } capability_cont_t;


/*
 * @brief NT3H Device structure.
 */
typedef struct {

    /* Device ID */
    uint16_t dev_id;

    /* User defined I2C write function pointer */
    nt3h_com_func_ptr_t write;

    /* User defined I2C read function pointer */
    nt3h_com_func_ptr_t read;

    /* User defined delay ms function pointer */
    nt3h_delay_ms_func_ptr_t delay_ms;

} nt3h_dev_t;

#ifdef __cplusplus
}
#endif /* End of CPP guard */
#endif /* NT3H_DEFS_H_ */
/** @}*/
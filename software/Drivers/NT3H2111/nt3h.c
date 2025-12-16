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
 * File        nt3h.c
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file nt3h.c
 * @brief Driver for NT3H2111 NFC device.
 */
#include <stdio.h>
#include <string.h>
#include "nt3h.h"

/* NT3H specific definitions */
#define NT3H_I2C_MEM_BLOCK_SIZE      16     /* Number of bytes in I2C memory block */
#define NT3H_SRAM_ADDRESS            0xF8   /* Memory address of SRAM region */
#define NT3H_SRAM_LENGTH             64     /* Length of SRAM region */
#define NT3H_MEMORY_ERASE_VALUE      0x00U  /* Value used to erase memory */

/* Capability Container field masks */
#define CAPABILITY_MAGIC_NUM    0xFF000000U
#define CAPABILITY_VER_ACCESS   0x00FF0000U
#define CAPABILITY_MLEN         0x0000FF00U
#define CAPABILITY_ADDITIONAL   0x000000FFU

/*
 * @brief Structure determining size of r/w operations.
 */
typedef struct
{
    uint8_t data[NT3H_I2C_MEM_BLOCK_SIZE];
} nt3h_block_t;

/* Factory default values of memory blocks 0, 56, 57, 58. */
static const nt3h_block_t factory_value_block_0  = { NT3H_FACTORY_VALUE_BLOCK_0  };
static const nt3h_block_t factory_value_block_56 = { NT3H_FACTORY_VALUE_BLOCK_56 };
static const nt3h_block_t factory_value_block_57 = { NT3H_FACTORY_VALUE_BLOCK_57 };
static const nt3h_block_t factory_value_block_58 = { NT3H_FACTORY_VALUE_BLOCK_58 };


/*!
 * @brief Read block(s) of data from NT3H memory.
 * 
 * @param[in]    dev : Pointer to NT3H device structure.
 * @param[in]   addr : Memory address to read blocks.
 * @param[out] block : Pointer to memory location to store block(s) read.
 * @param[in]    cnt : Number of blocks to read.
 * 
 * @return Result of API execution status.
 */
static nt3h_status_t read_blocks(nt3h_dev_t *dev, uint8_t addr, nt3h_block_t *block, uint8_t cnt);

/*!
 * @brief Write block(s) of data to NT3H memory.
 * 
 * @param[in]    dev : Pointer to NT3H device structure.
 * @param[in]   addr : Memory address to write blocks.
 * @param[out] block : Pointer to memory location containing blocks to write.
 * @param[in]    cnt : Number of blocks to write.
 * 
 * @return Result of API execution status.
 */
static nt3h_status_t write_blocks(nt3h_dev_t *dev, uint8_t addr, const nt3h_block_t *block, uint8_t cnt);

/*!
 * @bried This internal API is used to calculate the number of blocks needed
 * in a r/w operation to ensure all memory regions are covered.
 */
static size_t calculate_blocks_needed(uint16_t offset, size_t len);

/*!
 * @brief This internal API is used to validate the device pointer for
 * null conditions.
 *
 * @param[in] dev : Pointer to NT3H device structure.
 *
 * @return Result of API execution status.
 */
static nt3h_status_t null_ptr_check(nt3h_dev_t *dev);

/*!
 * @brief This API intialises NT3H NFC device.
 */
nt3h_status_t nt3h_init(nt3h_dev_t *dev)
{
    nt3h_status_t rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check if device is responding */
    if ((rslt = nt3h_check(dev)) != NT3H_OK)
        return rslt;

    return rslt;
}

/*!
 * @brief This API de-intialises NT3H NFC device.
 */
nt3h_status_t nt3h_deinit(nt3h_dev_t *dev)
{
    nt3h_status_t rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    return rslt;
}

nt3h_status_t nt3h_factory_reset(nt3h_dev_t *dev)
{
    nt3h_status_t rslt;
    
    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    if((rslt = write_blocks(dev, 0, &factory_value_block_0, 1)) != NT3H_OK)
        return rslt;

    if((rslt = write_blocks(dev, 56, &factory_value_block_56, 1)) != NT3H_OK)
        return rslt;

    if((rslt = write_blocks(dev, 57, &factory_value_block_57, 1)) != NT3H_OK)
        return rslt;

    if((rslt = write_blocks(dev, 58, &factory_value_block_58, 1)) != NT3H_OK)
        return rslt;

    return rslt;
}

/*!
 * @brief This API reads a number of bytes from NT3H memory.
 */
nt3h_status_t nt3h_read_bytes(nt3h_dev_t *dev, uint16_t addr, uint16_t offset, uint8_t *data, size_t len)
{
    nt3h_status_t rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check parameters are valid */
    if (data == NULL || len == 0)
        return NT3H_E_INVALID_ARGS;

    /* Remove any redundant offset if offset is greater than a block length */
    /* Incremembet address by number of blocks and subtract offset by byte equivalent */
    addr   += (offset / NT3H_I2C_MEM_BLOCK_SIZE);
    offset -= (offset / NT3H_I2C_MEM_BLOCK_SIZE) * NT3H_I2C_MEM_BLOCK_SIZE;

    /* Calculate the number of blocks needed to cover this r/w operation. */
    uint8_t blocks_needed = calculate_blocks_needed(offset, len);
    
    nt3h_block_t blocks[blocks_needed];

    if ((rslt = read_blocks(dev, addr, blocks, blocks_needed)) != NT3H_OK)
        return rslt;

    uint8_t *ptr = ((uint8_t *)blocks) + offset;
    memcpy(data, ptr, len);

    return rslt;
}

/*!
 * @brief This API write a number of bytes to NT3H memory.
 */
nt3h_status_t nt3h_write_bytes(nt3h_dev_t *dev, uint16_t addr, uint16_t offset, uint8_t *data, size_t len)
{
    nt3h_status_t rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */
    if (data == NULL || len == 0)
        return NT3H_E_INVALID_ARGS;

    /* Remove any redundant offset if offset is greater than a block length */
    /* Incremembet address by number of blocks and subtract offset by byte equivalent */
    addr   += (offset / NT3H_I2C_MEM_BLOCK_SIZE);
    offset -= (offset / NT3H_I2C_MEM_BLOCK_SIZE) * NT3H_I2C_MEM_BLOCK_SIZE;

    /* Calculate the number of blocks needed to cover this r/w operation. */
    uint8_t blocks_needed = calculate_blocks_needed(offset, len);

    nt3h_block_t blocks[blocks_needed];

    if ((rslt = read_blocks(dev, addr, blocks, blocks_needed)) != NT3H_OK)
        return rslt;

    uint8_t *ptr = ((uint8_t *)blocks) + offset;
    memcpy(ptr, data, len);

    if ((rslt = write_blocks(dev, addr, blocks, blocks_needed)) != NT3H_OK)
        return rslt;

    return rslt;
}

/*!
 * @brief This API erases a number of bytes in NT3H memory.
 */
nt3h_status_t nt3h_erase_bytes(nt3h_dev_t *dev, uint16_t addr, uint16_t offset, size_t len)
{
    nt3h_status_t rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */
    if (len == 0)
        return NT3H_E_INVALID_ARGS;

    /* Remove any redundant offset if offset is greater than a block length */
    /* Incremembet address by number of blocks and subtract offset by byte equivalent */
    addr   += (offset / NT3H_I2C_MEM_BLOCK_SIZE);
    offset -= (offset / NT3H_I2C_MEM_BLOCK_SIZE) * NT3H_I2C_MEM_BLOCK_SIZE;

    /* Calculate the number of blocks needed to cover this r/w operation. */
    uint8_t blocks_needed = calculate_blocks_needed(offset, len);

    nt3h_block_t blocks[blocks_needed];

    if ((rslt = read_blocks(dev, addr, blocks, blocks_needed)) != NT3H_OK)
        return rslt;

    uint8_t *ptr = ((uint8_t *)blocks) + offset;
    memset(ptr, NT3H_MEMORY_ERASE_VALUE, len);

    if ((rslt = write_blocks(dev, addr, blocks, blocks_needed)) != NT3H_OK)
        return rslt;

    return rslt;
}

/*!
 * @brief This API reads the 1-byte value of a Session register within NT3H memory.
 */
nt3h_status_t nt3h_read_register(nt3h_dev_t *dev, uint8_t reg, uint8_t *data)
{
    nt3h_status_t rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check parameters are valid */
    if (data == NULL)
        return NT3H_E_INVALID_ARGS;

    /* Create I2C payload to read from NFC register, according to NFC spec */
    uint8_t buf[2] = { NT3H_MEM_BLOCK_SESSION_REGS_1K, reg };

    if ((rslt = dev->write(dev->dev_id, buf, sizeof(buf))) != NT3H_OK)
        return rslt;

    if ((rslt = dev->read(dev->dev_id, buf, 1)) != NT3H_OK)
        return rslt;

    *data = buf[0];

    return rslt;
}

/*!
 * @brief This API writes the 1-byte value to a Session register within NT3H memory.
 */
nt3h_status_t nt3h_write_register(nt3h_dev_t *dev, uint8_t reg, uint8_t mask, uint8_t data)
{
    nt3h_status_t rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */

    /* Create I2C payload to write to NFC register according to NFC spec */
    uint8_t buf[4] = {NT3H_MEM_BLOCK_SESSION_REGS_1K, reg, mask, data};

    if ((rslt = dev->write(dev->dev_id, buf, sizeof(buf))) != NT3H_OK)
        return rslt;

    return rslt;
}

/*!
 * @brief This API reads the 1-byte value of a Configuration register within NT3H memory.
 */
nt3h_status_t nt3h_read_config(nt3h_dev_t *dev, uint8_t reg, uint8_t *data)
{
    nt3h_status_t rslt;
    nt3h_block_t block;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */

    if ((rslt = read_blocks(dev, NT3H_MEM_BLOCK_CONFIG_1K, &block, 1)) != NT3H_OK)
        return rslt;

    *data = block.data[reg];

    return rslt;
}

/*!
 * @brief This API writes the 1-byte value to a Configuration register within NT3H memory.
 */
nt3h_status_t nt3h_write_config(nt3h_dev_t *dev, uint8_t reg, uint8_t mask, uint8_t data)
{
    nt3h_status_t rslt;
    nt3h_block_t block;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */

    if ((rslt = read_blocks(dev, NT3H_MEM_BLOCK_CONFIG_1K, &block, 1)) != NT3H_OK)
        return rslt;

    block.data[reg] = (block.data[reg] & mask) | data;

    if ((rslt = write_blocks(dev, NT3H_MEM_BLOCK_CONFIG_1K, &block, 1)) != NT3H_OK)
        return rslt;

    return rslt;
}

/**
  * @brief  Program I2C address of NFC device by writing to Addr field
  * @param  dev Pointer to NT3H device structure
  * @param  i2c_addr 7-bit I2C address (e.g., 0x55 for default)
  * @note   Address byte format: [A6 A5 A4 A3 A2 A1 A0 0] - 7-bit addr in upper bits, LSB=0
  * @note   For 0x55: byte value = 0xAA (0x55 << 1)
  * @retval NT3H status
  */
nt3h_status_t nt3h_change_i2c_address(nt3h_dev_t *dev, uint8_t i2c_addr)
{
    nt3h_status_t rslt;
    nt3h_block_t block;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Copy current contents of Block 0 */
    if ((rslt = read_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
        return rslt;

    /* Address byte: 7-bit address in upper 7 bits, LSB must be 0 */
    block.data[0] = (i2c_addr << 1) & 0xFE;

    /* Write new block back to Block 0 */
    if ((rslt = write_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
        return rslt;

    return rslt;
}

/*!
 * @brief This API checks if there is currently an NFC field present on the NFC antenna.
 */
nt3h_status_t nt3h_is_field_present(nt3h_dev_t *dev, bool *is_field_present)
{
    nt3h_status_t rslt;
    uint8_t NS_REG;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    if(is_field_present == NULL)
        return NT3H_E_NULL_PTR;


    if ((rslt = nt3h_read_register(dev, 6, &NS_REG)) != NT3H_OK)
        return rslt;

    *is_field_present = NS_REG & 0x01; /* Mask for RF_FIELD_PRESENT */

    return rslt;
}

// /*!
//  * @brief This API reads the Capability Container memory region of the device.
//  */
// nt3h_status_t nt3h_read_capability_cont(nt3h_dev_t *dev, capability_cont_t *cc)
// {
//     nt3h_status_t rslt;
//     nt3h_block_t block;

//     /* Check for null pointer in device structure */
//     if ((rslt = null_ptr_check(dev)) != NT3H_OK)
//         return rslt;

//     if ((rslt = read_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
//         return rslt;

//     memcpy(cc, &block.data[12], 4);

//     return rslt;
// }

// /*!
//  * @brief This API writes the Capability Container memory region of the device.
//  */
// nt3h_status_t nt3h_write_capability_cont(nt3h_dev_t *dev, capability_cont_t *cc)
// {
//     nt3h_status_t rslt;
//     nt3h_block_t block;

//     /* Check for null pointer in device structure */
//     if ((rslt = null_ptr_check(dev)) != NT3H_OK)
//         return rslt;

//     if ((rslt = read_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
//         return rslt;

//     memcpy(&block.data[12], cc, 4);

//     /* Write new block back to Block 0 */
//     if ((rslt = write_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
//         return rslt;

//     return rslt;
// }

/*!
 * @brief This API checks the device is responding to I2C commands.
 */
nt3h_status_t nt3h_check(nt3h_dev_t *dev)
{
    nt3h_status_t rslt;
    nt3h_block_t block;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    if ((rslt = read_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
        return rslt;

    return rslt;
}

// /* =================================================================== */
// /* =============== Helpful toString Log Print Functions ============== */
// #if DEBUG_NFC == 1

// void NFC_PrintDeviceProperties(NFC_HandleTypeDef *hnfc)
// {
//     NFC_Block block;

//     Log_info("----- NFC Device Details ----");

//     if (NFC_ReadBlocks(hnfc, 0x00, &block, 1, 1000) == NFC_OK)
//     {
//         Log_info("Addr: %02X", block.Data[0]);

//         Log_info("Serial: %02X:%02X:%02X:%02X:%02X:%02X",
//                  block.Data[1], block.Data[2],
//                  block.Data[3], block.Data[4],
//                  block.Data[5], block.Data[6]);

//         Log_info("Static lock bytes: %02X:%02X", block.Data[10], block.Data[11]);
//     }

//     if (NFC_ReadCC(hnfc) == NFC_OK)
//     {
//         Log_info("CC: %02X:%02X:%02X:%02X",
//                  hnfc->CC.MagicNumber,
//                  hnfc->CC.Version,
//                  hnfc->CC.MLEN,
//                  hnfc->CC.AccessControl);

//         Log_info("Memory Size: %d bytes", hnfc->CC.MLEN * 8);
//     }

//     Log_info("-----------------------------");
// }

// void NFC_PrintMemory(NFC_HandleTypeDef *hnfc, uint16_t blockAddress, uint16_t noOfBlocks)
// {
//     Log_info("****** NFC Memory Dump *****");
//     NFC_Block blocks[noOfBlocks];
//     uint8_t buffer[128];

//     if (NFC_ReadBlocks(hnfc, blockAddress, blocks, noOfBlocks, 1000) == NFC_OK)
//     {
//         for (size_t blockNo = 0; blockNo < noOfBlocks; blockNo++)
//         {
//             uint8_t *ptr = &buffer[0];
//             ptr += sprintf((char *)ptr, "0x%02X", blockAddress);

//             for (int i = 0; i < 16; i += 4)
//             {
//                 ptr += sprintf((char *)ptr, "\t");

//                 for (int j = 0; j < 4; j++)
//                 {
//                     ptr += sprintf((char *)ptr, "%02X ", blocks[blockNo].Data[i + j]);
//                 }

//                 ptr += sprintf((char *)ptr, "| ");
//                 for (int j = 0; j < 4; j++)
//                 {
//                     //if((block->Data[i+j] >= 65 && block->Data[i+j] <= 90) ||
//                     //   (block->Data[i+j] >= 97 && block->Data[i+j] <= 122)) {
//                     if (blocks[blockNo].Data[i + j] >= 33 && blocks[blockNo].Data[i + j] <= 126)
//                     {
//                         ptr += sprintf((char *)ptr, "%c", blocks[blockNo].Data[i + j]);
//                     }
//                     else
//                     {
//                         ptr += sprintf((char *)ptr, ".");
//                     }
//                 }
//                 ptr += sprintf((char *)ptr, " |");

//                 Log_info("%s", buffer);
//                 ptr = &buffer[0];
//             }

//             blockAddress++;
//         }

//         Log_info("****************************");
//     }
// }

// void NFC_PrintSessionRegisters(NFC_HandleTypeDef *hnfc)
// {
//     uint8_t SessionReg[8];
//     HAL_StatusTypeDef resp = 0;

//     resp |= NFC_ReadRegister(hnfc, 0xFE, 0, &SessionReg[0], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 1, &SessionReg[1], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 2, &SessionReg[2], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 3, &SessionReg[3], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 4, &SessionReg[4], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 5, &SessionReg[5], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 6, &SessionReg[6], 1000);
//     //   resp |= NFC_ReadRegister(hnfc, 0xFE, 7, &SessionReg[7], 1000);

//     if (resp == HAL_OK)
//     {
//         Log_info("--- NFC Session Registers ---");
//         Log_info("           NC_REG: 0x%02X", SessionReg[0]);
//         Log_info("  LAST_NDEF_BLOCK: 0x%02X", SessionReg[1]);
//         Log_info("SRAM_MIRROR_BLOCK: 0x%02X", SessionReg[2]);
//         Log_info("           WDT_LS: 0x%02X", SessionReg[3]);
//         Log_info("           WDT_MS: 0x%02X", SessionReg[4]);
//         Log_info("    I2C_CLOCK_STR: 0x%02X", SessionReg[5]);
//         Log_info("           NS_REG: 0x%02X", SessionReg[6]);
//         //    Log_info("              RFU: 0x%02X", SessionReg[7]);
//         Log_info("-----------------------------");
//     }
// }

// void NFC_PrintConfigRegisters(NFC_HandleTypeDef *hnfc)
// {
//     NFC_Block configBlock;

//     if (NFC_ReadBlocks(hnfc, 0x3A, &configBlock, 1, 1000) == NFC_OK)
//     {
//         Log_info("---- NFC Config Registers ----");
//         Log_info("           NC_REG: 0x%02X", configBlock.Data[0]);
//         Log_info("  LAST_NDEF_BLOCK: 0x%02X", configBlock.Data[1]);
//         Log_info("SRAM_MIRROR_BLOCK: 0x%02X", configBlock.Data[2]);
//         Log_info("           WDT_LS: 0x%02X", configBlock.Data[3]);
//         Log_info("           WDT_MS: 0x%02X", configBlock.Data[4]);
//         Log_info("    I2C_CLOCK_STR: 0x%02X", configBlock.Data[5]);
//         Log_info("         REG_LOCK: 0x%02X", configBlock.Data[6]);
//         Log_info("              RFU: 0x%02X", configBlock.Data[7]);
//         Log_info("-----------------------------");
//     }
// }
// #endif

/*!
 * @brief Read block(s) of data from NFC memory.
 */
static nt3h_status_t read_blocks(nt3h_dev_t *dev, uint8_t addr, nt3h_block_t *block, uint8_t cnt)
{
    nt3h_status_t rslt;

    uint8_t tx_buffer[NT3H_I2C_MEM_BLOCK_SIZE + 1];

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check parameters are valid */
    if (block == NULL || cnt == 0)
        return NT3H_E_INVALID_ARGS;

    while (cnt > 0U)
    {
        tx_buffer[0] = addr;

        /* Send block address*/
        if((rslt = dev->write(dev->dev_id, tx_buffer, 1)) != NT3H_OK)
            return rslt;

        if((rslt = dev->read(dev->dev_id, (uint8_t*) block, NT3H_I2C_MEM_BLOCK_SIZE)) != NT3H_OK)
            return rslt;

        //if ((rslt = dev->read(dev->dev_id, addr, block->data, NT3H_I2C_MEM_BLOCK_SIZE)) != NT3H_OK)
        //    return rslt;

        block++; /* Move to next block of data */
        addr++;
        cnt--;
    }

    return rslt;
}

/*!
 * @brief Write block(s) of data to NT3H memory.
 */
static nt3h_status_t write_blocks(nt3h_dev_t *dev, uint8_t addr, const nt3h_block_t *block, uint8_t cnt)
{
    nt3h_status_t rslt;
    
    uint8_t tx_buffer[NT3H_I2C_MEM_BLOCK_SIZE + 1];

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check parameters are valid */
    if (block == NULL || cnt == 0)
        return NT3H_E_INVALID_ARGS;

    while (cnt > 0U)
    {
        tx_buffer[0] = addr;
        memcpy(&tx_buffer[1], block->data, NT3H_I2C_MEM_BLOCK_SIZE);
        
        /* Send block address and data */
        if((rslt = dev->write(dev->dev_id, tx_buffer, sizeof(tx_buffer))) != NT3H_OK)
            return rslt;

        // if ((rslt = dev->write(dev->dev_id, addr, block->data, NT3H_I2C_MEM_BLOCK_SIZE)) != NT3H_OK)
        //     return rslt;

        if ((addr > NT3H_SRAM_ADDRESS) && (addr < (NT3H_SRAM_ADDRESS + NT3H_SRAM_LENGTH)))
        {
            /* Address is within SRAM memory region. Time to write 1-block = 0.4ms. */
        }
        else
        {
            /* Address is within EEPROM memory region. Time to write 1-block = 4ms */
            dev->delay_ms(5); /* ALlow time for NFC to complete write to its memory */
        }

        block++; /* Move to next block of data */
        addr++;
        cnt--;
    }

    return rslt;
}

/*!
 * @bried This internal API is used to calculate the number of blocks needed
 * in a r/w operation to ensure all memory regions are covered.
 */
static size_t calculate_blocks_needed(uint16_t offset, size_t len)
{
    /* Calculate how many whole blocks */
    uint16_t blocks_needed = len / NT3H_I2C_MEM_BLOCK_SIZE;
    
    /* Calculate how many remainder bytes */
    uint16_t partial_bytes = len % NT3H_I2C_MEM_BLOCK_SIZE; 

    /* Add extra block for remainder */
    blocks_needed += (partial_bytes) ? 1 : 0;
    
    /* If operation reads into next block, add another block */
    blocks_needed += ((offset + len) > NT3H_I2C_MEM_BLOCK_SIZE) ? 1 : 0;

    return blocks_needed;
}

/*!
 * @brief This internal API is used to validate the device pointer for
 * null conditions.
 */
static nt3h_status_t null_ptr_check(nt3h_dev_t *dev)
{
    nt3h_status_t rslt;

    if (dev == NULL || dev->read == NULL || dev->write == NULL)
    {
        /* Device structure pointer is not valid */
        rslt = NT3H_E_NULL_PTR;
    }
    else
    {
        /* Device structure is fine */
        rslt = NT3H_OK;
    }

    return rslt;
}
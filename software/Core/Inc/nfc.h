/**
 ******************************************************************************
 * @file    nfc.h
 * @brief   NT3H2111 NFC Tag driver wrapper with field detection
 * @date    December 16, 2025
 ******************************************************************************
 */

#ifndef NFC_H
#define NFC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "nt3h.h"

/* NFC Status codes */
typedef enum {
    NFC_OK = 0,
    NFC_ERROR,
    NFC_NO_FIELD,
    NFC_BUSY,
} NFC_Status_t;

/* NFC Event flags */
typedef struct {
    bool field_detected;      // RF field is present
    bool field_changed;       // Field state changed since last check
    bool data_changed;        // Data in user memory changed
} NFC_Events_t;

/**
 * @brief Initialize NFC driver
 * @return NFC_OK on success
 */
NFC_Status_t NFC_Init(void);

/**
 * @brief Poll NFC for events (field detection, data changes)
 * @param events Pointer to event structure to populate
 * @return NFC_OK on success
 */
NFC_Status_t NFC_Poll(NFC_Events_t *events);

/**
 * @brief Check if RF field is detected
 * @return true if field present
 */
bool NFC_IsFieldDetected(void);

/**
 * @brief Read user memory from NFC tag
 * @param offset Byte offset in user memory
 * @param data Buffer to store data
 * @param len Number of bytes to read
 * @return NFC_OK on success
 */
NFC_Status_t NFC_ReadMemory(uint16_t offset, uint8_t *data, size_t len);

/**
 * @brief Write user memory to NFC tag
 * @param offset Byte offset in user memory
 * @param data Data to write
 * @param len Number of bytes to write
 * @return NFC_OK on success
 */
NFC_Status_t NFC_WriteMemory(uint16_t offset, const uint8_t *data, size_t len);

/**
 * @brief Read the NS_REG (session register) to check status flags
 * @param ns_reg Pointer to store NS_REG value
 * @return NFC_OK on success
 */
NFC_Status_t NFC_ReadSessionReg(uint8_t *ns_reg);

/**
 * @brief Get pointer to internal NT3H device (for advanced use)
 * @return Pointer to nt3h_dev_t
 */
nt3h_dev_t* NFC_GetDevice(void);

/**
 * @brief Write an NDEF text record to the NFC tag
 * @param text Null-terminated string to write
 * @return NFC_OK on success, NFC_ERROR on failure
 */
NFC_Status_t NFC_WriteText(const char *text);

/**
 * @brief Read NDEF text record from the NFC tag
 * @param buffer Buffer to store the read text
 * @param max_len Maximum number of characters to read (including null terminator)
 * @return NFC_OK on success, NFC_ERROR on failure or no text record found
 */
NFC_Status_t NFC_ReadText(char *buffer, size_t max_len);

/**
 * @brief Check if NFC tag has valid NDEF content
 * @return true if valid NDEF text exists
 */
bool NFC_HasContent(void);

/**
 * @brief Refresh the internal memory hash (call after external writes)
 */
void NFC_RefreshHash(void);

#ifdef __cplusplus
}
#endif

#endif /* NFC_H */

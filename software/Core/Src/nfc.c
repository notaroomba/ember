/**
 ******************************************************************************
 * @file    nfc.c
 * @brief   NT3H2111 NFC Tag driver wrapper with field detection
 * @date    December 16, 2025
 ******************************************************************************
 */

#include "nfc.h"
#include "main.h"
#include "config.h"
#include "utils.h"
#include <string.h>

/* External I2C handle */
extern I2C_HandleTypeDef hi2c3;

/* NT3H device structure */
static nt3h_dev_t nfc_dev;

/* Previous state for change detection */
static bool last_field_state = false;
static uint8_t last_memory_hash = 0;
static uint32_t nfc_field_lost_ts = 0;
static bool nfc_pending_hash_check = false;

/* User memory start block (after capability container) */
#define NFC_USER_MEMORY_START   0x01
#define NFC_USER_MEMORY_BLOCKS  55    /* NT3H2111 has 56 user blocks (0-55), block 0 is CC */

/* Session register offsets */
#define NT3H_NS_REG             0x00  /* NS_REG offset in session registers */

/* NS_REG bit definitions */
#define NS_REG_NDEF_DATA_READ   (1 << 7)  /* NDEF data read by RF */
#define NS_REG_I2C_LOCKED       (1 << 6)  /* I2C locked by RF */
#define NS_REG_RF_LOCKED        (1 << 5)  /* RF locked by I2C */
#define NS_REG_SRAM_I2C_READY   (1 << 4)  /* SRAM I2C ready */
#define NS_REG_SRAM_RF_READY    (1 << 3)  /* SRAM RF ready */
#define NS_REG_EEPROM_WR_ERR    (1 << 2)  /* EEPROM write error */
#define NS_REG_EEPROM_WR_BUSY   (1 << 1)  /* EEPROM write busy */
#define NS_REG_RF_FIELD         (1 << 0)  /* RF field detected */

/**
 * @brief I2C write callback for NT3H driver
 */
static nt3h_status_t nfc_i2c_write(uint8_t dev_id, uint8_t *data, size_t len)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(&hi2c3, dev_id << 1, data, len, NFC_I2C_TIMEOUT_MS);
    return (status == HAL_OK) ? NT3H_OK : NT3H_E_DEV_NOT_FOUND;
}

/**
 * @brief I2C read callback for NT3H driver
 */
static nt3h_status_t nfc_i2c_read(uint8_t dev_id, uint8_t *data, size_t len)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Receive(&hi2c3, dev_id << 1, data, len, NFC_I2C_TIMEOUT_MS);
    return (status == HAL_OK) ? NT3H_OK : NT3H_E_DEV_NOT_FOUND;
}

/**
 * @brief Delay callback for NT3H driver
 */
static void nfc_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief Compute hash of NDEF content only (not padding/unused bytes)
 */
static uint8_t compute_memory_hash(void)
{
    uint8_t data[64];
    uint8_t hash = 0;
    
    if (nt3h_read_bytes(&nfc_dev, NFC_USER_MEMORY_START, 0, data, sizeof(data)) != NT3H_OK) {
        return 0;
    }
    
    /* Only hash actual NDEF content, not unused bytes */
    /* Format: 03 LL [NDEF message of LL bytes] FE */
    if (data[0] != 0x03) {
        return 0;  /* Not valid NDEF */
    }
    
    uint8_t ndef_len = data[1];
    size_t total_len = 2 + ndef_len + 1;  /* TLV header + content + terminator */
    if (total_len > sizeof(data)) {
        total_len = sizeof(data);
    }
    
    for (size_t i = 0; i < total_len; i++) {
        hash ^= data[i];
        hash = (hash << 1) | (hash >> 7);  /* Rotate */
    }
    
    return hash;
}

/**
 * @brief Disable password protection on the NT3H2111 NFC tag
 */
void NFC_DisablePasswordProtection(void)
{
    uint8_t buf[16];

    // ---------- Block 0x38 ----------
    memset(buf, 0x00, sizeof(buf));

    buf[0] = 0x00;   // ACCESS = 0x00 (no NFC protection)
    buf[3] = 0xFF;   // AUTH0 = 0xFF (disable protection)
    buf[4] = 0x00;   // PWD = 0x00000000
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    buf[8] = 0x00;   // PACK = 0x0000
    buf[9] = 0x00;

    NFC_WriteMemory((0x38 - NFC_USER_MEMORY_START) * 16, buf, 16);
    HAL_Delay(10); // EEPROM write time

    // ---------- Block 0x39 ----------
    memset(buf, 0x00, sizeof(buf));

    buf[0] = 0x00; // PT_I2C = 0b00 (full I2C access)

    NFC_WriteMemory((0x39 - NFC_USER_MEMORY_START) * 16, buf, 16);
    HAL_Delay(10);
}

/**
 * @brief Scan I2C3 for NFC device and return its address
 * @return Found address or 0 if not found
 */
static uint8_t nfc_find_device(void)
{
    /* First check expected address */
    if (HAL_I2C_IsDeviceReady(&hi2c3, NFC_I2C_ADDRESS << 1, 1, 10) == HAL_OK) {
        return NFC_I2C_ADDRESS;
    }
    
    /* Scan for device at other addresses (skip 0x48 = TMP119) */
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (addr == 0x48) continue;
        if (addr == NFC_I2C_ADDRESS) continue;
        
        if (HAL_I2C_IsDeviceReady(&hi2c3, addr << 1, 1, 10) == HAL_OK) {
            print("NFC: Found at 0x%02X\r\n", addr);
            return addr;
        }
    }
    
    return 0;
}

NFC_Status_t NFC_Init(void)
{
    nt3h_status_t status;
    
    /* Find NFC device on I2C3 (scans for any address except TMP119 at 0x48) */
    uint8_t found_addr = nfc_find_device();
    if (found_addr == 0) {
        print("NFC: Not found\r\n");
        return NFC_ERROR;
    }
    
    /* Configure NT3H device */
    nfc_dev.dev_id = found_addr;
    nfc_dev.write = nfc_i2c_write;
    nfc_dev.read = nfc_i2c_read;
    nfc_dev.delay_ms = nfc_delay_ms;
    
    status = nt3h_init(&nfc_dev);
    if (status != NT3H_OK) {
        print("NFC: Init failed\r\n");
        return NFC_ERROR;
    }
    
    /* Change address to configured value if different */
    if (found_addr != NFC_I2C_ADDRESS) {
        // status = nt3h_change_i2c_address(&nfc_dev, NFC_I2C_ADDRESS);
        // if (status != NT3H_OK) {
        //     print("NFC: Address change failed\r\n");
        //     return NFC_ERROR;
        // }
        HAL_Delay(20);  /* Wait for EEPROM write */
        nfc_dev.dev_id = found_addr;
        
        /* Verify new address responds */
        if (HAL_I2C_IsDeviceReady(&hi2c3, found_addr << 1, 3, 100) != HAL_OK) {
            print("NFC: Address verify failed\r\n");
            return NFC_ERROR;
        }
    }
    
    /* Store initial state for change detection */
    last_field_state = NFC_IsFieldDetected();
    last_memory_hash = compute_memory_hash();
    
    print("NFC: OK\r\n");
    return NFC_OK;
}

/**
 * @brief Check if NFC tag has valid NDEF text content
 * @return true if valid NDEF text exists
 */
bool NFC_HasContent(void)
{
    uint8_t data[4];
    if (nt3h_read_bytes(&nfc_dev, 0x01, 0, data, sizeof(data)) != NT3H_OK) {
        return false;
    }
    /* Check for NDEF TLV (0x03) with non-zero length */
    return (data[0] == 0x03 && data[1] > 0);
}

/**
 * @brief Refresh the internal memory hash (call after external writes)
 */
void NFC_RefreshHash(void)
{
    last_memory_hash = compute_memory_hash();
}

NFC_Status_t NFC_Poll(NFC_Events_t *events)
{
    if (events == NULL) {
        return NFC_ERROR;
    }
    
    /* Clear events */
    events->field_detected = false;
    events->field_changed = false;
    events->data_changed = false;
    
    /* Check field detect pin (GPIO) */
    bool current_field = NFC_IsFieldDetected();
    events->field_detected = current_field;
    
    /* Detect field state change */
    if (current_field != last_field_state) {
        events->field_changed = true;
        last_field_state = current_field;
        
        if (current_field) {
            print("NFC: Field detected\r\n");
            /* Cancel any pending hash check when the field returns */
            nfc_pending_hash_check = false;
            nfc_field_lost_ts = 0;
        } else {
            print("NFC: Field lost\r\n");
            /* Start a non-blocking timer; compute hash only after stable absence */
            nfc_pending_hash_check = true;
            nfc_field_lost_ts = HAL_GetTick();
        }
        }

    /* If a hash check is pending and field remains absent for stable period,
     * perform double-read verification and report data change if needed.
     */
    if (nfc_pending_hash_check && !NFC_IsFieldDetected()) {
        const uint32_t stable_ms = 200;
        if ((HAL_GetTick() - nfc_field_lost_ts) >= stable_ms) {
            uint8_t hash1 = compute_memory_hash();
            HAL_Delay(5);
            uint8_t hash2 = compute_memory_hash();

            if (hash1 == hash2 && hash1 != last_memory_hash) {
                events->data_changed = true;
                last_memory_hash = hash1;
                print("NFC: Data changed\r\n");
            }

            /* Clear pending flag after processing */
            nfc_pending_hash_check = false;
            nfc_field_lost_ts = 0;
        }
    } else if (nfc_pending_hash_check && NFC_IsFieldDetected()) {
        /* Field returned before stability period elapsed; cancel check */
        nfc_pending_hash_check = false;
        nfc_field_lost_ts = 0;
    }
    
    
    return NFC_OK;
}

bool NFC_IsFieldDetected(void)
{
    /* Read the current state - active low when field present */
    return (HAL_GPIO_ReadPin(FIELD_DETECT_GPIO_Port, FIELD_DETECT_Pin) == GPIO_PIN_RESET);
}

NFC_Status_t NFC_ReadMemory(uint16_t offset, uint8_t *data, size_t len)
{
    if (data == NULL || len == 0) {
        return NFC_ERROR;
    }
    
    nt3h_status_t status = nt3h_read_bytes(&nfc_dev, NFC_USER_MEMORY_START, offset, data, len);
    
    return (status == NT3H_OK) ? NFC_OK : NFC_ERROR;
}

NFC_Status_t NFC_WriteMemory(uint16_t offset, const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0) {
        return NFC_ERROR;
    }
    
    nt3h_status_t status = nt3h_write_bytes(&nfc_dev, NFC_USER_MEMORY_START, offset, (uint8_t *)data, len);
    
    if (status == NT3H_OK) {
        /* Update hash after write */
        last_memory_hash = compute_memory_hash();
    }
    
    return (status == NT3H_OK) ? NFC_OK : NFC_ERROR;
}

NFC_Status_t NFC_ReadSessionReg(uint8_t *ns_reg)
{
    if (ns_reg == NULL) {
        return NFC_ERROR;
    }
    
    nt3h_status_t status = nt3h_read_register(&nfc_dev, NT3H_NS_REG, ns_reg);
    
    return (status == NT3H_OK) ? NFC_OK : NFC_ERROR;
}

nt3h_dev_t* NFC_GetDevice(void)
{
    return &nfc_dev;
}

NFC_Status_t NFC_WriteText(const char *text)
{
    if (text == NULL) {
        return NFC_ERROR;
    }
    
    size_t text_len = strlen(text);
    if (text_len == 0 || text_len > 200) {
        return NFC_ERROR;
    }
    
    /* NDEF Text Record:
     * 03 LL         - TLV: type=NDEF, length
     * D1 01 PL 54   - Record header: MB|ME|SR|TNF=1, type_len=1, payload_len, type='T'
     * 02 65 6E      - Status byte (UTF-8 + lang_len=2) + "en"
     * [text]        - Text payload
     * FE            - Terminator TLV
     */
    
    uint8_t payload_len = 1 + 2 + text_len;  // status + "en" + text
    uint8_t ndef_msg_len = 4 + payload_len;  // D1 01 PL 54 + payload
    
    /* Build complete NDEF in a 16-byte aligned buffer */
    uint8_t data[64];
    memset(data, 0, sizeof(data));
    
    size_t i = 0;
    data[i++] = 0x03;              // NDEF TLV type
    data[i++] = ndef_msg_len;      // NDEF TLV length
    data[i++] = 0xD1;              // MB=1, ME=1, SR=1, TNF=0x01
    data[i++] = 0x01;              // Type length = 1
    data[i++] = payload_len;       // Payload length
    data[i++] = 0x54;              // Type = 'T' (0x54)
    data[i++] = 0x02;              // Status: UTF-8, lang_len=2
    data[i++] = 0x65;              // 'e'
    data[i++] = 0x6E;              // 'n'
    memcpy(&data[i], text, text_len);
    i += text_len;
    data[i++] = 0xFE;              // Terminator TLV
    
    /* Write to user memory (block 1) */
    if (nt3h_write_bytes(&nfc_dev, 0x01, 0, data, i) != NT3H_OK) {
        return NFC_ERROR;
    }
    
    last_memory_hash = compute_memory_hash();
    return NFC_OK;
}

NFC_Status_t NFC_ReadText(char *buffer, size_t max_len)
{
    if (buffer == NULL || max_len == 0) {
        return NFC_ERROR;
    }
    
    buffer[0] = '\0';
    
    /* Read from user memory (block 1 = page 4) */
    uint8_t data[64];
    if (nt3h_read_bytes(&nfc_dev, 0x01, 0, data, sizeof(data)) != NT3H_OK) {
        return NFC_ERROR;
    }
    
    /* Parse NDEF TLV: 03 LL ... */
    if (data[0] != 0x03) {
        return NFC_ERROR;  // Not NDEF TLV
    }
    
    /* Parse record header: D1 01 PL 54 */
    // data[2] = flags (D1 = MB|ME|SR|TNF=1)
    // data[3] = type length (01)
    // data[4] = payload length
    // data[5] = type (54 = 'T')
    if ((data[2] & 0x07) != 0x01 || data[5] != 0x54) {
        return NFC_ERROR;  // Not a Text record
    }
    
    uint8_t payload_len = data[4];
    
    /* Parse payload: status_byte lang_code text */
    // data[6] = status byte (lang_len in lower 6 bits)
    uint8_t lang_len = data[6] & 0x3F;
    uint8_t text_len = payload_len - 1 - lang_len;
    uint8_t text_start = 7 + lang_len;  // After status + lang
    
    /* Copy text */
    size_t copy_len = (text_len < max_len - 1) ? text_len : max_len - 1;
    memcpy(buffer, &data[text_start], copy_len);
    buffer[copy_len] = '\0';
    
    return NFC_OK;
}

NFC_Status_t NFC_FactoryReset(void)
{
    nt3h_status_t status = nt3h_factory_reset(&nfc_dev);
    return (status == NT3H_OK) ? NFC_OK : NFC_ERROR;
}
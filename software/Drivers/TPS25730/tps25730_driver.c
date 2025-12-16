/**
 ******************************************************************************
 * @file    driver.c
 * @brief   TPS25730 USB PD Controller Driver Implementation
 * @author  Generated from TPS25730 Technical Reference Manual
 * @date    December 16, 2025
 ******************************************************************************
 */

#include "tps25730_driver.h"
#include <string.h>
#include <stdlib.h>
#include <stm32wbxx_hal_i2c.h>

/* I2C Timeout */
#define TPS25730_I2C_TIMEOUT    100

/* Helper macros for bit manipulation */
#define BIT_MASK(bits)          ((1U << (bits)) - 1)
#define GET_BITS(val, pos, bits) (((val) >> (pos)) & BIT_MASK(bits))
#define SET_BITS(val, pos, bits, data) \
    ((val) = ((val) & ~(BIT_MASK(bits) << (pos))) | (((data) & BIT_MASK(bits)) << (pos)))

/* PDO voltage/current conversion macros */
#define VOLTAGE_TO_PDO(mv)      ((mv) / 50)     // Convert mV to 50mV units
#define PDO_TO_VOLTAGE(units)   ((units) * 50)  // Convert 50mV units to mV
#define CURRENT_TO_PDO(ma)      ((ma) / 10)     // Convert mA to 10mA units
#define PDO_TO_CURRENT(units)   ((units) * 10)  // Convert 10mA units to mA
#define POWER_TO_PDO(mw)        ((mw) / 250)    // Convert mW to 250mW units
#define PDO_TO_POWER(units)     ((units) * 250) // Convert 250mW units to mW

/* PPS conversion macros */
#define PPS_VOLTAGE_TO_PDO(mv)      ((mv) / 100)     // Convert mV to 100mV units
#define PPS_PDO_TO_VOLTAGE(units)   ((units) * 100)  // Convert 100mV units to mV
#define PPS_CURRENT_TO_PDO(ma)      ((ma) / 50)      // Convert mA to 50mA units
#define PPS_PDO_TO_CURRENT(units)   ((units) * 50)   // Convert 50mA units to mA

/* Private function prototypes */
static bool TPS25730_ReadRegister(TPS25730_Handle *handle, uint8_t reg_addr, 
                                  uint8_t *data, uint16_t length);
static bool TPS25730_WriteRegister(TPS25730_Handle *handle, uint8_t reg_addr, 
                                   const uint8_t *data, uint16_t length);

/**
 * @brief Initialize TPS25730 driver
 * @param hi2c: Pointer to I2C handle
 * @param device_address: 7-bit I2C device address
 * @return Pointer to initialized handle, or NULL on error
 */
TPS25730_Handle* TPS25730_Init(I2C_HandleTypeDef *hi2c, uint8_t device_address)
{
    if (hi2c == NULL) {
        return NULL;
    }

    TPS25730_Handle *handle = (TPS25730_Handle*)malloc(sizeof(TPS25730_Handle));
    if (handle == NULL) {
        return NULL;
    }

    handle->hi2c = hi2c;
    handle->device_address = device_address << 1; // Convert to 8-bit address

    return handle;
}

/**
 * @brief Deinitialize TPS25730 driver
 * @param handle: Pointer to driver handle
 * @return true if successful
 */
bool TPS25730_DeInit(TPS25730_Handle *handle)
{
    if (handle != NULL) {
        free(handle);
        return true;
    }
    return false;
}

/**
 * @brief Read received source capabilities from charger
 * @param handle: Pointer to driver handle
 * @param caps: Pointer to store source capabilities
 * @return true if successful
 */
bool TPS25730_ReadRxSourceCaps(TPS25730_Handle *handle, TPS25730_SourceCaps *caps)
{
    uint8_t buffer[29];
    
    if (handle == NULL || caps == NULL) {
        return false;
    }

    if (!TPS25730_ReadRegister(handle, TPS25730_REG_RX_SOURCE_CAPS, buffer, sizeof(buffer))) {
        return false;
    }

    // Parse header (byte 0)
    caps->num_valid_pdos = buffer[0] & 0x07;
    
    if (caps->num_valid_pdos > TPS25730_MAX_PDOS) {
        caps->num_valid_pdos = TPS25730_MAX_PDOS;
    }

    // Parse PDOs (4 bytes each, little endian)
    for (uint8_t i = 0; i < caps->num_valid_pdos; i++) {
        uint8_t offset = 1 + (i * 4);
        caps->pdos[i] = (uint32_t)buffer[offset] |
                       ((uint32_t)buffer[offset + 1] << 8) |
                       ((uint32_t)buffer[offset + 2] << 16) |
                       ((uint32_t)buffer[offset + 3] << 24);
    }

    return true;
}

/**
 * @brief Read received sink capabilities from port partner
 * @param handle: Pointer to driver handle
 * @param caps: Pointer to store sink capabilities
 * @return true if successful
 */
bool TPS25730_ReadRxSinkCaps(TPS25730_Handle *handle, TPS25730_SinkCaps *caps)
{
    uint8_t buffer[29];
    
    if (handle == NULL || caps == NULL) {
        return false;
    }

    if (!TPS25730_ReadRegister(handle, TPS25730_REG_RX_SINK_CAPS, buffer, sizeof(buffer))) {
        return false;
    }

    // Parse header (byte 0)
    caps->num_valid_pdos = buffer[0] & 0x07;
    
    if (caps->num_valid_pdos > TPS25730_MAX_PDOS) {
        caps->num_valid_pdos = TPS25730_MAX_PDOS;
    }

    // Parse PDOs (4 bytes each, little endian)
    for (uint8_t i = 0; i < caps->num_valid_pdos; i++) {
        uint8_t offset = 1 + (i * 4);
        caps->pdos[i] = (uint32_t)buffer[offset] |
                       ((uint32_t)buffer[offset + 1] << 8) |
                       ((uint32_t)buffer[offset + 2] << 16) |
                       ((uint32_t)buffer[offset + 3] << 24);
    }

    return true;
}

/**
 * @brief Read transmit sink capabilities (what we advertise)
 * @param handle: Pointer to driver handle
 * @param caps: Pointer to store sink capabilities
 * @return true if successful
 */
bool TPS25730_ReadTxSinkCaps(TPS25730_Handle *handle, TPS25730_SinkCaps *caps)
{
    uint8_t buffer[29];
    
    if (handle == NULL || caps == NULL) {
        return false;
    }

    if (!TPS25730_ReadRegister(handle, TPS25730_REG_TX_SINK_CAPS, buffer, sizeof(buffer))) {
        return false;
    }

    // Parse header (byte 0)
    caps->num_valid_pdos = buffer[0] & 0x07;
    
    if (caps->num_valid_pdos > TPS25730_MAX_PDOS) {
        caps->num_valid_pdos = TPS25730_MAX_PDOS;
    }

    // Parse PDOs (4 bytes each, little endian)
    for (uint8_t i = 0; i < caps->num_valid_pdos; i++) {
        uint8_t offset = 1 + (i * 4);
        caps->pdos[i] = (uint32_t)buffer[offset] |
                       ((uint32_t)buffer[offset + 1] << 8) |
                       ((uint32_t)buffer[offset + 2] << 16) |
                       ((uint32_t)buffer[offset + 3] << 24);
    }

    return true;
}

/**
 * @brief Write transmit sink capabilities
 * @param handle: Pointer to driver handle
 * @param caps: Pointer to sink capabilities to write
 * @return true if successful
 */
bool TPS25730_WriteTxSinkCaps(TPS25730_Handle *handle, const TPS25730_SinkCaps *caps)
{
    uint8_t buffer[29] = {0};
    
    if (handle == NULL || caps == NULL) {
        return false;
    }

    if (caps->num_valid_pdos > TPS25730_MAX_PDOS) {
        return false;
    }

    // Build header (byte 0)
    buffer[0] = caps->num_valid_pdos & 0x07;

    // Build PDOs (4 bytes each, little endian)
    for (uint8_t i = 0; i < caps->num_valid_pdos; i++) {
        uint8_t offset = 1 + (i * 4);
        buffer[offset]     = (uint8_t)(caps->pdos[i] & 0xFF);
        buffer[offset + 1] = (uint8_t)((caps->pdos[i] >> 8) & 0xFF);
        buffer[offset + 2] = (uint8_t)((caps->pdos[i] >> 16) & 0xFF);
        buffer[offset + 3] = (uint8_t)((caps->pdos[i] >> 24) & 0xFF);
    }

    return TPS25730_WriteRegister(handle, TPS25730_REG_TX_SINK_CAPS, buffer, sizeof(buffer));
}

/**
 * @brief Build a fixed supply PDO from parameters
 * @param pdo: Pointer to fixed PDO parameters
 * @return 32-bit PDO value
 */
uint32_t TPS25730_BuildFixedPDO(const TPS25730_FixedPDO *pdo)
{
    uint32_t raw_pdo = 0;

    if (pdo == NULL) {
        return 0;
    }

    // Bits 9:0 - Operational Current (10mA units)
    raw_pdo |= (CURRENT_TO_PDO(pdo->operational_current_ma) & 0x3FF);
    
    // Bits 19:10 - Voltage (50mV units)
    raw_pdo |= ((VOLTAGE_TO_PDO(pdo->voltage_mv) & 0x3FF) << 10);
    
    // Bit 25 - Dual-Role Data
    if (pdo->dual_role_data) {
        raw_pdo |= (1UL << 25);
    }
    
    // Bit 28 - Higher Capability
    if (pdo->higher_capability) {
        raw_pdo |= (1UL << 28);
    }
    
    // Bit 29 - Dual-Role Power
    if (pdo->dual_role_power) {
        raw_pdo |= (1UL << 29);
    }
    
    // Bits 31:30 - Supply Type (00b for Fixed)
    // Already 0, no need to set

    return raw_pdo;
}

/**
 * @brief Build a variable supply PDO from parameters
 * @param pdo: Pointer to variable PDO parameters
 * @return 32-bit PDO value
 */
uint32_t TPS25730_BuildVariablePDO(const TPS25730_VariablePDO *pdo)
{
    uint32_t raw_pdo = 0;

    if (pdo == NULL) {
        return 0;
    }

    // Bits 9:0 - Operational Current (10mA units)
    raw_pdo |= (CURRENT_TO_PDO(pdo->operational_current_ma) & 0x3FF);
    
    // Bits 19:10 - Minimum Voltage (50mV units)
    raw_pdo |= ((VOLTAGE_TO_PDO(pdo->min_voltage_mv) & 0x3FF) << 10);
    
    // Bits 29:20 - Maximum Voltage (50mV units)
    raw_pdo |= ((VOLTAGE_TO_PDO(pdo->max_voltage_mv) & 0x3FF) << 20);
    
    // Bits 31:30 - Supply Type (01b for Variable)
    raw_pdo |= (1UL << 30);

    return raw_pdo;
}

/**
 * @brief Build a battery supply PDO from parameters
 * @param pdo: Pointer to battery PDO parameters
 * @return 32-bit PDO value
 */
uint32_t TPS25730_BuildBatteryPDO(const TPS25730_BatteryPDO *pdo)
{
    uint32_t raw_pdo = 0;

    if (pdo == NULL) {
        return 0;
    }

    // Bits 9:0 - Operational Power (250mW units)
    raw_pdo |= (POWER_TO_PDO(pdo->operational_power_mw) & 0x3FF);
    
    // Bits 19:10 - Minimum Voltage (50mV units)
    raw_pdo |= ((VOLTAGE_TO_PDO(pdo->min_voltage_mv) & 0x3FF) << 10);
    
    // Bits 29:20 - Maximum Voltage (50mV units)
    raw_pdo |= ((VOLTAGE_TO_PDO(pdo->max_voltage_mv) & 0x3FF) << 20);
    
    // Bits 31:30 - Supply Type (10b for Battery)
    raw_pdo |= (2UL << 30);

    return raw_pdo;
}

/**
 * @brief Build an APDO PPS PDO from parameters
 * @param pdo: Pointer to APDO PPS parameters
 * @return 32-bit PDO value
 */
uint32_t TPS25730_BuildAPDO_PPS(const TPS25730_APDO_PPS *pdo)
{
    uint32_t raw_pdo = 0;

    if (pdo == NULL) {
        return 0;
    }

    // Bits 6:0 - Maximum Current (50mA units)
    raw_pdo |= (PPS_CURRENT_TO_PDO(pdo->max_current_ma) & 0x7F);
    
    // Bits 15:8 - Minimum Voltage (100mV units)
    raw_pdo |= ((PPS_VOLTAGE_TO_PDO(pdo->min_voltage_mv) & 0xFF) << 8);
    
    // Bits 24:17 - Maximum Voltage (100mV units)
    raw_pdo |= ((PPS_VOLTAGE_TO_PDO(pdo->max_voltage_mv) & 0xFF) << 17);
    
    // Bits 31:30 - Supply Type (11b for APDO)
    raw_pdo |= (3UL << 30);

    return raw_pdo;
}

/**
 * @brief Parse a fixed supply PDO
 * @param raw_pdo: Raw 32-bit PDO value
 * @param pdo: Pointer to store parsed parameters
 * @return true if PDO is fixed type
 */
bool TPS25730_ParseFixedPDO(uint32_t raw_pdo, TPS25730_FixedPDO *pdo)
{
    if (pdo == NULL || TPS25730_GetPDOType(raw_pdo) != TPS25730_PDO_TYPE_FIXED) {
        return false;
    }

    pdo->operational_current_ma = PDO_TO_CURRENT(raw_pdo & 0x3FF);
    pdo->voltage_mv = PDO_TO_VOLTAGE((raw_pdo >> 10) & 0x3FF);
    pdo->dual_role_data = (raw_pdo & (1UL << 25)) != 0;
    pdo->higher_capability = (raw_pdo & (1UL << 28)) != 0;
    pdo->dual_role_power = (raw_pdo & (1UL << 29)) != 0;

    return true;
}

/**
 * @brief Parse a variable supply PDO
 * @param raw_pdo: Raw 32-bit PDO value
 * @param pdo: Pointer to store parsed parameters
 * @return true if PDO is variable type
 */
bool TPS25730_ParseVariablePDO(uint32_t raw_pdo, TPS25730_VariablePDO *pdo)
{
    if (pdo == NULL || TPS25730_GetPDOType(raw_pdo) != TPS25730_PDO_TYPE_VARIABLE) {
        return false;
    }

    pdo->operational_current_ma = PDO_TO_CURRENT(raw_pdo & 0x3FF);
    pdo->min_voltage_mv = PDO_TO_VOLTAGE((raw_pdo >> 10) & 0x3FF);
    pdo->max_voltage_mv = PDO_TO_VOLTAGE((raw_pdo >> 20) & 0x3FF);

    return true;
}

/**
 * @brief Parse a battery supply PDO
 * @param raw_pdo: Raw 32-bit PDO value
 * @param pdo: Pointer to store parsed parameters
 * @return true if PDO is battery type
 */
bool TPS25730_ParseBatteryPDO(uint32_t raw_pdo, TPS25730_BatteryPDO *pdo)
{
    if (pdo == NULL || TPS25730_GetPDOType(raw_pdo) != TPS25730_PDO_TYPE_BATTERY) {
        return false;
    }

    pdo->operational_power_mw = PDO_TO_POWER(raw_pdo & 0x3FF);
    pdo->min_voltage_mv = PDO_TO_VOLTAGE((raw_pdo >> 10) & 0x3FF);
    pdo->max_voltage_mv = PDO_TO_VOLTAGE((raw_pdo >> 20) & 0x3FF);

    return true;
}

/**
 * @brief Parse an APDO PPS PDO
 * @param raw_pdo: Raw 32-bit PDO value
 * @param pdo: Pointer to store parsed parameters
 * @return true if PDO is APDO PPS type
 */
bool TPS25730_ParseAPDO_PPS(uint32_t raw_pdo, TPS25730_APDO_PPS *pdo)
{
    if (pdo == NULL || TPS25730_GetPDOType(raw_pdo) != TPS25730_PDO_TYPE_APDO_PPS) {
        return false;
    }

    pdo->max_current_ma = PPS_PDO_TO_CURRENT(raw_pdo & 0x7F);
    pdo->min_voltage_mv = PPS_PDO_TO_VOLTAGE((raw_pdo >> 8) & 0xFF);
    pdo->max_voltage_mv = PPS_PDO_TO_VOLTAGE((raw_pdo >> 17) & 0xFF);

    return true;
}

/**
 * @brief Read active contract PDO
 * @param handle: Pointer to driver handle
 * @param contract: Pointer to store contract information
 * @return true if successful
 */
bool TPS25730_ReadActiveContractPDO(TPS25730_Handle *handle, TPS25730_ActiveContract *contract)
{
    uint8_t buffer[6];
    
    if (handle == NULL || contract == NULL) {
        return false;
    }

    if (!TPS25730_ReadRegister(handle, TPS25730_REG_ACTIVE_CONTRACT_PDO, buffer, sizeof(buffer))) {
        return false;
    }

    // Parse active PDO (bytes 1-4 in datasheet = buffer[0-3], little endian)
    contract->active_pdo = (uint32_t)buffer[0] |
                          ((uint32_t)buffer[1] << 8) |
                          ((uint32_t)buffer[2] << 16) |
                          ((uint32_t)buffer[3] << 24);

    // Parse first PDO control bits (bytes 5-6 in datasheet = buffer[4-5], little endian, bits 9:0)
    contract->first_pdo_control_bits = ((uint16_t)buffer[4] |
                                       ((uint16_t)buffer[5] << 8)) & 0x3FF;

    // Determine PDO type and parse accordingly
    contract->type = TPS25730_GetPDOType(contract->active_pdo);
    
    switch (contract->type) {
        case TPS25730_PDO_TYPE_FIXED:
            TPS25730_ParseFixedPDO(contract->active_pdo, &contract->data.fixed);
            break;
        case TPS25730_PDO_TYPE_VARIABLE:
            TPS25730_ParseVariablePDO(contract->active_pdo, &contract->data.variable);
            break;
        case TPS25730_PDO_TYPE_BATTERY:
            TPS25730_ParseBatteryPDO(contract->active_pdo, &contract->data.battery);
            break;
        case TPS25730_PDO_TYPE_APDO_PPS:
            TPS25730_ParseAPDO_PPS(contract->active_pdo, &contract->data.pps);
            break;
    }

    return true;
}

/**
 * @brief Read active contract RDO
 * @param handle: Pointer to driver handle
 * @param rdo: Pointer to store RDO information
 * @return true if successful
 */
bool TPS25730_ReadActiveContractRDO(TPS25730_Handle *handle, TPS25730_ActiveRDO *rdo)
{
    uint8_t buffer[4];
    uint32_t raw_rdo;
    
    if (handle == NULL || rdo == NULL) {
        return false;
    }

    if (!TPS25730_ReadRegister(handle, TPS25730_REG_ACTIVE_CONTRACT_RDO, buffer, sizeof(buffer))) {
        return false;
    }

    // Parse RDO (4 bytes, little endian)
    raw_rdo = (uint32_t)buffer[0] |
              ((uint32_t)buffer[1] << 8) |
              ((uint32_t)buffer[2] << 16) |
              ((uint32_t)buffer[3] << 24);

    // Parse fields
    rdo->object_position = (raw_rdo >> 28) & 0x07;
    rdo->giveback_flag = (raw_rdo & (1UL << 27)) != 0;
    rdo->capability_mismatch = (raw_rdo & (1UL << 26)) != 0;
    rdo->usb_comm_capable = (raw_rdo & (1UL << 25)) != 0;
    rdo->no_usb_suspend = (raw_rdo & (1UL << 24)) != 0;
    rdo->unchunked_supported = (raw_rdo & (1UL << 23)) != 0;
    rdo->operating_current_ma = PDO_TO_CURRENT((raw_rdo >> 10) & 0x3FF);
    rdo->max_min_operating_ma = PDO_TO_CURRENT(raw_rdo & 0x3FF);

    return true;
}

/**
 * @brief Execute Get Source Capabilities task
 * @param handle: Pointer to driver handle
 * @param timeout_ms: Timeout in milliseconds
 * @return Task result code
 */
TPS25730_TaskResult TPS25730_GetSourceCapabilities(TPS25730_Handle *handle, uint32_t timeout_ms)
{
    // This function would require implementation of the 4CC command interface
    // which includes CMD and DATA registers not fully detailed in the provided excerpt
    // Placeholder implementation
    (void)handle;
    (void)timeout_ms;
    return TPS25730_TASK_REJECTED; // Not implemented in this excerpt
}

/**
 * @brief Set a specific sink PDO with voltage and current
 * @param handle: Pointer to driver handle
 * @param pdo_index: PDO index (0-6)
 * @param voltage_mv: Desired voltage in millivolts
 * @param current_ma: Desired current in milliamps
 * @return true if successful
 */
bool TPS25730_SetSinkPDO(TPS25730_Handle *handle, uint8_t pdo_index, 
                         uint32_t voltage_mv, uint32_t current_ma)
{
    TPS25730_SinkCaps caps;
    TPS25730_FixedPDO fixed_pdo;
    
    if (handle == NULL || pdo_index >= TPS25730_MAX_PDOS) {
        return false;
    }

    // Read current sink capabilities
    if (!TPS25730_ReadTxSinkCaps(handle, &caps)) {
        return false;
    }

    // Build new fixed PDO
    fixed_pdo.voltage_mv = voltage_mv;
    fixed_pdo.operational_current_ma = current_ma;
    fixed_pdo.dual_role_data = false;
    fixed_pdo.higher_capability = false;
    fixed_pdo.dual_role_power = false;

    // Update the specified PDO
    caps.pdos[pdo_index] = TPS25730_BuildFixedPDO(&fixed_pdo);
    
    // Ensure num_valid_pdos includes this PDO
    if (caps.num_valid_pdos <= pdo_index) {
        caps.num_valid_pdos = pdo_index + 1;
    }

    // Write back to device
    return TPS25730_WriteTxSinkCaps(handle, &caps);
}

/**
 * @brief Request specific voltage and current from source
 * @param handle: Pointer to driver handle
 * @param voltage_mv: Desired voltage in millivolts
 * @param current_ma: Desired current in milliamps
 * @param timeout_ms: Timeout in milliseconds
 * @return true if successful
 */
bool TPS25730_RequestVoltage(TPS25730_Handle *handle, uint32_t voltage_mv, 
                             uint32_t current_ma, uint32_t timeout_ms)
{
    // Set the first PDO to the desired voltage/current
    if (!TPS25730_SetSinkPDO(handle, 0, voltage_mv, current_ma)) {
        return false;
    }

    // In a full implementation, this would trigger a renegotiation
    // This may require additional registers/tasks not shown in the excerpt
    (void)timeout_ms;
    
    return true;
}

/**
 * @brief Get currently active voltage and current
 * @param handle: Pointer to driver handle
 * @param voltage_mv: Pointer to store voltage in millivolts
 * @param current_ma: Pointer to store current in milliamps
 * @return true if successful
 */
bool TPS25730_GetActiveVoltage(TPS25730_Handle *handle, uint32_t *voltage_mv, 
                               uint32_t *current_ma)
{
    TPS25730_ActiveContract contract;
    
    if (handle == NULL || voltage_mv == NULL || current_ma == NULL) {
        return false;
    }

    if (!TPS25730_ReadActiveContractPDO(handle, &contract)) {
        return false;
    }

    // Extract voltage and current based on PDO type
    switch (contract.type) {
        case TPS25730_PDO_TYPE_FIXED:
            *voltage_mv = contract.data.fixed.voltage_mv;
            *current_ma = contract.data.fixed.operational_current_ma;
            break;
        case TPS25730_PDO_TYPE_VARIABLE:
            *voltage_mv = contract.data.variable.max_voltage_mv; // Use max
            *current_ma = contract.data.variable.operational_current_ma;
            break;
        case TPS25730_PDO_TYPE_BATTERY:
            *voltage_mv = contract.data.battery.max_voltage_mv; // Use max
            *current_ma = 0; // Battery uses power, not current
            break;
        case TPS25730_PDO_TYPE_APDO_PPS:
            *voltage_mv = contract.data.pps.max_voltage_mv; // Use max
            *current_ma = contract.data.pps.max_current_ma;
            break;
        default:
            return false;
    }

    return true;
}

/**
 * @brief Check if a PD contract is currently active
 * @param handle: Pointer to driver handle
 * @return true if contract is active
 */
bool TPS25730_IsContractActive(TPS25730_Handle *handle)
{
    TPS25730_ActiveContract contract;
    
    if (handle == NULL) {
        return false;
    }

    if (!TPS25730_ReadActiveContractPDO(handle, &contract)) {
        return false;
    }

    // If active_pdo is non-zero, a contract exists
    return (contract.active_pdo != 0);
}

/**
 * @brief Get complete status including contract and RDO
 * @param handle: Pointer to driver handle
 * @param contract: Pointer to store contract information (can be NULL)
 * @param rdo: Pointer to store RDO information (can be NULL)
 * @return true if successful
 */
bool TPS25730_GetStatus(TPS25730_Handle *handle, TPS25730_ActiveContract *contract,
                       TPS25730_ActiveRDO *rdo)
{
    if (handle == NULL) {
        return false;
    }

    bool success = true;

    if (contract != NULL) {
        success = success && TPS25730_ReadActiveContractPDO(handle, contract);
    }

    if (rdo != NULL) {
        success = success && TPS25730_ReadActiveContractRDO(handle, rdo);
    }

    return success;
}

/* Private Functions */

/**
 * @brief Read data from TPS25730 register
 * @param handle: Pointer to driver handle
 * @param reg_addr: Register address
 * @param data: Buffer to store read data
 * @param length: Number of bytes to read (not including byte count)
 * @return true if successful
 * 
 * Note: TPS25730 uses Unique Address Interface Protocol where the first
 * byte returned is always the Byte Count. This function handles that
 * automatically - the caller receives only the data bytes.
 */
static bool TPS25730_ReadRegister(TPS25730_Handle *handle, uint8_t reg_addr, 
                                  uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status;
    uint8_t temp_buffer[64]; // Temporary buffer to include byte count
    
    if (handle == NULL || data == NULL || length > 63) {
        return false;
    }

    // Read length + 1 bytes (first byte is the byte count per TPS25730 protocol)
    status = HAL_I2C_Mem_Read(handle->hi2c, handle->device_address, reg_addr,
                             I2C_MEMADD_SIZE_8BIT, temp_buffer, length + 1, TPS25730_I2C_TIMEOUT);

    if (status != HAL_OK) {
        return false;
    }
    
    // First byte is the byte count - skip it and copy the rest
    // temp_buffer[0] = byte count (should equal 'length')
    // temp_buffer[1..length] = actual data
    for (uint16_t i = 0; i < length; i++) {
        data[i] = temp_buffer[i + 1];
    }

    return true;
}

/**
 * @brief Write data to TPS25730 register
 * @param handle: Pointer to driver handle
 * @param reg_addr: Register address
 * @param data: Data to write
 * @param length: Number of bytes to write (not including byte count)
 * @return true if successful
 * 
 * Note: TPS25730 uses Unique Address Interface Protocol where the first
 * byte written must be the Byte Count. This function handles that
 * automatically.
 */
static bool TPS25730_WriteRegister(TPS25730_Handle *handle, uint8_t reg_addr, 
                                   const uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status;
    uint8_t temp_buffer[64]; // Temporary buffer to include byte count
    
    if (handle == NULL || data == NULL || length > 63) {
        return false;
    }

    // First byte is the byte count per TPS25730 protocol
    temp_buffer[0] = (uint8_t)length;
    
    // Copy data after byte count
    for (uint16_t i = 0; i < length; i++) {
        temp_buffer[i + 1] = data[i];
    }

    status = HAL_I2C_Mem_Write(handle->hi2c, handle->device_address, reg_addr,
                              I2C_MEMADD_SIZE_8BIT, temp_buffer, length + 1, 
                              TPS25730_I2C_TIMEOUT);

    return (status == HAL_OK);
}

/**
 * @brief Get PDO type from raw PDO value
 * @param pdo: Raw 32-bit PDO value
 * @return PDO type
 */
TPS25730_PDO_Type TPS25730_GetPDOType(uint32_t pdo)
{
    return (TPS25730_PDO_Type)((pdo >> 30) & 0x03);
}

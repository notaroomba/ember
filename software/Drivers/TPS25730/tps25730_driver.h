/**
 ******************************************************************************
 * @file    driver.h
 * @brief   TPS25730 USB PD Controller Driver Header
 * @author  Generated from TPS25730 Technical Reference Manual
 * @date    December 16, 2025
 ******************************************************************************
 * @attention
 *
 * This driver implements the TPS25730 USB PD controller interface including:
 * - Reading/Writing Sink Capabilities
 * - Monitoring active contracts and PDOs
 * - Executing PD tasks (Get Source Capabilities)
 * - Configuring voltage and current requirements
 *
 ******************************************************************************
 */

#ifndef TPS25730_DRIVER_H
#define TPS25730_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32wbxx.h"
#include "stm32wbxx_hal.h"

/* Register Addresses */
#define TPS25730_REG_RX_SOURCE_CAPS         0x30
#define TPS25730_REG_RX_SINK_CAPS           0x31
#define TPS25730_REG_TX_SINK_CAPS           0x33
#define TPS25730_REG_ACTIVE_CONTRACT_PDO    0x34
#define TPS25730_REG_ACTIVE_CONTRACT_RDO    0x35

/* Task 4CC Codes */
#define TPS25730_TASK_GET_SOURCE_CAPS       0x43527347  // 'GSrC'

/* Maximum number of PDOs */
#define TPS25730_MAX_PDOS                   7

/* PDO Supply Types */
typedef enum {
    TPS25730_PDO_TYPE_FIXED     = 0x00,
    TPS25730_PDO_TYPE_VARIABLE  = 0x01,
    TPS25730_PDO_TYPE_BATTERY   = 0x02,
    TPS25730_PDO_TYPE_APDO_PPS  = 0x03
} TPS25730_PDO_Type;

/* Task Result Codes */
typedef enum {
    TPS25730_TASK_SUCCESS           = 0x0,
    TPS25730_TASK_TIMEOUT           = 0x1,
    TPS25730_TASK_REJECTED          = 0x3,
    TPS25730_TASK_RX_BUFFER_LOCKED  = 0x4
} TPS25730_TaskResult;

/* Fixed Supply PDO Structure */
typedef struct {
    uint32_t operational_current_ma;    // 10mA units (bits 9:0)
    uint32_t voltage_mv;                // 50mV units (bits 19:10)
    bool dual_role_data;                // bit 25
    bool higher_capability;             // bit 28
    bool dual_role_power;               // bit 29
} TPS25730_FixedPDO;

/* Variable Supply PDO Structure */
typedef struct {
    uint32_t operational_current_ma;    // 10mA units (bits 9:0)
    uint32_t min_voltage_mv;            // 50mV units (bits 19:10)
    uint32_t max_voltage_mv;            // 50mV units (bits 29:20)
} TPS25730_VariablePDO;

/* Battery Supply PDO Structure */
typedef struct {
    uint32_t operational_power_mw;      // 250mW units (bits 9:0)
    uint32_t min_voltage_mv;            // 50mV units (bits 19:10)
    uint32_t max_voltage_mv;            // 50mV units (bits 29:20)
} TPS25730_BatteryPDO;

/* APDO (PPS) PDO Structure */
typedef struct {
    uint32_t max_current_ma;            // 50mA units (bits 6:0)
    uint32_t min_voltage_mv;            // 100mV units (bits 15:8)
    uint32_t max_voltage_mv;            // 100mV units (bits 24:17)
} TPS25730_APDO_PPS;

/* Active Contract PDO Information */
typedef struct {
    uint32_t active_pdo;                // Full 32-bit PDO
    uint16_t first_pdo_control_bits;    // Bits 29:20 of first PDO
    TPS25730_PDO_Type type;
    union {
        TPS25730_FixedPDO fixed;
        TPS25730_VariablePDO variable;
        TPS25730_BatteryPDO battery;
        TPS25730_APDO_PPS pps;
    } data;
} TPS25730_ActiveContract;

/* Active RDO Information */
typedef struct {
    uint8_t object_position;            // Which PDO is selected (1-7)
    bool giveback_flag;
    bool capability_mismatch;
    bool usb_comm_capable;
    bool no_usb_suspend;
    bool unchunked_supported;
    uint32_t operating_current_ma;      // Or voltage for PPS
    uint32_t max_min_operating_ma;      // Or voltage for PPS
} TPS25730_ActiveRDO;

/* Sink Capabilities Structure */
typedef struct {
    uint8_t num_valid_pdos;
    uint32_t pdos[TPS25730_MAX_PDOS];   // Raw PDO values
} TPS25730_SinkCaps;

/* Driver Handle Structure */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t device_address;
} TPS25730_Handle;

/* Initialization and Configuration Functions */
TPS25730_Handle* TPS25730_Init(I2C_HandleTypeDef *hi2c, uint8_t device_address);
bool TPS25730_DeInit(TPS25730_Handle *handle);

/* Source Capabilities Structure (from charger) */
typedef struct {
    uint8_t num_valid_pdos;
    uint32_t pdos[TPS25730_MAX_PDOS];   // Raw PDO values
} TPS25730_SourceCaps;

/* Source Capabilities Functions */
bool TPS25730_ReadRxSourceCaps(TPS25730_Handle *handle, TPS25730_SourceCaps *caps);

/* Sink Capabilities Functions */
bool TPS25730_ReadRxSinkCaps(TPS25730_Handle *handle, TPS25730_SinkCaps *caps);
bool TPS25730_ReadTxSinkCaps(TPS25730_Handle *handle, TPS25730_SinkCaps *caps);
bool TPS25730_WriteTxSinkCaps(TPS25730_Handle *handle, const TPS25730_SinkCaps *caps);

/* PDO Building Helper Functions */
uint32_t TPS25730_BuildFixedPDO(const TPS25730_FixedPDO *pdo);
uint32_t TPS25730_BuildVariablePDO(const TPS25730_VariablePDO *pdo);
uint32_t TPS25730_BuildBatteryPDO(const TPS25730_BatteryPDO *pdo);
uint32_t TPS25730_BuildAPDO_PPS(const TPS25730_APDO_PPS *pdo);

/* PDO Parsing Helper Functions */
bool TPS25730_ParseFixedPDO(uint32_t raw_pdo, TPS25730_FixedPDO *pdo);
bool TPS25730_ParseVariablePDO(uint32_t raw_pdo, TPS25730_VariablePDO *pdo);
bool TPS25730_ParseBatteryPDO(uint32_t raw_pdo, TPS25730_BatteryPDO *pdo);
bool TPS25730_ParseAPDO_PPS(uint32_t raw_pdo, TPS25730_APDO_PPS *pdo);
TPS25730_PDO_Type TPS25730_GetPDOType(uint32_t pdo);

/* Active Contract Functions */
bool TPS25730_ReadActiveContractPDO(TPS25730_Handle *handle, TPS25730_ActiveContract *contract);
bool TPS25730_ReadActiveContractRDO(TPS25730_Handle *handle, TPS25730_ActiveRDO *rdo);

/* Task Functions */
TPS25730_TaskResult TPS25730_GetSourceCapabilities(TPS25730_Handle *handle, uint32_t timeout_ms);

/* High-Level Configuration Functions */
bool TPS25730_SetSinkPDO(TPS25730_Handle *handle, uint8_t pdo_index, 
                         uint32_t voltage_mv, uint32_t current_ma);
bool TPS25730_RequestVoltage(TPS25730_Handle *handle, uint32_t voltage_mv, 
                             uint32_t current_ma, uint32_t timeout_ms);
bool TPS25730_GetActiveVoltage(TPS25730_Handle *handle, uint32_t *voltage_mv, 
                               uint32_t *current_ma);

/* Status Functions */
bool TPS25730_IsContractActive(TPS25730_Handle *handle);
bool TPS25730_GetStatus(TPS25730_Handle *handle, TPS25730_ActiveContract *contract,
                       TPS25730_ActiveRDO *rdo);

#ifdef __cplusplus
}
#endif

#endif /* TPS25730_DRIVER_H */

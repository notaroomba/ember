/**
 ******************************************************************************
 * @file    rtd.c
 * @brief   RTD (MAX31865) temperature sensor driver implementation
 * @date    December 16, 2025
 ******************************************************************************
 */

#include "rtd.h"
#include "max31865.h"
#include "main.h"
#include <stdbool.h>

// ---- Hardware config: update as needed ----
#define RTD_RREF_OHMS      430.0f   // Reference resistor value (ohms)
#define RTD_RTD_OHMS       100.0f   // RTD nominal resistance (PT100)
#define RTD_3WIRE          false    // Set true for 3-wire, false for 2/4-wire
#define RTD_FILTER_50HZ    false    // Set true for 50Hz, false for 60Hz
#define RTD_HIGH_FAULT     400      // High fault threshold (ohms)
#define RTD_LOW_FAULT      10       // Low fault threshold (ohms)

// ---- Static driver object ----
static max31865_t rtd_dev;
static volatile float rtd_last_celsius = 0.0f;
static volatile uint8_t rtd_last_fault = 0;

// ---- HAL SPI1 and NSS pin (hardware NSS) ----
extern SPI_HandleTypeDef hspi1;

static void rtd_chipselect(bool select) {
    // Hardware NSS: nothing to do, but could assert/deassert if using GPIO
    // If using software NSS, control NSS pin here
    // Example: HAL_GPIO_WritePin(RTD_NSS_GPIO_Port, RTD_NSS_Pin, select ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static uint8_t rtd_spi_trx(uint8_t data) {
    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 10);
    return rx;
}

static void rtd_delay_charged(void) {
    HAL_Delay(10); // 10ms for bias to settle
}

static void rtd_delay_conversion(void) {
    HAL_Delay(70); // 65ms for conversion (rounded up)
}

static void rtd_high_fault_cb(void) {
    rtd_last_fault = max31865_err_RTD_HIGH_THRESHOLD;
}

static void rtd_low_fault_cb(void) {
    rtd_last_fault = max31865_err_RTD_LOW_THRESHOLD;
}

void RTD_Init(void) {
    max31865_init(&rtd_dev,
        rtd_chipselect,
        rtd_spi_trx,
        rtd_delay_charged,
        rtd_delay_conversion,
        rtd_high_fault_cb,
        rtd_low_fault_cb,
        RTD_RTD_OHMS,
        RTD_RREF_OHMS,
        RTD_LOW_FAULT,
        RTD_HIGH_FAULT,
        RTD_3WIRE,
        RTD_FILTER_50HZ);
    rtd_last_fault = 0;
}

float RTD_ReadCelsius(void) {
    rtd_last_celsius = max31865_readCelsius(&rtd_dev);
    rtd_last_fault = max31865_readFault(&rtd_dev);
    return rtd_last_celsius;
}

float RTD_ReadOhms(void) {
    return max31865_readRTD_ohm(&rtd_dev);
}

bool RTD_Fault(void) {
    return (rtd_last_fault != 0);
}

uint8_t RTD_FaultCode(void) {
    return rtd_last_fault;
}

void RTD_ClearFault(void) {
    max31865_clearFault(&rtd_dev);
    rtd_last_fault = 0;
}

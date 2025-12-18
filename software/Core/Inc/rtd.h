/**
 ******************************************************************************
 * @file    rtd.h
 * @brief   RTD (MAX31865) temperature sensor driver interface
 * @date    December 16, 2025
 ******************************************************************************
 */

#ifndef RTD_H
#define RTD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void RTD_Init(void);
float RTD_ReadCelsius(void);
float RTD_ReadOhms(void);
bool RTD_Fault(void);
uint8_t RTD_FaultCode(void);
void RTD_ClearFault(void);

#ifdef __cplusplus
}
#endif

#endif // RTD_H

/**
 ******************************************************************************
 * @file    input.h
 * @brief   Input handling for encoder and button
 * @date    December 16, 2025
 ******************************************************************************
 */

#ifndef INPUT_H
#define INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

/* Input event flags - check these in main loop and clear after handling */
extern volatile bool input_encoder_cw;        // Clockwise rotation detected
extern volatile bool input_encoder_ccw;       // Counter-clockwise rotation detected
extern volatile bool input_button_pressed;    // Short press detected
extern volatile bool input_button_long_pressed; // Long press detected

/* Encoder position - can be read/written directly */
extern volatile int32_t input_encoder_position;

/**
 * @brief Initialize input handling (encoder + button)
 * @note  Call after MX_LPTIM1_Init()
 * @return true if successful, false if LPTIM encoder start failed
 */
bool Input_Init(void);

/**
 * @brief Poll encoder for rotation
 * @note  Call frequently in main loop
 */
void Input_PollEncoder(void);

/**
 * @brief Poll button with debouncing
 * @note  Call frequently in main loop
 */
void Input_PollButton(void);

/**
 * @brief Poll all inputs (encoder + button)
 * @note  Convenience function - calls both poll functions
 */
void Input_Poll(void);

/**
 * @brief Reset encoder position to zero
 */
void Input_ResetEncoder(void);

/**
 * @brief Set encoder position to a specific value
 * @param position New position value
 */
void Input_SetEncoderPosition(int32_t position);

/**
 * @brief Get encoder position
 * @return Current encoder position
 */
int32_t Input_GetEncoderPosition(void);

/**
 * @brief Check if button is currently held down
 * @return true if button is pressed
 */
bool Input_IsButtonHeld(void);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_H */

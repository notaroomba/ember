/**
 ******************************************************************************
 * @file    input.c
 * @brief   Input handling for encoder and button
 * @date    December 16, 2025
 ******************************************************************************
 */

#include "input.h"
#include "main.h"
#include "utils.h"
#include "config.h"

/* External LPTIM handle from main.c */
extern LPTIM_HandleTypeDef hlptim1;

/* Button state machine states */
typedef enum {
    BTN_IDLE,
    BTN_DEBOUNCE_PRESS,
    BTN_PRESSED,
    BTN_DEBOUNCE_RELEASE
} ButtonState;

/* Private variables */
static int16_t last_encoder_cnt = 0;
static int16_t encoder_accum = 0;
static ButtonState button_state = BTN_IDLE;
static uint32_t button_debounce_start = 0;

/* Public event flags */
volatile bool input_encoder_cw = false;
volatile bool input_encoder_ccw = false;
volatile bool input_button_pressed = false;
volatile bool input_button_long_pressed = false;
volatile int32_t input_encoder_position = 0;

bool Input_Init(void) {
    // Start LPTIM1 in encoder mode - period of 0xFFFF for full 16-bit range
    if (HAL_LPTIM_Encoder_Start(&hlptim1, 0xFFFF) != HAL_OK) {
        return false;
    }
    
    last_encoder_cnt = 0;
    encoder_accum = 0;
    input_encoder_position = 0;
    button_state = BTN_IDLE;
    
    input_encoder_cw = false;
    input_encoder_ccw = false;
    input_button_pressed = false;
    input_button_long_pressed = false;
    
    return true;
}

void Input_PollEncoder(void) {
    int16_t current_cnt = (int16_t)LPTIM1->CNT;
    
    if (current_cnt != last_encoder_cnt) {
        int16_t diff = current_cnt - last_encoder_cnt;
        last_encoder_cnt = current_cnt;
        encoder_accum += diff;
        
        // Check for complete detent rotations
        while (encoder_accum >= ENCODER_COUNTS_PER_DETENT) {
            encoder_accum -= ENCODER_COUNTS_PER_DETENT;
            input_encoder_position++;
            input_encoder_cw = true;
        }
        while (encoder_accum <= -ENCODER_COUNTS_PER_DETENT) {
            encoder_accum += ENCODER_COUNTS_PER_DETENT;
            input_encoder_position--;
            input_encoder_ccw = true;
        }
    }
}

void Input_PollButton(void) {
    bool btn_raw = (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET);
    uint32_t now = HAL_GetTick();
    
    switch (button_state) {
        case BTN_IDLE:
            if (btn_raw) {
                button_state = BTN_DEBOUNCE_PRESS;
                button_debounce_start = now;
            }
            break;
            
        case BTN_DEBOUNCE_PRESS:
            if (!btn_raw) {
                // Released during debounce - false trigger
                button_state = BTN_IDLE;
            } else if ((now - button_debounce_start) >= BUTTON_DEBOUNCE_MS) {
                // Debounce complete, button is really pressed
                button_state = BTN_PRESSED;
                button_debounce_start = now;  // Reuse for long press timing
            }
            break;
            
        case BTN_PRESSED:
            if (!btn_raw) {
                // Released - check if short or long press
                if ((now - button_debounce_start) >= BUTTON_LONG_PRESS_MS) {
                    input_button_long_pressed = true;
                } else {
                    input_button_pressed = true;
                }
                button_state = BTN_DEBOUNCE_RELEASE;
                button_debounce_start = now;
            }
            break;
            
        case BTN_DEBOUNCE_RELEASE:
            if ((now - button_debounce_start) >= BUTTON_DEBOUNCE_MS) {
                button_state = BTN_IDLE;
            }
            break;
    }
}

void Input_Poll(void) {
    Input_PollEncoder();
    Input_PollButton();
}

void Input_ResetEncoder(void) {
    input_encoder_position = 0;
    encoder_accum = 0;
}

void Input_SetEncoderPosition(int32_t position) {
    input_encoder_position = position;
}

int32_t Input_GetEncoderPosition(void) {
    return input_encoder_position;
}

bool Input_IsButtonHeld(void) {
    return (button_state == BTN_PRESSED);
}

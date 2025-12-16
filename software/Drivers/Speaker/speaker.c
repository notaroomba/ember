/**
 ******************************************************************************
 * @file    speaker.c
 * @brief   Speaker/Buzzer Driver Implementation using PWM
 * @date    December 16, 2025
 ******************************************************************************
 */

#include "speaker.h"
#include <stdlib.h>

/* Private variables */
static Speaker_Handle speaker_instance;
static bool initialized = false;
static uint8_t current_volume = 50; // 50% duty cycle for square wave

/**
 * @brief Initialize speaker driver
 */
Speaker_Handle* Speaker_Init(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t timer_clock)
{
    if (htim == NULL || initialized) {
        return initialized ? &speaker_instance : NULL;
    }
    
    speaker_instance.htim = htim;
    speaker_instance.channel = channel;
    speaker_instance.timer_clock = timer_clock;
    speaker_instance.playing = false;
    speaker_instance.melody = NULL;
    speaker_instance.melody_length = 0;
    speaker_instance.melody_index = 0;
    speaker_instance.note_end_tick = 0;
    
    initialized = true;
    
    // Make sure PWM is stopped initially
    HAL_TIM_PWM_Stop(htim, channel);
    
    return &speaker_instance;
}

/**
 * @brief Deinitialize speaker driver
 */
void Speaker_DeInit(Speaker_Handle *handle)
{
    if (handle == NULL) return;
    
    Speaker_Stop(handle);
    initialized = false;
}

/**
 * @brief Play a single tone
 */
void Speaker_Tone(Speaker_Handle *handle, uint16_t frequency)
{
    if (handle == NULL || handle->htim == NULL) return;
    
    if (frequency == 0) {
        Speaker_Stop(handle);
        return;
    }
    
    // Calculate prescaler and ARR for desired frequency
    // PWM frequency = timer_clock / ((PSC + 1) * (ARR + 1))
    // We want: ARR = (timer_clock / (frequency * (PSC + 1))) - 1
    
    uint32_t prescaler = 0;
    uint32_t arr;
    
    // Find suitable prescaler to keep ARR in reasonable range
    // Start with PSC = 0 and increase if ARR would be too large
    arr = (handle->timer_clock / frequency) - 1;
    
    // If ARR is too large, increase prescaler
    while (arr > 65535 && prescaler < 65535) {
        prescaler++;
        arr = (handle->timer_clock / (frequency * (prescaler + 1))) - 1;
    }
    
    // Clamp ARR
    if (arr > 65535) arr = 65535;
    if (arr < 1) arr = 1;
    
    // Set prescaler and auto-reload
    handle->htim->Instance->PSC = prescaler;
    handle->htim->Instance->ARR = arr;
    
    // Set duty cycle (CCR) for volume control
    uint32_t ccr = (arr * current_volume) / 100;
    
    switch (handle->channel) {
        case TIM_CHANNEL_1:
            handle->htim->Instance->CCR1 = ccr;
            break;
        case TIM_CHANNEL_2:
            handle->htim->Instance->CCR2 = ccr;
            break;
        case TIM_CHANNEL_3:
            handle->htim->Instance->CCR3 = ccr;
            break;
        case TIM_CHANNEL_4:
            handle->htim->Instance->CCR4 = ccr;
            break;
        default:
            break;
    }
    
    // Generate update event to load new values
    handle->htim->Instance->EGR = TIM_EGR_UG;
    
    // Start PWM
    HAL_TIM_PWM_Start(handle->htim, handle->channel);
}

/**
 * @brief Stop playing
 */
void Speaker_Stop(Speaker_Handle *handle)
{
    if (handle == NULL || handle->htim == NULL) return;
    
    HAL_TIM_PWM_Stop(handle->htim, handle->channel);
    handle->playing = false;
    handle->melody = NULL;
}

/**
 * @brief Play a tone for a specific duration (blocking)
 */
void Speaker_ToneBlocking(Speaker_Handle *handle, uint16_t frequency, uint16_t duration_ms)
{
    if (handle == NULL) return;
    
    if (frequency > 0) {
        Speaker_Tone(handle, frequency);
    }
    
    HAL_Delay(duration_ms);
    
    Speaker_Stop(handle);
}

/**
 * @brief Start playing a melody asynchronously
 */
void Speaker_PlayMelody(Speaker_Handle *handle, const Speaker_Note *melody, uint16_t length)
{
    if (handle == NULL || melody == NULL || length == 0) return;
    
    handle->melody = melody;
    handle->melody_length = length;
    handle->melody_index = 0;
    handle->playing = true;
    
    // Start first note
    if (melody[0].frequency > 0) {
        Speaker_Tone(handle, melody[0].frequency);
    } else {
        Speaker_Stop(handle);
        handle->playing = true; // Keep playing flag for rest notes
    }
    handle->note_end_tick = HAL_GetTick() + melody[0].duration;
}

/**
 * @brief Update melody playback (call this in main loop)
 */
bool Speaker_Update(Speaker_Handle *handle)
{
    if (handle == NULL || !handle->playing || handle->melody == NULL) {
        return false;
    }
    
    // Check if current note duration has elapsed
    if (HAL_GetTick() >= handle->note_end_tick) {
        handle->melody_index++;
        
        // Check if melody is finished
        if (handle->melody_index >= handle->melody_length) {
            Speaker_Stop(handle);
            return false;
        }
        
        // Play next note
        const Speaker_Note *note = &handle->melody[handle->melody_index];
        
        if (note->frequency > 0) {
            Speaker_Tone(handle, note->frequency);
        } else {
            // Rest - stop PWM but keep playing flag
            HAL_TIM_PWM_Stop(handle->htim, handle->channel);
        }
        
        handle->note_end_tick = HAL_GetTick() + note->duration;
    }
    
    return true;
}

/**
 * @brief Check if melody is currently playing
 */
bool Speaker_IsPlaying(Speaker_Handle *handle)
{
    if (handle == NULL) return false;
    return handle->playing;
}

/**
 * @brief Set speaker volume (duty cycle)
 */
void Speaker_SetVolume(Speaker_Handle *handle, uint8_t volume)
{
    if (volume > 100) volume = 100;
    current_volume = volume;
    
    // If currently playing, update the duty cycle
    if (handle != NULL && handle->playing && handle->htim != NULL) {
        uint32_t arr = handle->htim->Instance->ARR;
        uint32_t ccr = (arr * current_volume) / 100;
        
        switch (handle->channel) {
            case TIM_CHANNEL_1:
                handle->htim->Instance->CCR1 = ccr;
                break;
            case TIM_CHANNEL_2:
                handle->htim->Instance->CCR2 = ccr;
                break;
            case TIM_CHANNEL_3:
                handle->htim->Instance->CCR3 = ccr;
                break;
            case TIM_CHANNEL_4:
                handle->htim->Instance->CCR4 = ccr;
                break;
            default:
                break;
        }
    }
}

/**
 * @brief Play a beep pattern (blocking)
 */
void Speaker_Beep(Speaker_Handle *handle, uint16_t frequency, uint16_t on_time, uint16_t off_time, uint8_t count)
{
    if (handle == NULL) return;
    
    for (uint8_t i = 0; i < count; i++) {
        Speaker_Tone(handle, frequency);
        HAL_Delay(on_time);
        Speaker_Stop(handle);
        
        if (i < count - 1) {
            HAL_Delay(off_time);
        }
    }
}

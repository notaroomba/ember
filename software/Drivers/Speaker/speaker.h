/**
 ******************************************************************************
 * @file    speaker.h
 * @brief   Speaker/Buzzer Driver Header using PWM
 * @date    December 16, 2025
 ******************************************************************************
 */

#ifndef SPEAKER_H
#define SPEAKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32wbxx.h"
#include "stm32wbxx_hal.h"

/* Musical note frequencies (Hz) */
#define NOTE_REST   0
#define NOTE_C3     131
#define NOTE_CS3    139
#define NOTE_D3     147
#define NOTE_DS3    156
#define NOTE_E3     165
#define NOTE_F3     175
#define NOTE_FS3    185
#define NOTE_G3     196
#define NOTE_GS3    208
#define NOTE_A3     220
#define NOTE_AS3    233
#define NOTE_B3     247

#define NOTE_C4     262
#define NOTE_CS4    277
#define NOTE_D4     294
#define NOTE_DS4    311
#define NOTE_E4     330
#define NOTE_F4     349
#define NOTE_FS4    370
#define NOTE_G4     392
#define NOTE_GS4    415
#define NOTE_A4     440
#define NOTE_AS4    466
#define NOTE_B4     494

#define NOTE_C5     523
#define NOTE_CS5    554
#define NOTE_D5     587
#define NOTE_DS5    622
#define NOTE_E5     659
#define NOTE_F5     698
#define NOTE_FS5    740
#define NOTE_G5     784
#define NOTE_GS5    831
#define NOTE_A5     880
#define NOTE_AS5    932
#define NOTE_B5     988

#define NOTE_C6     1047
#define NOTE_CS6    1109
#define NOTE_D6     1175
#define NOTE_DS6    1245
#define NOTE_E6     1319
#define NOTE_F6     1397
#define NOTE_FS6    1480
#define NOTE_G6     1568
#define NOTE_GS6    1661
#define NOTE_A6     1760
#define NOTE_AS6    1865
#define NOTE_B6     1976

#define NOTE_C7     2093

/* Note structure for melodies */
typedef struct {
    uint16_t frequency;  // Frequency in Hz (0 = rest)
    uint16_t duration;   // Duration in ms
} Speaker_Note;

/* Speaker handle structure */
typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint32_t timer_clock;    // Timer clock frequency in Hz
    volatile bool playing;
    volatile const Speaker_Note *melody;
    volatile uint16_t melody_length;
    volatile uint16_t melody_index;
    volatile uint32_t note_end_tick;
} Speaker_Handle;

/**
 * @brief Initialize speaker driver
 * @param htim: Pointer to TIM handle
 * @param channel: TIM channel (TIM_CHANNEL_1, etc.)
 * @param timer_clock: Timer clock frequency in Hz
 * @return Pointer to speaker handle, or NULL on error
 */
Speaker_Handle* Speaker_Init(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t timer_clock);

/**
 * @brief Deinitialize speaker driver
 * @param handle: Pointer to speaker handle
 */
void Speaker_DeInit(Speaker_Handle *handle);

/**
 * @brief Play a single tone
 * @param handle: Pointer to speaker handle
 * @param frequency: Frequency in Hz (0 to stop)
 */
void Speaker_Tone(Speaker_Handle *handle, uint16_t frequency);

/**
 * @brief Stop playing
 * @param handle: Pointer to speaker handle
 */
void Speaker_Stop(Speaker_Handle *handle);

/**
 * @brief Play a tone for a specific duration (blocking)
 * @param handle: Pointer to speaker handle
 * @param frequency: Frequency in Hz
 * @param duration_ms: Duration in milliseconds
 */
void Speaker_ToneBlocking(Speaker_Handle *handle, uint16_t frequency, uint16_t duration_ms);

/**
 * @brief Start playing a melody asynchronously
 * @param handle: Pointer to speaker handle
 * @param melody: Array of notes
 * @param length: Number of notes in melody
 */
void Speaker_PlayMelody(Speaker_Handle *handle, const Speaker_Note *melody, uint16_t length);

/**
 * @brief Update melody playback (call this in main loop or timer interrupt)
 * @param handle: Pointer to speaker handle
 * @return true if still playing, false if finished
 */
bool Speaker_Update(Speaker_Handle *handle);

/**
 * @brief Check if melody is currently playing
 * @param handle: Pointer to speaker handle
 * @return true if playing
 */
bool Speaker_IsPlaying(Speaker_Handle *handle);

/**
 * @brief Set speaker volume (duty cycle)
 * @param handle: Pointer to speaker handle
 * @param volume: Volume 0-100 (percent duty cycle, 50 = loudest square wave)
 */
void Speaker_SetVolume(Speaker_Handle *handle, uint8_t volume);

/**
 * @brief Play a beep pattern
 * @param handle: Pointer to speaker handle
 * @param frequency: Frequency in Hz
 * @param on_time: On time in ms
 * @param off_time: Off time in ms
 * @param count: Number of beeps
 */
void Speaker_Beep(Speaker_Handle *handle, uint16_t frequency, uint16_t on_time, uint16_t off_time, uint8_t count);

#ifdef __cplusplus
}
#endif

#endif /* SPEAKER_H */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "config.h"
#include "utils.h"
#include "tps25730_driver.h"
#include "speaker.h"
#include "input.h"
#include "heater.h"
#include "tmp116.h"
#include "nfc.h"
#include "driver_ina226_basic.h"

#include "rtd.h"

#include "thermocouple.h"

#include "swallow2.h"

#define SSD1306_BLENDER_PRO_BOLD_FONT_13x19

#include "ssd1306.h"
#include "ssd1306_tests.h"
#include <stdlib.h>
#include "ui.h"
#include "blenderProBold.h"
#include "fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;
DMA_HandleTypeDef hdma_i2c3_tx;

IPCC_HandleTypeDef hipcc;

LPTIM_HandleTypeDef hlptim1;

QSPI_HandleTypeDef hqspi;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
TPS25730_Handle *tps_handle = NULL;
Speaker_Handle *speaker = NULL;
TMP116_Handle_t tmp116;
static uint32_t last_temp_read = 0;

static uint32_t last_tc_poll = 0;
static uint32_t last_nfc_poll = 0;
static uint32_t last_ina226_poll = 0;
// Encoder tone settings (configurable)
static uint32_t encoder_tone_base_freq = 2093; // C7
static int16_t encoder_tone_steps = 0; // steps from base, CCW +1 => +50Hz
static uint32_t encoder_tone_step_hz = 50; // Hz per step
static uint32_t encoder_tone_current_freq = 2093; // Start at base freq

/* Random teleport variables for SSD1306 */
static uint32_t last_teleport = 0;
static const uint32_t TELEPORT_INTERVAL_MS = 300; // teleport interval
static const uint8_t TELEPORT_RADIUS = 10; // circle radius

// Forward-declare menu callback
static void main_menu_select_cb(uint8_t idx, void *ctx);

// Settings screen state and helpers
static volatile bool settings_active = false;
static volatile int settings_heater_percent = 0;
static void settings_show(void);
static void settings_update_display(void);
static void settings_on_ok(void *ctx);

SSD1306_t SSD1306_Disp;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C3_Init(void);
static void MX_QUADSPI_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_LPTIM1_Init(void);
static void MX_SPI1_Init(void);
static void MX_IPCC_Init(void);
static void MX_RTC_Init(void);
static void MX_RF_Init(void);
/* USER CODE BEGIN PFP */
/**
  * @brief Callback for main menu selections
  * @param idx: Index of the selected item
  * @param ctx: Context pointer (ui_menu_t *)
  * @retval None
  */
static void main_menu_select_cb(uint8_t idx, void *ctx) {
    ui_menu_t *m = (ui_menu_t *)ctx;
    switch (idx) {
        case 0: // Home
            ui_show_home(m);
            break;
        case 1: { // Status - show USB-C PD voltage & amperage + temperature
            static char status_buf[128];
            int len = 0;
            if (tps_handle != NULL) {
                uint32_t voltage_mv = 0, current_ma = 0;
                if (TPS25730_GetActiveVoltage(tps_handle, &voltage_mv, &current_ma)) {
                    uint32_t volts = voltage_mv / 1000;
                    uint32_t vfrac = (voltage_mv % 1000) / 10; // hundredths
                    uint32_t amps_int = current_ma / 1000;
                    uint32_t amps_frac = (current_ma % 1000) / 10; // hundredths
                    len = snprintf(status_buf, sizeof(status_buf), "%lu.%02lu V\n%lu.%02lu A (%lumA)", volts, vfrac, amps_int, amps_frac, current_ma);
                } else {
                    len = snprintf(status_buf, sizeof(status_buf), "USB-C PD: unavailable");
                }
            } else {
                len = snprintf(status_buf, sizeof(status_buf), "USB-C PD: not initialized");
            }
            if (len < 0) len = 0;
            if (len > (int)sizeof(status_buf) - 1) len = sizeof(status_buf) - 1;

            float temperature;
            if (TMP116_GetTemperature(&tmp116, &temperature) == TMP116_OK) {
                int rem = (int)sizeof(status_buf) - len;
                if (rem > 1) {
                    int n = snprintf(status_buf + len, rem, "\nTemp: %.2f C", temperature);
                    if (n > 0) len += n;
                }
            } else {
                int rem = (int)sizeof(status_buf) - len;
                if (rem > 1) {
                    int n = snprintf(status_buf + len, rem, "\nTemp: N/A");
                    if (n > 0) len += n;
                }
            }

            ui_show_text("Status", status_buf, &Font_11x18, &Font_6x8, NULL, NULL);
            break;
        }
        case 2: // Settings
            settings_show();
            break;
        case 3: // About
            ui_show_text("About", "Ember V1\n@NotARoomba", &Font_11x18, &Font_6x8, NULL, NULL);
            break;
        default:
            break;
    }
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Static buffer used by settings UI so the pointer remains valid
static char settings_buf[128];

// Show the settings screen and initialize value from current heater percent
static void settings_show(void) {
    settings_heater_percent = (int)(Heater_GetPercent() + 0.5f);
    snprintf(settings_buf, sizeof(settings_buf), "Heater: %d%%\nShort press to save\nLong press to return", settings_heater_percent);
    ui_show_text("Settings", settings_buf, &Font_11x18, &Font_6x8, settings_on_ok, NULL);
    settings_active = true;
}

// Update the displayed text - update current text screen in-place
static void settings_update_display(void) {
    snprintf(settings_buf, sizeof(settings_buf), "Heater: %d%%\nShort press to save\nLong press to return", settings_heater_percent);
    ui_update_text(settings_buf);
}

// Save callback (short press) - apply and close
static void settings_on_ok(void *ctx) {
    (void)ctx;
    print("Settings: Heater set to %d%%\r\n", settings_heater_percent);
    Heater_SetPercent((float)settings_heater_percent);
    settings_active = false;
    // Close the settings screen
    ui_handle_event(UI_EVT_BACK);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Config code for STM32_WPAN (HSE Tuning must be done before system clock configuration) */
  MX_APPE_Config();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* IPCC initialisation */
  MX_IPCC_Init();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_QUADSPI_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_LPTIM1_Init();
  MX_SPI1_Init();
  MX_USB_Device_Init();
  MX_RTC_Init();
  MX_RF_Init();
  /* USER CODE BEGIN 2 */
  
  HAL_Delay(1000);

  /* Prevent unused-variable warnings until poll code is enabled */
  (void)last_temp_read;
  (void)last_ina226_poll;

  /* Scan I2C3 bus for devices */
  // int i2c3_devices = I2C_ScanBus(&hi2c3);
  // print("I2C3: %d device(s) found\r\n", i2c3_devices);

  /* Register LED pins with utils so Update_LEDs can control them */
  LEDs_RegisterPin(LED_STATUS_PD,       LED_PD_GPIO_Port,       LED_PD_Pin);
  LEDs_RegisterPin(LED_STATUS_PREHEAT,  LED_PREHEAT_GPIO_Port,  LED_PREHEAT_Pin);
  LEDs_RegisterPin(LED_STATUS_SOAK,     LED_SOAK_GPIO_Port,     LED_SOAK_Pin);
  LEDs_RegisterPin(LED_STATUS_REFLOW,   LED_REFLOW_GPIO_Port,   LED_REFLOW_Pin);
  LEDs_RegisterPin(LED_STATUS_COOL,     LED_COOLING_GPIO_Port,  LED_COOLING_Pin);
  LEDs_RegisterPin(LED_STATUS_STATUS,   LED_BLUE_GPIO_Port,    LED_BLUE_Pin);
  LEDs_RegisterPin(LED_STATUS_GOOD,     LED_GREEN_GPIO_Port,     LED_GREEN_Pin);
  LEDs_RegisterPin(LED_STATUS_ERROR,    LED_RED_GPIO_Port,      LED_RED_Pin);

  /* Set blink interval (ms) and start blinking all LEDs for visual test */
  LEDs_SetBlinkInterval(500);
  LEDs_Blinking();
  
  // Initialize input handling (encoder + button)
  if (!Input_Init()) {
    print("Input: Init FAILED\r\n");
    Error_Handler();
  }
  
  // Initialize heater PWM control
  Heater_Init();
  // Heater_Off();

  Heater_SetPercent(0);

  // Initialize TMP119 temperature sensor on I2C3
  if (TMP116_Init(&tmp116, &hi2c3, TMP116_I2C_ADDRESS) == TMP116_OK) {
    print("TMP119: OK\r\n");
  } else {
    print("TMP119: FAILED\r\n");
  }
  

  // Initialize NT3H2111 NFC tag on I2C3
  if (NFC_Init() == NFC_OK) {
    // NFC_DisablePasswordProtection();
    // Factory reset NFC tag after init using NFC wrapper
    // if (NFC_FactoryReset() == NFC_OK) {
    //   print("NFC: Factory reset complete\r\n");
    // } else {
    //   print("NFC: Factory reset FAILED\r\n");
    // }
    // // NFC_WriteText("Ember V1 NFC Tag");
    NFC_RefreshHash();  // Sync hash with current memory state
  }

  print("Ember V1!\r\n");
  
  // Initialize TPS25730 on I2C1 (assuming address 0x20)
  tps_handle = TPS25730_Init(&hi2c1, 0x20);
  if (tps_handle == NULL) {
    print("TPS25730: Init FAILED\r\n");
  }
  
  HAL_Delay(250);
  
  if (tps_handle != NULL) {
    uint32_t voltage_mv, current_ma;
    
    /*
    // === List all available PDOs from charger ===
    TPS25730_SourceCaps source_caps;
    if (TPS25730_ReadRxSourceCaps(tps_handle, &source_caps)) {
      print("=== Source PDOs ===\r\n");
      for (uint8_t i = 0; i < source_caps.num_valid_pdos; i++) {
        uint32_t pdo = source_caps.pdos[i];
        TPS25730_FixedPDO fixed;
        if (TPS25730_ParseFixedPDO(pdo, &fixed)) {
          print("PDO %u: %lu.%02luV @ %lu.%02luA\r\n", i+1, 
                fixed.voltage_mv/1000, (fixed.voltage_mv%1000)/10,
                fixed.operational_current_ma/1000, (fixed.operational_current_ma%1000)/10);
        }
      }
    }
    */
    
    // Get current voltage
    if (TPS25730_GetActiveVoltage(tps_handle, &voltage_mv, &current_ma)) {
      print("Current: %lu.%02luV @ %lu.%02luA\r\n", 
            voltage_mv/1000, (voltage_mv%1000)/10,
            current_ma/1000, (current_ma%1000)/10);
    }
    
    // Request 20V @ 5A
    print("Requesting 20V @ 5A...\r\n");
    if (TPS25730_RequestVoltage(tps_handle, 20000, 5000, 5000)) {
      HAL_Delay(250);
      
      // Get updated voltage
      if (TPS25730_GetActiveVoltage(tps_handle, &voltage_mv, &current_ma)) {
        print("Updated: %lu.%02luV @ %lu.%02luA\r\n", 
              voltage_mv/1000, (voltage_mv%1000)/10,
              current_ma/1000, (current_ma%1000)/10);
      }
    } else {
      print("Request failed\r\n");
    }
  }

  // Initialize speaker and play jingle bells
  speaker = Speaker_Init(&htim1, TIM_CHANNEL_1, 64000000);
  
  // Jingle Bells melody
  // const Speaker_Note jingle_bells[] = {
  //   // "Jingle bells, jingle bells"
  //   {NOTE_E5, 200}, {NOTE_E5, 200}, {NOTE_E5, 400},
  //   {NOTE_E5, 200}, {NOTE_E5, 200}, {NOTE_E5, 400},
  //   // "Jingle all the way"
  //   {NOTE_E5, 200}, {NOTE_G5, 200}, {NOTE_C5, 200}, {NOTE_D5, 200}, {NOTE_E5, 600},
  //   {NOTE_REST, 200},
  //   // "Oh what fun it is to ride"
  //   {NOTE_F5, 200}, {NOTE_F5, 200}, {NOTE_F5, 200}, {NOTE_F5, 200},
  //   {NOTE_F5, 200}, {NOTE_E5, 200}, {NOTE_E5, 200}, {NOTE_E5, 100}, {NOTE_E5, 100},
  //   // "In a one horse open sleigh"
  //   {NOTE_E5, 200}, {NOTE_D5, 200}, {NOTE_D5, 200}, {NOTE_E5, 200}, {NOTE_D5, 400}, {NOTE_G5, 400},
  // };
  
  // Speaker_PlayMelody(speaker, jingle_bells, sizeof(jingle_bells)/sizeof(jingle_bells[0]));
  
  // // Wait for melody to finish
  // while (Speaker_Update(speaker)) {
  //   // Melody still playing
  // }



  // Initialize INA226 on I2C1 (bus voltage/current monitor)
  // 3W 2mΩ SMD ±50ppm/℃ Current Sense Resistor ±1% 2512
  // if (ina226_basic_init(INA226_ADDRESS_0, 0.002) == 0) {
  //   print("INA226: OK\r\n");
  // } else {
  //   print("INA226: FAILED\r\n");
  // }


  // Initialize RTD (MAX31865) on SPI1
  RTD_Init();
  print("RTD: MAX31865 initialized\r\n");

  // Initialize Thermocouple (MAX6675) on SPI2
  Thermocouple_Init();
  print("Thermocouple: MAX6675 initialized\r\n");

    LEDs_Off();
    Set_LED_Status(LED_STATUS_GOOD, ON);
    Set_LED_Status(LED_STATUS_PD, BLINKING);
    // u8g2_Setup_ssd1306_i2c_128x64_noname_1();
    ssd1306_Init();

    // Initialize UI
    ui_init();
    static const char *main_menu_items[] = {"Home", "Status", "Settings", "About"};
    static ui_menu_t main_menu; // initialize at runtime to allow Font_7x10 assignment
    main_menu.items = main_menu_items;
    main_menu.count = sizeof(main_menu_items)/sizeof(main_menu_items[0]);
    main_menu.selected = 0;
    main_menu.top = 0;
    main_menu.lines = 4;
    main_menu.left_padding = 2;
    main_menu.right_padding = 2;
    // select the preferred font: try Minecraft 8x10 first, then configured default
    main_menu.font = &fontBlenderProBold18pt13x19;
    // main_menu.title = "Main Menu";
    main_menu.on_select = main_menu_select_cb;
    main_menu.cb_ctx = &main_menu;
    // Show the branded home screen and allow the user to press OK to enter the main menu
    ui_show_home(&main_menu);

    // ssd1306_TestAll();

 
    // HAL_Delay(500);
    // ssd1306_FillCircle(64, 32, 10, White);
    // ssd1306_UpdateScreen();
    // HAL_Delay(500);
    // ssd1306_FillCircle(64, 32, 10, White);
    // ssd1306_UpdateScreen();
    // ssd1306_FillCircle(64, 32, 10, White);
    // ssd1306_UpdateScreen();
  /* USER CODE END 2 */

  /* Init code for STM32_WPAN */
  MX_APPE_Init();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Poll inputs (non-blocking)
    Input_Poll();
    Update_LEDs();
    Speaker_Update(speaker);

    // Poll Thermocouple (MAX6675) for temperature every 500ms
    if ((HAL_GetTick() - last_tc_poll) >= 500) {
      last_tc_poll = HAL_GetTick();
       float tc_temp = Thermocouple_ReadCelsius();
      if (Thermocouple_Ok()) {
        print("Thermocouple: %.2f C\r\n", tc_temp);
      } else {
        print("Thermocouple: read failed\r\n");
      }
    }

    // let UI process events and redraw when ready
    ui_tick();

    // Random teleport – only attempt when no UI screen is active
    if (!ui_is_active() && (int32_t)(HAL_GetTick() - last_teleport) >= (int32_t)TELEPORT_INTERVAL_MS) {
        if (SSD1306_Disp.state == SSD1306_STATE_READY) {
            last_teleport = HAL_GetTick();
            ssd1306_Fill(Black); // clear buffer
            uint8_t x = (uint8_t)(rand() % (SSD1306_WIDTH - 2*TELEPORT_RADIUS)) + TELEPORT_RADIUS;
            uint8_t y = (uint8_t)(rand() % (SSD1306_HEIGHT - 2*TELEPORT_RADIUS)) + TELEPORT_RADIUS;
            ssd1306_FillCircle(x, y, TELEPORT_RADIUS, White);
            ssd1306_UpdateScreen();
        }
    }

    // Handle encoder events
    if (input_encoder_cw) {
      input_encoder_cw = false;

      if (settings_active) {
          // Counter-clockwise -> increase percentage
          if (settings_heater_percent < 100) settings_heater_percent++;
          settings_update_display();
          print("Settings: Heater %d%%\r\n", settings_heater_percent);
          // if (speaker != NULL) { Speaker_Beep(speaker, 1000, 30, 10, 1); }
      } else {
          // Notify UI (rotary clockwise -> DOWN)
          ui_handle_event(UI_EVT_DOWN);

          print("Encoder CW, position: %ld\r\n", input_encoder_position);
          // Decrease pitch by 50 Hz per clockwise step
          // encoder_tone_steps--;
          // int32_t freq = (int32_t)encoder_tone_base_freq + (int32_t)encoder_tone_steps * (int32_t)encoder_tone_step_hz;
          // if (freq < 20) freq = 20;
          // if (freq > 20000) freq = 20000;
          // encoder_tone_current_freq = (uint32_t)freq;
          // if (speaker != NULL) {
          //   Speaker_Beep(speaker, (uint16_t)freq, 30, 10, 1);
          // }
      }
    }
    if (input_encoder_ccw) {
      input_encoder_ccw = false;

      if (settings_active) {
          // Clockwise -> decrease percentage
          if (settings_heater_percent > 0) settings_heater_percent--;
          settings_update_display();
          print("Settings: Heater %d%%\r\n", settings_heater_percent);
          // if (speaker != NULL) { Speaker_Beep(speaker, 1200, 30, 10, 1); }
      } else {
          // Notify UI (rotary counter-clockwise -> UP)
          ui_handle_event(UI_EVT_UP);

          print("Encoder CCW, position: %ld\r\n", input_encoder_position);
          // Increase pitch by 50 Hz per counter-clockwise step
          // encoder_tone_steps++;
          // int32_t freq = (int32_t)encoder_tone_base_freq + (int32_t)encoder_tone_steps * (int32_t)encoder_tone_step_hz;
          // if (freq < 20) freq = 20;
          // if (freq > 20000) freq = 20000;
          // encoder_tone_current_freq = (uint32_t)freq;
          // if (speaker != NULL) {
          //   Speaker_Beep(speaker, (uint16_t)freq, 30, 10, 1);
          // }
      }
    }

    // Handle button events
    if (input_button_pressed) {
      input_button_pressed = false;
      // Notify UI (button press -> OK/select)
      ui_handle_event(UI_EVT_OK);

      print("Button short press! Position: %ld\r\n", input_encoder_position);
      // if (speaker != NULL) {
      //   // Play current beep and print frequency
      //   // Speaker_Beep(speaker, (uint16_t)encoder_tone_current_freq, 40, 10, 1);
      //   // print("Speaker current beep: %lu Hz\r\n", encoder_tone_current_freq);
      //     Speaker_PlayMelody(speaker, swallow, sizeof(swallow)/sizeof(swallow[0]));
      //   while(Speaker_Update(speaker)) {
          
      //   }
      }
      // TODO: Handle short press (e.g., select/confirm)
    
    if (input_button_long_pressed) {
      input_button_long_pressed = false;
      if (settings_active) {
          settings_active = false;
          ui_handle_event(UI_EVT_BACK);
          print("Settings: canceled\r\n");
      } else {
        ui_handle_event(UI_EVT_BACK);
          print("Button long press!\r\n");
          // TODO: Additional global long-press behavior
      }
    }
    // Poll temperature sensor periodically
    // if ((HAL_GetTick() - last_temp_read) >= TEMP_READ_INTERVAL_MS) {
    //   last_temp_read = HAL_GetTick();
    //   float temperature;
    //   if (TMP116_GetTemperature(&tmp116, &temperature) == TMP116_OK) {
    //     print("Temp: %d.%02d C\r\n", (int)temperature, (int)((temperature - (int)temperature) * 100));
    //   }
    // }
    
    // Poll NFC for field detection and data changes
    if ((HAL_GetTick() - last_nfc_poll) >= NFC_POLL_INTERVAL_MS) {
      last_nfc_poll = HAL_GetTick();
      NFC_Events_t nfc_events;
      if (NFC_Poll(&nfc_events) == NFC_OK) {
        Set_LED_Status(LED_STATUS_STATUS, nfc_events.field_detected ? ON : OFF);
        //play a jingle  c4 and then f4 in sequence
        
        if (nfc_events.data_changed) {
                        Speaker_PlayMelody(speaker, (Speaker_Note[]){ {NOTE_F4, 75}, {NOTE_C4, 50} }, 2);

          // Read first 16 bytes of user memory to see what changed
          uint8_t nfc_data[16];
          char nfc_text[128];
          if (NFC_ReadMemory(0, nfc_data, sizeof(nfc_data)) == NFC_OK) {
            for (size_t i = 0; i < sizeof(nfc_data); i++) {
              print("%02x ", nfc_data[i]);
            }
            print("\r\n");
          }
        if (NFC_ReadText(nfc_text, sizeof(nfc_text)) == NFC_OK) {
          print("NFC Text: %s\r\n", nfc_text);
        }
        } else if  (nfc_events.field_detected) {
              // Field detected - play C4
              Speaker_PlayMelody(speaker, (Speaker_Note[]){ {NOTE_C4, 75}, {NOTE_F4, 50} }, 2);
            }

      }
    }
    // Poll INA226 for current/voltage every 500ms
    // if ((HAL_GetTick() - last_ina226_poll) >= 500) {
    //   last_ina226_poll = HAL_GetTick();
    //   float bus_mv = 0, current_ma = 0, power_mw = 0;
    //   if (ina226_basic_read(&bus_mv, &current_ma, &power_mw) == 0) {
    //     print("INA226: %.2f mV, %.2f mA, %.2f mW\r\n", bus_mv, current_ma, power_mw);
    //   } else {
    //     print("INA226: read failed\r\n");
    //   }
    // }
    

    // Poll RTD (MAX31865) for temperature every 500ms
    // static uint32_t last_rtd_poll = 0;
    // if ((HAL_GetTick() - last_rtd_poll) >= 500) {
    //   last_rtd_poll = HAL_GetTick();
    //   float rtd_temp = RTD_ReadCelsius();
    //   float rtd_ohms = RTD_ReadOhms();
    //   if (!RTD_Fault()) {
    //     print("RTD: %.2f C, %.2f Ohms\r\n", rtd_temp, rtd_ohms);
    //   } else {
    //     print("RTD: FAULT (code 0x%02X)\r\n", RTD_FaultCode());
    //     RTD_ClearFault();
    //   }
    // }


    
    // Other main loop tasks here...
    
    /* USER CODE END WHILE */
    MX_APPE_Process();

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI1
                              |RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 32;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_RFWAKEUP;
  PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_HSE_DIV1024;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10B17DB5;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */
  hi2c3.hdmatx = &hdma_i2c3_tx;
  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x10B17DB5;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief IPCC Initialization Function
  * @param None
  * @retval None
  */
static void MX_IPCC_Init(void)
{

  /* USER CODE BEGIN IPCC_Init 0 */

  /* USER CODE END IPCC_Init 0 */

  /* USER CODE BEGIN IPCC_Init 1 */

  /* USER CODE END IPCC_Init 1 */
  hipcc.Instance = IPCC;
  if (HAL_IPCC_Init(&hipcc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IPCC_Init 2 */

  /* USER CODE END IPCC_Init 2 */

}

/**
  * @brief LPTIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPTIM1_Init(void)
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim1.Init.UltraLowPowerClock.Polarity = LPTIM_CLOCKPOLARITY_RISING_FALLING;
  hlptim1.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_2TRANSITIONS;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_EXTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 255;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = 1;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}

/**
  * @brief RF Initialization Function
  * @param None
  * @retval None
  */
static void MX_RF_Init(void)
{

  /* USER CODE BEGIN RF_Init 0 */

  /* USER CODE END RF_Init 0 */

  /* USER CODE BEGIN RF_Init 1 */

  /* USER CODE END RF_Init 1 */
  /* USER CODE BEGIN RF_Init 2 */

  /* USER CODE END RF_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = CFG_RTC_ASYNCH_PRESCALER;
  hrtc.Init.SynchPrediv = CFG_RTC_SYNCH_PRESCALER;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
  hspi2.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 6399;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  // Reconfigure TIM2 for heater PWM using config.h values
  // PWM freq = TIM_CLK / ((PSC+1) * (ARR+1))
  htim2.Init.Prescaler = HEATER_TIM_PRESCALER;
  htim2.Init.Period = HEATER_TIM_PERIOD;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMAMUX1_OVR_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMAMUX1_OVR_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMAMUX1_OVR_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init 1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_BLUE_Pin|LED_GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_COOLING_Pin|LED_REFLOW_Pin|LED_SOAK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_PREHEAT_Pin|LED_PD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : CURRENT_ALERT_Pin PT_DRDY_Pin */
  GPIO_InitStruct.Pin = CURRENT_ALERT_Pin|PT_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FIELD_DETECT_Pin SINK_EN_Pin */
  GPIO_InitStruct.Pin = FIELD_DETECT_Pin|SINK_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : TEMP_ALERT_Pin */
  GPIO_InitStruct.Pin = TEMP_ALERT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TEMP_ALERT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_BLUE_Pin LED_GREEN_Pin */
  GPIO_InitStruct.Pin = LED_BLUE_Pin|LED_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_RED_Pin */
  GPIO_InitStruct.Pin = LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_RED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : FAULT_IN_Pin */
  GPIO_InitStruct.Pin = FAULT_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FAULT_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_COOLING_Pin LED_REFLOW_Pin LED_SOAK_Pin */
  GPIO_InitStruct.Pin = LED_COOLING_Pin|LED_REFLOW_Pin|LED_SOAK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_PREHEAT_Pin LED_PD_Pin */
  GPIO_InitStruct.Pin = LED_PREHEAT_Pin|LED_PD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init 2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


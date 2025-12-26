/**
 * Private configuration file for the SSD1306 library.
 * This example is configured for STM32F0, I2C and including all fonts.
 */

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

// Pull in application-level configuration (font selection, etc.)
#include "config.h"

// Choose a microcontroller family
#define STM32WB
// #define STM32F0
//#define STM32F1
//#define STM32F4
//#define STM32L0
//#define STM32L1
//#define STM32L4
//#define STM32F3
//#define STM32H7
//#define STM32F7
//#define STM32G0
//#define STM32C0
//#define STM32U5

// Choose a bus
#define SSD1306_USE_I2C
//#define SSD1306_USE_SPI

// I2C Configuration
#define SSD1306_I2C_PORT        hi2c3
#define SSD1306_I2C_ADDR        (0x3C << 1)

// If you plan to use the DMA-based transfer (recommended), make sure the
// selected I2C handle has its TX DMA handle assigned in MX_I2C#_Init, e.g.:
//   hi2c3.hdmatx = &hdma_i2c3_tx;
// or in MX_DMA_Init() set up the correct DMA stream and link it to the
// selected hi2cX.hdmatx. Without this the driver will fall back to
// blocking transfers.

// SPI Configuration
//#define SSD1306_SPI_PORT        hspi1
//#define SSD1306_CS_Port         OLED_CS_GPIO_Port
//#define SSD1306_CS_Pin          OLED_CS_Pin
//#define SSD1306_DC_Port         OLED_DC_GPIO_Port
//#define SSD1306_DC_Pin          OLED_DC_Pin
//#define SSD1306_Reset_Port      OLED_Res_GPIO_Port
//#define SSD1306_Reset_Pin       OLED_Res_Pin

// Mirror the screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Set inverse color if needed
// # define SSD1306_INVERSE_COLOR

// Include only needed fonts
/* Font selection macros have been moved to Core/Inc/config.h to centralize
   application-level configuration. Define SSD1306_INCLUDE_FONT_<WxH> there
   to include specific fonts and reduce flash usage. */
// e.g. #define SSD1306_INCLUDE_FONT_9x12

// The width of the screen can be set using this
// define. The default value is 128.
// #define SSD1306_WIDTH           64

// If your screen horizontal axis does not start
// in column 0 you can use this define to
// adjust the horizontal offset
// #define SSD1306_X_OFFSET

// The height can be changed as well if necessary.
// It can be 32, 64 or 128. The default value is 64.
// #define SSD1306_HEIGHT          64

#endif /* __SSD1306_CONF_H__ */

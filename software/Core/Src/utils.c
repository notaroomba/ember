#include "utils.h"
#include "usbd_cdc_if.h"
#include <stdarg.h>

void print(const char* format, ...) {
    char buffer[256];
    va_list args;
    
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (len > 0 && len < (int)sizeof(buffer)) {
        CDC_Transmit_FS((uint8_t *)buffer, len);
    }
    HAL_Delay(10); // Small delay to ensure transmission
}
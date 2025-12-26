/* ui.h - Simple UI abstraction inspired by u8g2 "selection list" and menu system
 * Provides scrollable menus, buttons, and graph widgets for SSD1306 displays.
 * Lightweight, minimal dependencies (uses ssd1306 API)
 */

#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stdbool.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"  // provide Font_6x8, Font_7x10 etc

#ifdef __cplusplus
extern "C" {
#endif

// Generic UI events (map your input system to these)
typedef enum {
    UI_EVT_NONE = 0,
    UI_EVT_UP,
    UI_EVT_DOWN,
    UI_EVT_LEFT,
    UI_EVT_RIGHT,
    UI_EVT_OK,
    UI_EVT_BACK,
    UI_EVT_TICK
} ui_event_t;

// Menu item selection callback
typedef void (*ui_menu_cb_t)(uint8_t idx, void *ctx);

// Simple scrollable menu structure
#define UI_MENU_DEFAULT_CHAR_SPACING 1  // default extra pixels between characters when rendering menus

typedef struct {
    const char **items;    // array of C strings
    uint8_t count;         // number of items
    uint8_t selected;      // currently selected index
    uint8_t top;           // top visible index (scroll)
    uint8_t lines;         // visible lines (0 = automatic)
    const SSD1306_Font_t *font;   // font to use (pointer)
    const char *title;     // optional title displayed on top (NULL = none)
    ui_menu_cb_t on_select; // selection callback
    void *cb_ctx;          // user context passed to callback
    uint8_t char_spacing;  // extra pixel spacing between characters (0 = use default)
    uint8_t left_padding;  // left padding (pixels) when rendering menu items
    uint8_t right_padding; // right padding (pixels) when rendering menu items (new)
    uint8_t item_vpad;     // vertical padding (pixels) between menu items (new)
} ui_menu_t;

// Simple graph widget (scrolling buffer)
typedef struct {
    int16_t *buffer;     // pointer to samples (user-provided)
    uint16_t size;       // buffer size
    uint16_t head;       // next write index
    int16_t min;         // expected minimum value
    int16_t max;         // expected maximum value
    uint8_t x, y, w, h;  // drawing area in pixels
    const char *title;   // optional title
    bool autoscale;      // if true, autoscale min/max from data
} ui_graph_t;

// Button widget
typedef void (*ui_button_cb_t)(void *ctx);

typedef struct {
    uint8_t x, y, w, h;     // button rectangle
    const char *label;      // label text
    ui_button_cb_t cb;      // callback when pressed
    void *ctx;              // callback context
} ui_button_t;

// Public API
void ui_init(void);

// Screens: start a menu/graph screen (takes pointer to user allocated object)
void ui_start_menu(ui_menu_t *menu);
void ui_start_graph(ui_graph_t *graph);
void ui_show_home(ui_menu_t *menu); // show branded home screen that can jump to a menu
void ui_graph_push(ui_graph_t *g, int16_t value);

// Button screen helper
void ui_start_buttons(ui_button_t *buttons, uint8_t count, uint8_t initial_selected);

// Event handling
void ui_handle_event(ui_event_t evt);

// Periodic call to let UI process pending events and queued renders
void ui_tick(void);

// Utility
bool ui_is_active(void); // true if a UI screen is currently displayed

#ifdef __cplusplus
}
#endif

#endif // UI_H

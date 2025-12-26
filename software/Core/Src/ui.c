/* ui.c - Lightweight UI system inspired by U8G2 selection lists
 * Supports: scrollable menu, simple graph widget, and buttons
 * Uses SSD1306 low-level API (ssd1306_*) for rendering.
 */

#include "ui.h"
#include "ssd1306.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

/* Use the global SSD1306_Disp structure defined in main.c for driver state */
extern SSD1306_t SSD1306_Disp;

// Internal screen types
typedef enum {
    UI_SCR_NONE = 0,
    UI_SCR_MENU,
    UI_SCR_GRAPH,
    UI_SCR_TEXT
} ui_scr_type_t;

static ui_scr_type_t current_type = UI_SCR_NONE;
static ui_menu_t *current_menu = NULL;
static ui_graph_t *current_graph = NULL;

// Pending event (simple single-entry queue)
static ui_event_t pending_event = UI_EVT_NONE;
static bool need_redraw = false;

#define UI_STACK_DEPTH 6
typedef struct {
    ui_scr_type_t type;
    void *obj;
} ui_stack_entry_t;
static ui_stack_entry_t ui_stack[UI_STACK_DEPTH];
static int ui_stack_top = -1;

// Text screen
typedef struct {
    const char *title;
    const char *text;
    const SSD1306_Font_t *font;
} ui_text_t;
static ui_text_t text_screen;

// Forward declarations
static void render_menu(void);
static void render_graph(void);
static void render_text(void);
static void ui_push_screen(ui_scr_type_t type, void *obj);
static void ui_pop_screen(void);
static uint8_t compute_menu_lines(ui_menu_t *m);
static void request_render(void);

// Helper: clamp
static inline int16_t clamp16(int16_t v, int16_t a, int16_t b) {
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

// Measure pixel width of a string (already present)
static uint16_t calc_text_width(const SSD1306_Font_t *font, const char *str) {
    uint16_t w = 0;
    if (!font || !str) return 0;
    uint8_t spacing = SSD1306_Disp.CharSpacing;
    while (*str) {
        if (font->char_width) w += font->char_width[(unsigned char)*str - 32];
        else w += font->width;
        if (*(str + 1) != '\0') w += spacing;
        ++str;
    }
    return w;
}

// Write a string clipped to a maximum pixel width (truncates characters as needed)
static void write_string_clipped(const SSD1306_Font_t *font, const char *str, uint8_t max_w, SSD1306_COLOR color)
{
    if (!font || !str) return;
    if (calc_text_width(font, str) <= max_w) {
        ssd1306_WriteString((char*)str, *font, color);
        return;
    }
    char buf[128];
    size_t len = strlen(str);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    strncpy(buf, str, len);
    buf[len] = '\0';
    while (len > 0 && calc_text_width(font, buf) > max_w) {
        buf[--len] = '\0';
    }
    ssd1306_WriteString(buf, *font, color);
}

// Render menu into SSD1306 buffer (does not start display transfer)
static void render_menu(void) {
    ui_menu_t *m = current_menu;
    if (!m || !m->font) return;

    uint8_t lines = compute_menu_lines(m);
    // Draw background
    ssd1306_Fill(SSD1306_PX_CLR_BLACK);

    // Save/establish character spacing (preserve existing)
    uint8_t old_spacing = ssd1306_GetCharSpacing();
    uint8_t spacing = (m->char_spacing == 0) ? UI_MENU_DEFAULT_CHAR_SPACING : m->char_spacing;
    ssd1306_SetCharSpacing(spacing);

    // Title if present
    uint8_t y = 0;
    uint8_t left = m->left_padding ? m->left_padding : 2; // default left padding
    uint8_t right = m->right_padding ? m->right_padding : 2; // default right padding
    uint8_t vpad = (m->item_vpad == 0) ? 1 : m->item_vpad; // default vertical padding
    uint8_t line_h = m->font->height + vpad;

    if (m->title) {
        ssd1306_SetCursor(left, 0);
        char tmp[64];
        strncpy(tmp, m->title, sizeof(tmp)-1);
        tmp[sizeof(tmp)-1] = '\0';
        ssd1306_WriteString(tmp, *(m->font), White);
        y += m->font->height;
        // leave vpad gap
        y += vpad;
    }

    // Adjust available lines after title
    uint8_t avail_lines = (SSD1306_HEIGHT - y) / line_h;
    if (avail_lines < lines) lines = avail_lines;

    // Ensure top is valid
    if (m->top > m->selected) m->top = m->selected;
    if (m->selected >= m->top + lines) m->top = m->selected - (lines - 1);

    // Draw each visible item (with clipping to right padding)
    for (uint8_t i = 0; i < lines; ++i) {
        uint8_t idx = m->top + i;
        if (idx >= m->count) break;
        uint8_t text_x = left;                       // text respects left padding
        uint8_t text_y = y + i * line_h;
        ssd1306_SetCursor(text_x, text_y);
        // ssd1306_WriteString expects char*, so copy into local buffer to be safe
        char buf[64];
        strncpy(buf, m->items[idx], sizeof(buf)-1);
        buf[sizeof(buf)-1] = '\0';
        uint8_t max_w = SSD1306_WIDTH - left - right; // text clipping uses paddings
        write_string_clipped(m->font, buf, max_w, White);
    }

    // Restore previous char spacing
    ssd1306_SetCharSpacing(old_spacing);

    // Highlight selected row (invert rectangle — full width regardless of padding)
    uint8_t sel_row = m->selected - m->top;
    if (sel_row < lines) {
        uint8_t y1 = y + sel_row * line_h;
        uint8_t y2 = y1 + line_h - 1;
        // extend highlight across the full screen width
        ssd1306_InvertRectangle(0, y1, SSD1306_WIDTH - 1, y2);
    }
}

// Graph rendering
static void render_graph(void) {
    ui_graph_t *g = current_graph;
    if (!g || !g->buffer || g->size == 0) return;

    ssd1306_Fill(SSD1306_PX_CLR_BLACK);

    // Title
    uint8_t top_y = g->y;
    uint8_t h = g->h;
    if (g->title) {
        ssd1306_SetCursor(g->x, g->y);
        // reuse Font_6x8 by default
        ssd1306_WriteString((char*)g->title, Font_6x8, White);
        top_y += Font_6x8.height + 1;
        h -= (Font_6x8.height + 1);
    }

    // Determine min/max
    int16_t minv = g->min;
    int16_t maxv = g->max;
    if (g->autoscale) {
        minv = INT16_MAX; maxv = INT16_MIN;
        for (uint16_t i = 0; i < g->size; ++i) {
            int16_t v = g->buffer[i];
            if (v < minv) minv = v;
            if (v > maxv) maxv = v;
        }
        if (minv >= maxv) { maxv = minv + 1; }
    }

    // Draw samples across width
    for (uint8_t x = 0; x < g->w; ++x) {
        // map x to buffer index (oldest on left)
        uint16_t idx = (g->head + x) % g->size;
        int16_t v = g->buffer[idx];
        // map v to y coordinate within h
        float t = (float)(v - minv) / (float)(maxv - minv);
        if (t < 0.0f) {
            t = 0.0f;
        }
        if (t > 1.0f) {
            t = 1.0f;
        }
        uint8_t plot_y = top_y + (h - 1) - (uint8_t)(t * (h - 1));
        ssd1306_DrawPixel(g->x + x, plot_y, White);
    }
}

// Push a sample into graph buffer
void ui_graph_push(ui_graph_t *g, int16_t value) {
    if (!g || !g->buffer || g->size == 0) return;
    g->buffer[g->head] = value;
    g->head = (g->head + 1) % g->size;
    need_redraw = true;
}

// Internal event handling for menu
static void menu_handle_event(ui_event_t evt) {
    ui_menu_t *m = current_menu;
    if (!m) return;
    switch (evt) {
        case UI_EVT_UP:
            if (m->selected == 0) m->selected = m->count - 1;
            else m->selected--;
            if (m->selected < m->top) m->top = m->selected;
            request_render();
            break;
        case UI_EVT_DOWN:
            if (m->selected + 1 >= m->count) m->selected = 0;
            else m->selected++;
            {
                uint8_t lines = compute_menu_lines(m);
                if (m->selected >= m->top + lines) m->top = m->selected - (lines - 1);
            }
            request_render();
            break;
        case UI_EVT_OK:
            if (m->on_select) {
                m->on_select(m->selected, m->cb_ctx);
            } else {
                // default: push a text screen showing the selected item
                text_screen.title = m->items[m->selected];
                text_screen.text = NULL;
                text_screen.font = m->font ? m->font : &Font_11x18;
                ui_push_screen(UI_SCR_TEXT, &text_screen);
            }
            break;
        case UI_EVT_BACK:
            ui_pop_screen();
            break;
        default:
            break;
    }
}

// Internal event for graph
static void graph_handle_event(ui_event_t evt) {
    (void)evt;
    // simple graphs don't respond to events by default
}

// Button screen implementation
static ui_button_t *current_buttons = NULL;
static uint8_t current_buttons_count = 0;
static uint8_t current_button_selected = 0;

void ui_start_buttons(ui_button_t *buttons, uint8_t count, uint8_t initial_selected) {
    current_type = UI_SCR_NONE; // reset other screens
    current_buttons = buttons;
    current_buttons_count = count;
    current_button_selected = (initial_selected < count) ? initial_selected : 0;
    current_type = (count > 0) ? UI_SCR_MENU : UI_SCR_NONE; // reuse menu screen type for now
    request_render();
}

static void render_buttons(void) {
    if (!current_buttons || current_buttons_count == 0) return;

    // Choose font for labels
    const SSD1306_Font_t *font = &Font_11x18; // larger text for buttons
    uint8_t btn_h = font->height + 10; // bigger buttons
    uint8_t margin = 6;
    uint8_t total_margin = (current_buttons_count + 1) * margin;
    uint8_t available_w = SSD1306_WIDTH - total_margin;
    uint8_t btn_w = available_w / current_buttons_count;

    // Vertical center
    uint8_t y = (SSD1306_HEIGHT - btn_h) / 2;

    ssd1306_Fill(SSD1306_PX_CLR_BLACK);

    for (uint8_t i = 0; i < current_buttons_count; ++i) {
        uint8_t x = margin + i * (btn_w + margin);
        current_buttons[i].x = x;
        current_buttons[i].y = y;
        current_buttons[i].w = btn_w;
        current_buttons[i].h = btn_h;

        uint8_t r = btn_h / 2; // radius for rounded ends
        uint8_t cx_left = x + r;
        uint8_t cx_right = x + btn_w - r - 1;
        uint8_t cy = y + r;

        if (i == current_button_selected) {
            // filled capsule
            ssd1306_FillRectangle(x + r, y, x + btn_w - r - 1, y + btn_h - 1, White);
            ssd1306_FillCircle(cx_left, cy, r, White);
            ssd1306_FillCircle(cx_right, cy, r, White);
            // label in black
            uint16_t tw = calc_text_width(font, current_buttons[i].label);
            int16_t tx = x + (btn_w - tw) / 2;
            int16_t ty = y + (btn_h - font->height) / 2;
            ssd1306_SetCursor((uint8_t)tx, (uint8_t)ty);
            ssd1306_WriteString((char*)current_buttons[i].label, *font, Black);
        } else {
            // outlined capsule
            ssd1306_DrawRectangle(x + r, y, x + btn_w - r - 1, y + btn_h - 1, White);
            ssd1306_DrawCircle(cx_left, cy, r, White);
            ssd1306_DrawCircle(cx_right, cy, r, White);
            // small dotted shadow on bottom-right
            for (int sx = 0; sx < 3; ++sx) {
                for (int py = 0; py < btn_h; py += 2) {
                    ssd1306_DrawPixel(x + btn_w - 1 + sx, y + py, White);
                }
                for (int px = 0; px < btn_w; px += 2) {
                    ssd1306_DrawPixel(x + px, y + btn_h - 1 + sx, White);
                }
            }
            // label in white
            uint16_t tw = calc_text_width(font, current_buttons[i].label);
            int16_t tx = x + (btn_w - tw) / 2;
            int16_t ty = y + (btn_h - font->height) / 2;
            ssd1306_SetCursor((uint8_t)tx, (uint8_t)ty);
            ssd1306_WriteString((char*)current_buttons[i].label, *font, White);
        }
    }
}

// Compute how many lines fit for a given menu (honors optional m->lines)
static uint8_t compute_menu_lines(ui_menu_t *m)
{
    if (!m) return 0;
    const SSD1306_Font_t *font = m->font ? m->font : &Font_6x8;
    uint8_t y = 0;
    uint8_t vpad = (m->item_vpad == 0) ? 1 : m->item_vpad;
    uint8_t line_h = font->height + vpad;
    if (m->title) {
        y += font->height;
        // small gap
        y += vpad;
    }
    uint8_t avail = (SSD1306_HEIGHT - y) / line_h;
    if (avail == 0) avail = 1;
    if (m->lines > 0) {
        return (m->lines <= avail) ? m->lines : avail;
    }
    return avail;
}

// Mark that the UI needs a redraw (deferred until display is ready)
static void request_render(void)
{
    need_redraw = true;
}

// Public API: initialize UI subsystem
void ui_init(void)
{
    ui_stack_top = -1;
    current_menu = NULL;
    current_graph = NULL;
    current_buttons = NULL;
    current_buttons_count = 0;
    current_button_selected = 0;
    pending_event = UI_EVT_NONE;
    need_redraw = false;
}

// Start a menu screen (user-owned menu object)
void ui_start_menu(ui_menu_t *menu)
{
    if (!menu) return;
    // sanitize indices
    if (menu->count == 0) {
        menu->selected = 0;
        menu->top = 0;
    } else {
        if (menu->selected >= menu->count) menu->selected = 0;
        if (menu->top >= menu->count) menu->top = 0;
    }
    ui_push_screen(UI_SCR_MENU, menu);
}

// Start a graph screen
void ui_start_graph(ui_graph_t *graph)
{
    if (!graph) return;
    ui_push_screen(UI_SCR_GRAPH, graph);
}

// Return true if any UI screen or button screen is active
bool ui_is_active(void)
{
    return (ui_stack_top >= 0) || (current_buttons && current_buttons_count > 0);
}

// Enhance ui_tick to render buttons when active
void ui_tick(void) {
    // If display became ready and we have a pending event, handle it
    if (SSD1306_Disp.state == SSD1306_STATE_READY && pending_event != UI_EVT_NONE) {
        ui_event_t ev = pending_event;
        pending_event = UI_EVT_NONE;
        ui_handle_event(ev);
    }

    if (need_redraw && SSD1306_Disp.state == SSD1306_STATE_READY) {
        need_redraw = false; // clear before rendering
        // call appropriate render
        if (current_type == UI_SCR_MENU && current_menu) {
            render_menu();
            ssd1306_UpdateScreen();
        } else if (current_type == UI_SCR_GRAPH && current_graph) {
            render_graph();
            ssd1306_UpdateScreen();
        } else if (current_type == UI_SCR_TEXT) {
            render_text();
            ssd1306_UpdateScreen();
        } else if (current_buttons && current_buttons_count > 0) {
            render_buttons();
            ssd1306_UpdateScreen();
        }
    }
}

// Handle events for buttons
static void buttons_handle_event(ui_event_t evt) {
    if (!current_buttons || current_buttons_count == 0) return;
    switch (evt) {
        case UI_EVT_LEFT:
        case UI_EVT_UP: // treat up as left for buttons
            if (current_button_selected == 0) current_button_selected = current_buttons_count - 1;
            else current_button_selected--;
            request_render();
            break;
        case UI_EVT_RIGHT:
        case UI_EVT_DOWN: // treat down as right for buttons
            current_button_selected = (current_button_selected + 1) % current_buttons_count;
            request_render();
            break;
        case UI_EVT_OK:
            if (current_buttons[current_button_selected].cb)
                current_buttons[current_button_selected].cb(current_buttons[current_button_selected].ctx);
            break;
        case UI_EVT_BACK:
            // exit buttons screen
            current_buttons = NULL;
            current_buttons_count = 0;
            request_render();
            break;
        default:
            break;
    }
}

// Modify ui_handle_event to route to button handler when appropriate
void ui_handle_event(ui_event_t evt) {
    // If display is busy, queue event
    if (SSD1306_Disp.state != SSD1306_STATE_READY) {
        pending_event = evt;
        return;
    }

    if (current_buttons && current_buttons_count > 0) {
        buttons_handle_event(evt);
        return;
    }

    if (current_type == UI_SCR_MENU) {
        menu_handle_event(evt);
    } else if (current_type == UI_SCR_GRAPH) {
        graph_handle_event(evt);
    }
}

static void ui_refresh_current(void)
{
    if (ui_stack_top < 0) {
        current_type = UI_SCR_NONE;
        current_menu = NULL;
        current_graph = NULL;
        current_buttons = NULL;
        current_buttons_count = 0;
    } else {
        ui_stack_entry_t e = ui_stack[ui_stack_top];
        current_type = e.type;
        if (e.type == UI_SCR_MENU) {
            current_menu = (ui_menu_t *)e.obj;
            current_graph = NULL;
            current_buttons = NULL;
        } else if (e.type == UI_SCR_GRAPH) {
            current_graph = (ui_graph_t *)e.obj;
            current_menu = NULL;
            current_buttons = NULL;
        } else if (e.type == UI_SCR_TEXT) {
            // text screen stored in text_screen
            current_menu = NULL;
            current_graph = NULL;
            current_buttons = NULL;
        } else {
            current_menu = NULL;
            current_graph = NULL;
            current_buttons = NULL;
        }
    }
}

static void ui_push_screen(ui_scr_type_t type, void *obj)
{
    if (ui_stack_top < UI_STACK_DEPTH - 1) {
        ui_stack[++ui_stack_top].type = type;
        ui_stack[ui_stack_top].obj = obj;
        ui_refresh_current();
        request_render();
    }
}

static void ui_pop_screen(void)
{
    if (ui_stack_top >= 0) ui_stack_top--;
    ui_refresh_current();
    request_render();
}

// Render function for text screen
static void render_text(void)
{
    ssd1306_Fill(SSD1306_PX_CLR_BLACK);
    if (text_screen.title) {
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString((char*)text_screen.title, *(text_screen.font ? text_screen.font : &Font_11x18), White);
    }
    if (text_screen.text) {
        ssd1306_SetCursor(0, text_screen.font ? text_screen.font->height + 2 : Font_11x18.height + 2);
        ssd1306_WriteString((char*)text_screen.text, Font_6x8, White);
    }
}

#ifndef LV_COLOR_SELECTOR_H
#define LV_COLOR_SELECTOR_H

#include "../lvgl.h"

#if LV_USE_GRID == 0
#error "LV_USE_GRID needs to be enabled"
#endif

// The type for a callback everytime the color changes
typedef void (*color_changed_cb_t)(lv_color_t color);

// Create a color selector widget
lv_obj_t* color_selector_create(lv_obj_t* parent, color_changed_cb_t cb);

// Change the color of a color selector
void change_color(lv_obj_t* color_selector_w, lv_color_t color);

#endif /*LV_COLOR_SELECTOR_H*/
#ifndef SLIDER_H
#define SLIDER_H

#include <lvgl.h>

// Create a simple slider widget
lv_obj_t *slider_create(lv_obj_t *parent, lv_color_t color, lv_event_cb_t slider_pressed_cb);

#endif // SLIDER_H
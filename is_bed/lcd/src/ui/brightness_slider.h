#ifndef BRIGHTNESS_SLIDER_H
#define BRIGHTNESS_SLIDER_H

#include <lvgl.h>

// Create a simple slider widget
lv_obj_t *brightness_slider_create(lv_obj_t *parent, lv_event_cb_t slider_changed_cb, lv_group_t *encoder_group, uint32_t index, const char* label);

#endif // BRIGHTNESS_SLIDER_H
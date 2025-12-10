#ifndef PATTERN_SLIDER_H
#define PATTERN_SLIDER_H

#include <lvgl.h>

// The callback type for pattern changed
typedef void (*pattern_changed_cb_t)(uint32_t pattern_index);

// Create a slider to select patterns
lv_obj_t *pattern_slider_create(lv_obj_t *parent, pattern_changed_cb_t callback, lv_group_t *encoder_group);

#endif // PATTERN_SLIDER_H
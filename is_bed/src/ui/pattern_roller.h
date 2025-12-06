#ifndef PATTERN_ROLLER_H
#define PATTERN_ROLLER_H

#include <lvgl.h>

// Create a roller to select patterns
lv_obj_t *pattern_roller_create(lv_obj_t *parent, lv_event_cb_t pattern_changed_cb, lv_group_t *encoder_group);

#endif // PATTERN_ROLLER_H
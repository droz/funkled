#ifndef PATTERN_SLIDER_H
#define PATTERN_SLIDER_H

#include <lvgl.h>
#include <Arduino.h>

#define MAX_LED_PATTERNS 64
#define MAX_LED_PATTERN_NAME_LENGTH 32

// The available LED patterns
extern String pattern_names[MAX_LED_PATTERNS];
extern size_t num_patterns;
// The types of the patterns
extern uint8_t pattern_types[MAX_LED_PATTERNS];
// The currently displayed (not selected yet) pattern index
extern uint32_t displayed_pattern_index;
// The currently selected (user clicked validate) pattern index
extern uint32_t selected_pattern_index;

// The callback type for pattern changed
typedef void (*pattern_changed_cb_t)(uint32_t pattern_index);

// Create a slider to select patterns
lv_obj_t *pattern_slider_create(lv_obj_t *parent, pattern_changed_cb_t callback, lv_group_t *encoder_group);

// Set the current pattern on the slider
void pattern_slider_set_pattern(uint32_t pattern_index);

#endif // PATTERN_SLIDER_H
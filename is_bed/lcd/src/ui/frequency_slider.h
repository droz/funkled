#ifndef FREQUENCY_SLIDER_H
#define FREQUENCY_SLIDER_H

#include <lvgl.h>

// The current frequency value
// To get the value in Hz: frequency_hz = frequency / 10;
extern uint8_t frequency;

// Create a simple slider widget
lv_obj_t *frequency_slider_create(lv_obj_t *parent, lv_event_cb_t slider_changed_cb, const char* label);

#endif // FREQUENCY_SLIDER_H
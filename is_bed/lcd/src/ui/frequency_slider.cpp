
#include "frequency_slider.h"
#include "slider.h"
#include <Arduino.h>

//
// Global variables
//
uint8_t frequency = 1;

//
// Constants
//
static const uint32_t kSliderSize = 50;
static const lv_color_t kKnobColor = lv_color_hex(0xFFFFFF);


//
// Callbacks
//
static void slider_changed_local_cb(lv_event_t *e);

//
// Global functions
//
lv_obj_t *frequency_slider_create(lv_obj_t *parent, lv_event_cb_t slider_changed_cb) {
    lv_obj_t *arc_w = lv_arc_create(parent);
    lv_obj_align(arc_w, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_width(arc_w, 200);
    lv_obj_set_height(arc_w, 200);
    lv_arc_set_range(arc_w, 0, 255);
    lv_obj_add_event_cb(arc_w, slider_changed_local_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(arc_w, slider_changed_local_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_set_style_arc_width(arc_w, 20, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_w, 20, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(arc_w, 2, LV_PART_KNOB);
    lv_obj_set_style_border_opa(arc_w, LV_OPA_100, LV_PART_KNOB);
    lv_obj_set_style_border_color(arc_w, lv_color_black(), LV_PART_KNOB);
    // Add a label on top of the arc to show the curent value
    lv_obj_t *text_shadow_w = lv_label_create(arc_w);
    lv_obj_t *text_w = lv_label_create(arc_w);
    lv_obj_set_style_text_color(text_shadow_w, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(text_w, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(text_shadow_w, LV_ALIGN_BOTTOM_MID, 3, 3);
    lv_obj_align(text_w, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(text_w, LV_FONT_DEFAULT, LV_PART_MAIN);
    lv_obj_set_style_text_font(text_shadow_w, LV_FONT_DEFAULT, LV_PART_MAIN);
    // Initial value
    lv_arc_set_value(arc_w, 128);
    lv_obj_send_event(arc_w, LV_EVENT_KEY, NULL);

    return arc_w;
}

//
// Private callbacks and helper functions
//
static void slider_changed_local_cb(lv_event_t *e)
{
    lv_obj_t *arc_w = (lv_obj_t *)lv_event_get_target(e);
    float val = lv_arc_get_value(arc_w) / 255.0f;
    // Use a quadratic scale for more precision at low frequencies
    val = powf(val, 2.0f);
    float frequency_hz = val * 9.8f + 0.2f;
    String frequency_str = String(frequency_hz) + " Hz";
    lv_obj_t *text_shadow_w = lv_obj_get_child(arc_w, 0);
    lv_obj_t *text_w = lv_obj_get_child(arc_w, 1);
    lv_label_set_text(text_shadow_w, frequency_str.c_str());
    lv_label_set_text(text_w, frequency_str.c_str());
    frequency = (uint8_t) (frequency_hz * 10.0f);
}

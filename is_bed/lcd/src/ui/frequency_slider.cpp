
#include "frequency_slider.h"
#include "slider.h"
#include <Arduino.h>

//
// Global variables
//
uint8_t frequency = 1;

//
// Local variables
//
// The timer for pulsing the frequency display
lv_timer_t *pulse_timer = NULL;

//
// Constants
//
static const uint32_t kSliderSize = 50;
static const lv_color_t kKnobColor = lv_color_hex(0xFFFFFF);


//
// Callbacks
//
static void slider_changed_local_cb(lv_event_t *e);
static void pulse_timer_cb(lv_timer_t *timer);
static void pulse_animation_cb(void *var, int32_t v);

//
// Global functions
//
lv_obj_t *frequency_slider_create(lv_obj_t *parent, lv_event_cb_t slider_changed_cb) {
    lv_obj_t *arc_w = lv_arc_create(parent);
    lv_obj_align(arc_w, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_width(arc_w, 170);
    lv_obj_set_height(arc_w, 170);
    lv_arc_set_range(arc_w, 0, 255);
    lv_obj_add_event_cb(arc_w, slider_changed_local_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(arc_w, slider_changed_local_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_set_style_arc_width(arc_w, 20, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_w, 20, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(arc_w, 2, LV_PART_KNOB);
    lv_obj_set_style_border_opa(arc_w, LV_OPA_100, LV_PART_KNOB);
    lv_obj_set_style_border_color(arc_w, lv_color_black(), LV_PART_KNOB);
    // Add a label on top of the arc to show the curent value
    lv_obj_t *patch_w = lv_obj_create(arc_w);
    lv_obj_set_scrollbar_mode(patch_w, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(patch_w, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(patch_w, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_height(patch_w, 100);
    lv_obj_set_width(patch_w, 100);
    lv_obj_set_style_border_width(patch_w, 0, 0);
    lv_obj_set_style_bg_color(patch_w, lv_color_hex(0x202020), 0);
    lv_obj_t *text_w = lv_label_create(patch_w);
    lv_obj_set_style_text_color(text_w, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(text_w, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(text_w, LV_FONT_DEFAULT, LV_PART_MAIN);
    // Create a timer to pulse the patch
    pulse_timer = lv_timer_create(pulse_timer_cb, 10000 / frequency, arc_w);
    // Initial value
    lv_arc_set_value(arc_w, 150);
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
    lv_obj_t *patch_w = lv_obj_get_child(arc_w, 0);
    lv_obj_t *text_w = lv_obj_get_child(patch_w, 0);
    lv_label_set_text(text_w, frequency_str.c_str());
    frequency = (uint8_t) (frequency_hz * 10.0f);
    lv_timer_set_period(pulse_timer, 10000 / frequency);
}

static void pulse_timer_cb(lv_timer_t *timer) {
    lv_obj_t *arc_w = (lv_obj_t *)lv_timer_get_user_data(timer);
    if (lv_obj_has_flag(arc_w, LV_OBJ_FLAG_HIDDEN)) {
        return; // Do not pulse if the arc is hidden
    }
    lv_obj_t *patch_w = lv_obj_get_child(arc_w, 0);
    lv_obj_set_style_bg_color(patch_w, lv_color_white(), 0);
    
    // Create an animation to pulse the patch
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, pulse_animation_cb);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_duration(&a, 20);
    lv_anim_set_var(&a, patch_w);
    lv_anim_start(&a);
}

static void pulse_animation_cb(void *var, int32_t v) {
    lv_obj_t *patch_w = (lv_obj_t *)var;
    // Set the background color based on the animation value
    if (v == 0) {
        lv_obj_set_style_bg_color(patch_w, lv_color_white(), 0);
    }
    if (v == 255) {
        lv_obj_set_style_bg_color(patch_w, lv_color_black(), 0);
    }
}
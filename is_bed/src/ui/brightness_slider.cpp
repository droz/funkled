
#include "brightness_slider.h"
#include "slider.h"

//
// Callbacks
//
static void slider_changed_local_cb(lv_event_t *e);

//
// Global functions
//
lv_obj_t *brightness_slider_create(lv_obj_t *parent, lv_event_cb_t slider_changed_cb, lv_group_t *encoder_group, uint32_t index) {
    // Create the slider widgget.
    lv_obj_t *slider_w = slider_create(parent, lv_color_hex(0xFFFFFF), slider_changed_cb);
    // Set the user data to the index of the corresponding LED string
    lv_obj_set_user_data(slider_w, (void *)index);
    // Add our own callback to the slider, to take care of the animation
    lv_obj_add_event_cb(slider_w, slider_changed_local_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(slider_w, slider_changed_local_cb, LV_EVENT_PRESSING, NULL);
    // Some custom formatting
    lv_obj_align(slider_w, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_opa(slider_w, LV_OPA_TRANSP, 0);
    lv_obj_set_height(slider_w, 30);
    lv_obj_set_width(slider_w, LV_PCT(90));
    lv_obj_set_style_bg_opa(slider_w, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider_w, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_right(slider_w, 15, 0);
    lv_obj_set_style_pad_left(slider_w, 15, 0);
    // Register the slider with the encoder group
    lv_group_add_obj(encoder_group, slider_w);
    lv_group_focus_obj(slider_w);
    lv_group_set_editing(encoder_group, true);
    lv_group_focus_freeze(encoder_group, true);
    // Call the callback to set the initial value
    lv_slider_set_value(slider_w, 255, LV_ANIM_OFF);
    return slider_w;
}

//
// Private callbacks and helper functions
//
static void slider_changed_local_cb(lv_event_t *e)
{
    lv_obj_t *slider_w = (lv_obj_t *)lv_event_get_target(e);
    // Make sure the slider is still focused, even if the button was clicked
    lv_group_set_editing(lv_obj_get_group(slider_w), true);
}

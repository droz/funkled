
#include "slider.h"

lv_obj_t *slider_create(lv_obj_t *parent, lv_color_t color, lv_event_cb_t slider_pressed_cb)
{
    lv_obj_t *slider_w = lv_slider_create(parent);
    lv_obj_set_style_bg_color(slider_w, color, LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider_w, color, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider_w, LV_OPA_50, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(slider_w, 2, LV_PART_KNOB);
    lv_obj_set_style_border_color(slider_w, lv_color_hex(0x808080), LV_PART_KNOB);
    lv_obj_set_style_pad_right(slider_w, 10, 0);
    lv_obj_set_style_pad_all(slider_w, 1, LV_PART_KNOB);
    lv_obj_set_height(slider_w, 20);
    lv_slider_set_range(slider_w, 0, 15);
    lv_slider_set_value(slider_w, 0, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_w, slider_pressed_cb, LV_EVENT_PRESSING, parent);
    lv_obj_add_event_cb(slider_w, slider_pressed_cb, LV_EVENT_RELEASED, parent);
    lv_obj_add_event_cb(slider_w, slider_pressed_cb, LV_EVENT_KEY, parent);
    lv_obj_add_event_cb(slider_w, slider_pressed_cb, LV_EVENT_LONG_PRESSED, parent);
    lv_obj_add_event_cb(slider_w, slider_pressed_cb, LV_EVENT_LONG_PRESSED_REPEAT, parent);
    lv_obj_add_flag(slider_w, LV_OBJ_FLAG_EVENT_BUBBLE);
    return slider_w;
}
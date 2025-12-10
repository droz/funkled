#include "pattern_slider.h"
#include "led_pattern.h"
#include "led_array.h"
#include <lvgl.h>
#include <Arduino.h>

//
// Consts
//
static const uint32_t slider_size = 45; // Size of the buttons

//
// Local variables
//
// This is the main button that indicates which pattern is selected
static lv_obj_t *pattern_button_w = NULL;
static lv_obj_t *pattern_label_w = NULL;
// The pattern that the UI is currently on
static uint32_t current_pattern_index = 0;
// The callback to call when the pattern changes
static pattern_changed_cb_t pattern_changed_cb = NULL;

//
// Static prototypes
//
static void left_btn_event_cb(lv_event_t *e);
static void right_btn_event_cb(lv_event_t *e);

String slider_options() {
    // Allocate memory (+1 for the null-terminator)
    String options;
    for (uint32_t i = 0; i < num_led_patterns(); i++)
    {
        if (options.length() > 0) {
            options += "\n";
        }
        options += led_patterns[i].name;
    }
    return options;
}

//
// Global functions
//
lv_obj_t *pattern_slider_create(lv_obj_t *parent, pattern_changed_cb_t callback, lv_group_t *encoder_group)
{
    // Save the callback
    pattern_changed_cb = callback;

    lv_obj_t* slider_w = lv_obj_create(parent);
    static const int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static const int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(slider_w, grid_col_dsc, grid_row_dsc);
    lv_obj_set_width(slider_w, LV_PCT(97));
    lv_obj_align(slider_w, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_height(slider_w, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(slider_w, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(slider_w, LV_OPA_0, 0);
    lv_obj_set_style_pad_gap(slider_w, 3, 0);
    lv_obj_set_style_pad_all(slider_w, 1, 0);

    // Left button
    lv_obj_t *left_btn_w = lv_btn_create(slider_w);
    lv_obj_add_event_cb(left_btn_w, left_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(left_btn_w, slider_size, slider_size);
    lv_obj_t *lbl_left = lv_label_create(left_btn_w);
    lv_label_set_text(lbl_left, LV_SYMBOL_LEFT);
    lv_obj_center(lbl_left);
    lv_obj_set_grid_cell(left_btn_w, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_END, 0, 1);
    lv_obj_set_style_text_font(left_btn_w, LV_FONT_DEFAULT, LV_PART_MAIN);

    // Center button
    pattern_button_w = lv_btn_create(slider_w);
    lv_obj_set_style_bg_color(pattern_button_w, lv_color_hex(0x505050), 0);
    lv_obj_set_height(pattern_button_w, slider_size);
    pattern_label_w = lv_label_create(pattern_button_w);
    lv_label_set_text(pattern_label_w, led_patterns[current_pattern_index].name);
    lv_obj_center(pattern_label_w);
    lv_obj_set_grid_cell(pattern_button_w, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_END, 0, 1);
    lv_obj_set_style_text_font(pattern_label_w, LV_FONT_DEFAULT, LV_PART_MAIN);

    // Right button
    lv_obj_t *right_btn_w = lv_btn_create(slider_w);
    lv_obj_add_event_cb(right_btn_w, right_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(right_btn_w, slider_size, slider_size);
    lv_obj_t *lbl_right = lv_label_create(right_btn_w);
    lv_label_set_text(lbl_right, LV_SYMBOL_RIGHT);
    lv_obj_center(lbl_right);
    lv_obj_set_grid_cell(right_btn_w, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_END, 0, 1);
    lv_obj_set_style_text_font(right_btn_w, LV_FONT_DEFAULT, LV_PART_MAIN);

    return slider_w;
}

void pattern_slider_set_pattern(lv_obj_t *slider_w, uint32_t pattern_index)
{
    if (pattern_index >= num_led_patterns()) {
        return; // Invalid pattern index
    }
    current_pattern_index = pattern_index;
    lv_label_set_text(pattern_label_w, led_patterns[current_pattern_index].name);
}

//
// Private callbacks and helper functions
//

// Left button callback
static void left_btn_event_cb(lv_event_t *e)
{
    // Decrement the pattern index
    current_pattern_index = (current_pattern_index == 0) ? num_led_patterns() - 1 : current_pattern_index - 1;
    pattern_slider_set_pattern(pattern_button_w, current_pattern_index);
    pattern_changed_cb(current_pattern_index);
}

// Right button callback
static void right_btn_event_cb(lv_event_t *e)
{
    // Increment the pattern index
    current_pattern_index = (current_pattern_index + 1) % num_led_patterns();
    pattern_slider_set_pattern(pattern_button_w, current_pattern_index);
    pattern_changed_cb(current_pattern_index);
}



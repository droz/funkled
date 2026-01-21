#include "pattern_slider.h"
#include <lvgl.h>
#include <Arduino.h>

//
// Constants
//
static const uint32_t kSliderSize = 45; // Size of the buttons

//
// Shared variables
//
String pattern_names[MAX_LED_PATTERNS];
uint8_t pattern_types[MAX_LED_PATTERNS];
size_t num_patterns = 0;
uint32_t displayed_pattern_index = 0;
uint32_t selected_pattern_index = 0;

//
// Local variables
//
// This is the main button that indicates which pattern is selected
static lv_obj_t *pattern_button_w = NULL;
static lv_obj_t *pattern_label_w = NULL;
// The callback to call when the pattern changes
static pattern_changed_cb_t pattern_changed_cb = NULL;

//
// Static prototypes
//
static void left_btn_event_cb(lv_event_t *e);
static void right_btn_event_cb(lv_event_t *e);

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
    lv_obj_set_size(left_btn_w, kSliderSize, kSliderSize);
    lv_obj_t *lbl_left = lv_label_create(left_btn_w);
    lv_label_set_text(lbl_left, LV_SYMBOL_LEFT);
    lv_obj_center(lbl_left);
    lv_obj_set_grid_cell(left_btn_w, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_END, 0, 1);
    lv_obj_set_style_text_font(left_btn_w, LV_FONT_DEFAULT, LV_PART_MAIN);

    // Center button
    pattern_button_w = lv_btn_create(slider_w);
    lv_obj_set_style_bg_color(pattern_button_w, lv_color_hex(0x505050), 0);
    lv_obj_set_height(pattern_button_w, kSliderSize);
    pattern_label_w = lv_label_create(pattern_button_w);
    lv_label_set_text(pattern_label_w, "");
    lv_obj_center(pattern_label_w);
    lv_obj_set_grid_cell(pattern_button_w, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_END, 0, 1);
    lv_obj_set_style_text_font(pattern_label_w, LV_FONT_DEFAULT, LV_PART_MAIN);

    // Right button
    lv_obj_t *right_btn_w = lv_btn_create(slider_w);
    lv_obj_add_event_cb(right_btn_w, right_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(right_btn_w, kSliderSize, kSliderSize);
    lv_obj_t *lbl_right = lv_label_create(right_btn_w);
    lv_label_set_text(lbl_right, LV_SYMBOL_RIGHT);
    lv_obj_center(lbl_right);
    lv_obj_set_grid_cell(right_btn_w, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_END, 0, 1);
    lv_obj_set_style_text_font(right_btn_w, LV_FONT_DEFAULT, LV_PART_MAIN);

    return slider_w;
}

void pattern_slider_set_pattern(uint32_t pattern_index)
{
    if (pattern_index >= num_patterns) {
        return; // Invalid pattern index
    }
    displayed_pattern_index = pattern_index;
    pattern_changed_cb(displayed_pattern_index);
    lv_label_set_text(pattern_label_w, pattern_names[displayed_pattern_index].c_str());
}

//
// Private callbacks and helper functions
//

// Left button callback
static void left_btn_event_cb(lv_event_t *e)
{
    // Decrement the pattern index
    displayed_pattern_index = (displayed_pattern_index == 0) ? num_patterns - 1 : displayed_pattern_index - 1;
    pattern_slider_set_pattern(displayed_pattern_index);
}

// Right button callback
static void right_btn_event_cb(lv_event_t *e)
{
    // Increment the pattern index
    displayed_pattern_index = (displayed_pattern_index + 1) % num_patterns;
    pattern_slider_set_pattern(displayed_pattern_index);
}



#include "color_selector.h"
#include "slider.h"
#include <Arduino.h>

//
// Global variables
//
lv_color_t selected_color = lv_color_hex(0xFFFFFF);

//
// Static prototypes
//
static void colorwheel_pressed_cb(lv_event_t * e);

//
// Global functions
//
lv_obj_t* color_selector_create(lv_obj_t* parent, color_changed_cb_t cb) {
    lv_obj_t* color_selector_w = lv_btn_create(parent);
    // Use the widget user data to store the callback
    lv_obj_set_user_data(color_selector_w, (void*) cb);
    static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, 30, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(color_selector_w, grid_col_dsc, grid_row_dsc);
    lv_obj_add_flag(color_selector_w, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_bg_opa(color_selector_w, LV_OPA_0, 0);
    lv_obj_set_style_pad_all(color_selector_w, 0, 0);
    lv_obj_set_style_pad_right(color_selector_w, 10, 0);
    lv_obj_set_style_border_opa(color_selector_w, LV_OPA_0, 0);
    lv_obj_set_style_shadow_opa(color_selector_w, LV_OPA_0, 0);
    lv_obj_set_align(color_selector_w, LV_ALIGN_TOP_MID);
    // The color wheel
    LV_IMAGE_DECLARE(img_colorwheel); 
    lv_obj_t* image_w = lv_image_create(color_selector_w);
    lv_image_set_src(image_w, &img_colorwheel);
    lv_obj_set_grid_cell(image_w, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_add_event_cb(color_selector_w, colorwheel_pressed_cb, LV_EVENT_PRESSING, color_selector_w);
    lv_obj_add_event_cb(color_selector_w, colorwheel_pressed_cb, LV_EVENT_RELEASED, color_selector_w);
    lv_obj_add_flag(image_w, LV_OBJ_FLAG_EVENT_BUBBLE);
    // The color patch
    lv_obj_t* patch_w = lv_obj_create(color_selector_w);
    lv_obj_set_scrollbar_mode(patch_w, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(patch_w, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_height(patch_w, 30);
    lv_obj_set_width(patch_w, 100);
    lv_obj_set_style_radius(patch_w, 10, 0);
    lv_obj_set_grid_cell(patch_w, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_clear_flag(patch_w, LV_OBJ_FLAG_CLICKABLE);
    // Initial color set
    change_color(color_selector_w, lv_color_hex(0xFFFFFF));

    return color_selector_w;
}

//
// Private callbacks
//
static void colorwheel_pressed_cb(lv_event_t * e) {
    lv_indev_t * indev = lv_indev_active();
    if(indev == NULL) return;
    lv_indev_type_t indev_type = lv_indev_get_type(indev);
    if(indev_type != LV_INDEV_TYPE_POINTER) return;
    lv_point_t p;
    lv_indev_get_point(indev, &p);
    // Get coordinates relative to the center of the image
    lv_obj_t* color_selector_w = (lv_obj_t*) lv_event_get_user_data(e);
    lv_obj_t* image_w = lv_obj_get_child(color_selector_w, 0);
    lv_area_t image_coords;
    lv_obj_get_coords(image_w, &image_coords);
    p.x -= image_coords.x1;
    p.y -= image_coords.y1;
    // Get the pixel color and alpha
    lv_image_dsc_t* image_dcs = (lv_image_dsc_t*) lv_image_get_src(image_w);
    if (image_dcs->header.cf != LV_COLOR_FORMAT_RGB565A8) {
        return;
    }
    if (p.x < 0 || p.y < 0 || p.x >= image_dcs->header.w || p.y >= image_dcs->header.h) {
        return;
    }
    uint32_t offset_rgb = (p.y * image_dcs->header.w + p.x) * 2;
    uint32_t offset_alpha = image_dcs->header.w * image_dcs->header.h * 2 + p.y * image_dcs->header.w + p.x;
    uint8_t alpha = image_dcs->data[offset_alpha];
    if (alpha == 0) {
        return;
    }
    uint16_t rgb = image_dcs->data[offset_rgb] | (image_dcs->data[offset_rgb + 1] << 8);
    uint8_t r = (rgb & 0xF800) >> 8;
    uint8_t g = (rgb & 0x07E0) >> 3;
    uint8_t b = (rgb & 0x001F) << 3;
    lv_color_t color = lv_color_make(r, g, b);
    change_color(color_selector_w, color);
    return;
}

void change_color(lv_obj_t* color_selector_w, lv_color_t color) {
    lv_obj_t* patch_w = lv_obj_get_child(color_selector_w, 1);
    lv_obj_set_style_bg_color(patch_w, color, 0);
    selected_color = color;
    // Call the callback
    color_changed_cb_t cb = (color_changed_cb_t) lv_obj_get_user_data(color_selector_w);
    cb(color);
}


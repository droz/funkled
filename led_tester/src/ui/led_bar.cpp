#include "led_bar.h"
#include "led_pattern.h"
#include "led_palette.h"
#include "led_array.h"
#include <Arduino.h>

//
// Typedefs
//
// This internal struct is used to store all the information related to the LED
// widget. It is not exposed to the user.
typedef struct {
    // The horizontal grid descriptor array
    int32_t* grid_col_dsc;
    // The channel index
    uint32_t channel;
    // The number of LEDs in the array
    uint32_t num_leds;
    // The information about the pattern
    led_bar_dsc_t dsc;
} led_bar_data_t;


//
// Static prototypes
//
static lv_obj_t* led_widget_create(lv_obj_t* parent, int size);
static void led_bar_delete_cb(lv_event_t * e);
static void led_bar_timer_cb(lv_timer_t* timer);
static void led_set_color(lv_obj_t * led, lv_color_t color);

//
// Global functions
//
lv_obj_t* led_bar_create(
        lv_obj_t* parent,
        uint32_t num_leds,
        uint32_t channel,
        const led_bar_dsc_t* dsc)
{
    lv_obj_t* led_bar_w = lv_obj_create(parent);
    // The grid descriptor arrays do not get copied by the lvgl library, so
    // they cannot be static or locally scoped.
    // allocate a new array of the size of the number of LEDs plus one for the
    // last template value
    int32_t* grid_col_dsc = new int32_t[num_leds + 1];
    for (uint32_t i = 0; i < num_leds; i++) {
        grid_col_dsc[i] = LV_GRID_FR(1);
    }
    grid_col_dsc[num_leds] = LV_GRID_TEMPLATE_LAST;
    static const int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(led_bar_w, grid_col_dsc, grid_row_dsc);
    for (uint32_t i = 0; i < num_leds; i++) {
        lv_obj_t* led_w = led_widget_create(led_bar_w, 15);
        lv_obj_set_grid_cell(led_w, LV_GRID_ALIGN_STRETCH, i, 1, LV_GRID_ALIGN_END, 0, 1);
    }
    lv_obj_set_height(led_bar_w, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(led_bar_w, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(led_bar_w, LV_OPA_0, 0);
    lv_obj_set_style_pad_gap(led_bar_w, 3, 0);
    lv_obj_set_style_pad_all(led_bar_w, 1, 0);
    lv_obj_clear_flag(led_bar_w, LV_OBJ_FLAG_CLICKABLE);

    // Create a struct with all the internal data
    led_bar_data_t* led_data = new led_bar_data_t;
    led_data->grid_col_dsc = grid_col_dsc;
    led_data->dsc = *dsc;
    led_data->channel = channel;
    led_data->num_leds = num_leds;
    lv_obj_set_user_data(led_bar_w, led_data);

    // Create a timer for this LED bar
    lv_timer_create(led_bar_timer_cb, 50, led_bar_w);

    // Register a callback for deletion of the widget and associated storage
    lv_obj_add_event_cb(led_bar_w, led_bar_delete_cb, LV_EVENT_DELETE, NULL);
    return led_bar_w;
}

//
// Private static functions
//

static lv_obj_t* led_widget_create(lv_obj_t* parent, int size)
{
    // The lvgl LED object has very non-linear way to control the brightness
    // and halo around the widget. So instead we make our own using similar idea
    // (a simple widget with a variable shadow)
    lv_obj_t* led_w = lv_obj_create(parent);
    lv_obj_set_height(led_w, size);
    lv_obj_set_width(led_w, size);
    lv_obj_set_style_pad_all(led_w, 0, 0);
    lv_obj_set_style_radius(led_w, size * 2 / 5, 0);
    lv_obj_set_style_border_width(led_w, 0, 0);
    lv_obj_set_style_shadow_spread(led_w, 2, 0);
    lv_obj_set_style_shadow_width(led_w, 5, 0);
    lv_obj_set_style_bg_opa(led_w, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_main_opa(led_w, LV_OPA_COVER, 0);
    led_set_color(led_w, lv_color_hex(0x000000));
    lv_obj_clear_flag(led_w, LV_OBJ_FLAG_CLICKABLE);

    return led_w;
}

static void led_set_color(lv_obj_t * led, lv_color_t color)
{
    // Figure out the overall brightness of the color
    uint32_t brightness = max(max(color.red, color.green), color.blue);
    uint32_t shadow_opacity = brightness < 128 ? 0 : (brightness - 128) * 2;
    lv_obj_set_style_bg_color(led, color, 0);
    lv_obj_set_style_shadow_color(led, color, 0);
    lv_obj_set_style_shadow_opa(led, shadow_opacity, 0);
}

//
// Private callbacks
//
static void led_bar_delete_cb(lv_event_t * e) {
    lv_obj_t* led_bar_w = (lv_obj_t*) lv_event_get_target(e);
    led_bar_data_t* led_data = (led_bar_data_t*) lv_obj_get_user_data(led_bar_w);
    delete[] led_data->grid_col_dsc;
    delete led_data;
}

static void led_bar_timer_cb(lv_timer_t * timer)
{
    // Get the user data
    lv_obj_t* led_bar_w = (lv_obj_t*) lv_timer_get_user_data(timer);
    led_bar_data_t* led_bar_data = (led_bar_data_t*) lv_obj_get_user_data(led_bar_w);

    // Fill the LED array with the pattern
    // If any of the information is not available, use the current one
    uint32_t channel = led_bar_data->channel;
    if (channel == CHANNEL_CURRENT) {
        channel = current_channel;
    }
    const led_palette_t* palette = led_bar_data->dsc.palette;
    if (palette == NULL) {
        palette = &led_palettes[led_strings[channel].palette_index];
    }
    CRGB color = led_strings[channel].single_color;
    if (led_bar_data->dsc.single_color) {
        color = *led_bar_data->dsc.single_color;
    }
    led_pattern_func_t pattern_update = led_bar_data->dsc.pattern_update;
    if (pattern_update == NULL) {
        pattern_update = led_patterns[led_strings[channel].pattern_index].update;
    }
    uint32_t period_ms = led_strings[channel].update_period_ms;
    if (led_bar_data->dsc.period_ms) {
        period_ms = *led_bar_data->dsc.period_ms;
    }
    uint8_t brightness = led_strings[channel].brightness;
    if (led_bar_data->dsc.brightness) {
        brightness = *led_bar_data->dsc.brightness;
    }
    uint32_t time_ms = millis();
    CRGB leds[led_bar_data->num_leds];
    pattern_update(time_ms, period_ms, composed_palette(palette, color), color, led_bar_data->num_leds, leds);

    // Update the LEDs
    for (uint32_t i = 0; i < led_bar_data->num_leds; i++) {
        lv_obj_t* led = lv_obj_get_child(led_bar_w, i);
        CRGB color_crgb = leds[i];
        color_crgb.nscale8(brightness);
        // Apply some simple gamma correction, to make the LEDs more realistic looking
        color_crgb = applyGamma_video (color_crgb, 0.35);
        lv_color_t color_lv = lv_color_make(color_crgb.red, color_crgb.green, color_crgb.blue);
        led_set_color(led, color_lv);
    }
}

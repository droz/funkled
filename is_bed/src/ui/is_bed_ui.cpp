#include "is_bed_ui.h"
#include "composite_image.h"
#include "led_pattern.h"
#include "led_palette.h"
#include "led_array.h"
#include "slider.h"
#include "brightness_slider.h"
#include "pattern_slider.h"
#include <Arduino.h>
#include <FastLED.h>

//
// Constants
//
static const uint32_t sliders_hide_delay_ms = 2000; // Time after which the sliders hide
static const uint32_t sliders_anim_time_ms = 400;   // Time for the slider show/hide animation
static const uint32_t sliders_spacing = 45;        // Vertical spacing between sliders

//
// Static prototypes
//
static void brightness_changed_cb(lv_event_t *e);
static void pattern_changed_cb(uint32_t pattern_index);
static void ok_btn_event_cb(lv_event_t *e);
static void cancel_btn_event_cb(lv_event_t *e);
static void off_btn_event_cb(lv_event_t *e);
static void background_clicked_cb(lv_event_t *e);
static lv_obj_t *ok_button_create(lv_obj_t *parent);
static lv_obj_t *cancel_button_create(lv_obj_t *parent);
static lv_obj_t *off_button_create(lv_obj_t *parent);
static void animate_sliders(bool show);
static void sliders_anim_cb(void *var, int32_t v);
static void timer_hide_sliders_cb(lv_timer_t *timer);
static void show_sliders();

//
// Static variables
//
// Fonts
static const lv_font_t *font_large;
static const lv_font_t *font_normal;
// Some widgets we want to keep around for easy access
static lv_obj_t *background_image_w;
static lv_obj_t *center_brightness_w;
static lv_obj_t *front_brightness_w;
static lv_obj_t *headboard_brightness_w;
static lv_obj_t *cage_brightness_w;
static lv_obj_t *pattern_slider_w;
static lv_obj_t *ok_btn_w;
static lv_obj_t *cancel_btn_w;
static lv_obj_t *off_button_w;
// This timer is used to hide the slider after a while
static lv_timer_t *sliders_hide_timer = NULL;


// The encoder groups
static lv_group_t *encoder_groups[4];

// The composite image of the bed
LV_IMAGE_DECLARE(background);
LV_IMAGE_DECLARE(center);
LV_IMAGE_DECLARE(front);
LV_IMAGE_DECLARE(headboard);
LV_IMAGE_DECLARE(cage);
static composite_image_layer_t layers[] = {
    {.image_dsc = cage,
     .led_string = &led_strings[0]},
    {.image_dsc = center,
     .led_string = &led_strings[1]},
    {.image_dsc = front,
     .led_string = &led_strings[2]},
    {.image_dsc = headboard,
     .led_string = &led_strings[4]}};
static DMAMEM uint8_t composite_buffer[TFT_HOR_RES * TFT_VER_RES * 3];
static composite_image_dsc_t composite_dsc = {
    .buffer = composite_buffer,
    .background_image_dsc = background,
    .layers = layers,
    .layer_count = sizeof(layers) / sizeof(layers[0])};

//
// Global functions
//

void is_bed_ui(void)
{
    font_large = &lv_font_montserrat_16;
    font_normal = &lv_font_montserrat_12;
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK,
                          font_normal);

    // Figure out which input devices we have.
    for (lv_indev_t *indev = lv_indev_get_next(NULL); indev != NULL; indev = lv_indev_get_next(indev))
    {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER)
        {
            // Get the encoder index from the user data
            uint32_t encoder_index = (uint32_t)lv_indev_get_user_data(indev);
            // Add the input device to the corresponding group
            encoder_groups[encoder_index] = lv_group_create();
            lv_indev_set_group(indev, encoder_groups[encoder_index]);
            // Disable wrapping for the group
            lv_group_set_wrap(encoder_groups[encoder_index], false);
        }
    }

    // Make sure that the top level screen is not scrollable
    lv_obj_t *screen_w = lv_screen_active();
    lv_obj_clear_flag(screen_w, LV_OBJ_FLAG_SCROLLABLE);

    // Add the various widgets
    background_image_w = composite_image_create(screen_w, background_clicked_cb, &composite_dsc);
    pattern_slider_w = pattern_slider_create(screen_w, pattern_changed_cb, encoder_groups[0]);
    center_brightness_w = brightness_slider_create(screen_w, brightness_changed_cb, encoder_groups[0], ZONE_CENTER);
    front_brightness_w = brightness_slider_create(screen_w, brightness_changed_cb, encoder_groups[1], ZONE_FRONT);
    headboard_brightness_w = brightness_slider_create(screen_w, brightness_changed_cb, encoder_groups[2], ZONE_HEADBOARD);
    cage_brightness_w = brightness_slider_create(screen_w, brightness_changed_cb, encoder_groups[3], ZONE_CAGE);
    ok_btn_w = ok_button_create(screen_w);
    cancel_btn_w = cancel_button_create(screen_w);
    off_button_w = off_button_create(screen_w);
    // Finally, send an initial event to locate the sliders correctly
    lv_obj_send_event(center_brightness_w, LV_EVENT_KEY, NULL);
}

//
// Private helper functions
//
static lv_obj_t *ok_button_create(lv_obj_t *parent)
{
    lv_obj_t *btn_w = lv_btn_create(parent);
    lv_obj_add_event_cb(btn_w, ok_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_w, lv_color_hex(0x64C364), 0);
    lv_obj_set_size(btn_w, 50, 50);
    lv_obj_align(btn_w, LV_ALIGN_BOTTOM_RIGHT, -5, -70);
    lv_obj_set_style_radius(btn_w, LV_RADIUS_CIRCLE, 0);
    lv_obj_t *label_w = lv_label_create(btn_w);
    lv_label_set_text(label_w, LV_SYMBOL_OK);
    lv_obj_center(label_w);
    lv_obj_set_style_text_font(label_w, LV_FONT_DEFAULT, LV_PART_MAIN);
    // We make the button invisible for now
    lv_obj_add_flag(btn_w, LV_OBJ_FLAG_HIDDEN);

    return btn_w;
}
static lv_obj_t *cancel_button_create(lv_obj_t *parent)
{
    lv_obj_t *btn_w = lv_btn_create(parent);
    lv_obj_add_event_cb(btn_w, cancel_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_w, lv_color_hex(0xC36464), 0);
    lv_obj_set_size(btn_w, 50, 50);
    lv_obj_align(btn_w, LV_ALIGN_BOTTOM_LEFT, 5, -70);
    lv_obj_set_style_radius(btn_w, LV_RADIUS_CIRCLE, 0);
    lv_obj_t *label_w = lv_label_create(btn_w);
    lv_label_set_text(label_w, LV_SYMBOL_CLOSE);
    lv_obj_center(label_w);
    lv_obj_set_style_text_font(label_w, LV_FONT_DEFAULT, LV_PART_MAIN);
    // We make the button invisible for now
    lv_obj_add_flag(btn_w, LV_OBJ_FLAG_HIDDEN);

    return btn_w;
}
static lv_obj_t *off_button_create(lv_obj_t *parent)
{
    lv_obj_t *btn_w = lv_btn_create(parent);
    lv_obj_add_event_cb(btn_w, off_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_w, lv_color_hex(0xC36464), 0);
    lv_obj_set_size(btn_w, 50, 50);
    lv_obj_align(btn_w, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_radius(btn_w, LV_RADIUS_CIRCLE, 0);
    lv_obj_t *label_w = lv_label_create(btn_w);
    lv_label_set_text(label_w, LV_SYMBOL_POWER);
    lv_obj_center(label_w);
    lv_obj_set_style_text_font(label_w, LV_FONT_DEFAULT, LV_PART_MAIN);

    return btn_w;
}

static void animate_sliders(bool show)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, sliders_anim_cb);
    if (show) {
        lv_anim_set_values(&a, 256, 0);
    } else {
        lv_anim_set_values(&a, 0, 256);
    }
    lv_anim_set_duration(&a, sliders_anim_time_ms);
    lv_anim_start(&a);
}

//
// Callbacks
//

static void show_sliders() {
    // If there is already a timer waiting and in this case we just reset it
    if (sliders_hide_timer != NULL) {
        lv_timer_reset(sliders_hide_timer);
    } else {
        // The timer does not exist, create it
        sliders_hide_timer = lv_timer_create(timer_hide_sliders_cb, sliders_hide_delay_ms, NULL);
        lv_timer_set_repeat_count(sliders_hide_timer, 1);                           // Only run once
    }
    // If the sliders are already visible, no need to animate it again
    if (lv_obj_get_style_opa(center_brightness_w, 0) == LV_OPA_TRANSP)
    {
        animate_sliders(true);
    }
}

static void brightness_changed_cb(lv_event_t *e)
{
    // First we show the sliders
    show_sliders();
    // Get the slider widget that triggered the event
    lv_obj_t *slider_w = (lv_obj_t *)lv_event_get_target(e);
    // Get the zone of the corresponding LEDs from the user data
    uint32_t zone = (uint32_t)lv_obj_get_user_data(slider_w);
    // Get the current brightness value from the slider
    int32_t min_value = lv_slider_get_min_value(slider_w);
    int32_t max_value = lv_slider_get_max_value(slider_w);
    int32_t slider_value = lv_slider_get_value(slider_w);
    float_t slider_percent = (float)slider_value / (max_value - min_value);
    uint32_t brightness = slider_percent * 255.0;
    // When the user presses the button, we want to update the brightness to zero or full
    if (lv_event_get_code(e) == LV_EVENT_KEY && lv_event_get_key(e) == LV_KEY_ENTER) {
        if (brightness > 128) {
            brightness = 0; // Turn off the LEDs
        } else {
            brightness = 255; // Turn on the LEDs
        }
        lv_slider_set_value(slider_w, brightness, LV_ANIM_OFF); // Update the slider value
    }
    // Update the corresponding LED string brightness
    led_zones[zone].brightness = brightness;
}


static void pattern_changed_cb(uint32_t pattern_index)
{
    bool changed = false;
    for (uint32_t i = 0; i < num_zones; i++)
    {
          led_zones[i].ui_pattern_index = pattern_index;
          if (led_zones[i].led_pattern_index != pattern_index) {
              changed = true;
          }
    }
    // Show or hide the validate buttons depending on whether there was a change
    if (changed) {
        lv_obj_clear_flag(ok_btn_w, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(cancel_btn_w, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ok_btn_w, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cancel_btn_w, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ok_btn_event_cb(lv_event_t *e)
{
    // When the user clicks the validate button, we copy the UI pattern index to the actual pattern index
    for (uint32_t i = 0; i < num_zones; i++)
    {
          led_zones[i].led_pattern_index = led_zones[i].ui_pattern_index;
    }
    // Hide the validate buttons
    lv_obj_add_flag(ok_btn_w, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cancel_btn_w, LV_OBJ_FLAG_HIDDEN);
}

static void cancel_btn_event_cb(lv_event_t *e)
{
    // When the user clicks the cancel button, we revert the UI pattern index to the actual pattern index
    for (uint32_t i = 0; i < num_zones; i++)
    {
          led_zones[i].ui_pattern_index = led_zones[i].led_pattern_index;
    }
    // Update the pattern slider to reflect the reverted pattern
    uint32_t pattern_index = led_zones[0].led_pattern_index;
    pattern_slider_set_pattern(pattern_slider_w, pattern_index);
    // Hide the validate buttons
    lv_obj_add_flag(ok_btn_w, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cancel_btn_w, LV_OBJ_FLAG_HIDDEN);
}

static void off_btn_event_cb(lv_event_t *e)
{
    // When the user clicks the off button, we set all brightnesses to zero
    for (uint32_t i = 0; i < num_zones; i++)
    {
          led_zones[i].brightness = 0;
    }
    // Update all brightness sliders to reflect the change
    lv_slider_set_value(center_brightness_w, 0, LV_ANIM_OFF);
    lv_slider_set_value(front_brightness_w, 0, LV_ANIM_OFF);
    lv_slider_set_value(headboard_brightness_w, 0, LV_ANIM_OFF);
    lv_slider_set_value(cage_brightness_w, 0, LV_ANIM_OFF);
}

static void sliders_anim_cb(void *var, int32_t v)
{
    int32_t y = lv_map(v, 0, 256, 70, -200);
    int32_t opa = lv_map(v, 0, 256, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_obj_set_style_opa(center_brightness_w, opa, 0);
    lv_obj_set_style_opa(front_brightness_w, opa, 0);
    lv_obj_set_style_opa(headboard_brightness_w, opa, 0);
    lv_obj_set_style_opa(cage_brightness_w, opa, 0);
    lv_obj_set_y(center_brightness_w, y);
    lv_obj_set_y(front_brightness_w, y + sliders_spacing);
    lv_obj_set_y(headboard_brightness_w, y + 2 * sliders_spacing);
    lv_obj_set_y(cage_brightness_w, y + 3 * sliders_spacing);
}

static void timer_hide_sliders_cb(lv_timer_t *timer)
{
    animate_sliders(false);
    lv_timer_del(timer);
    sliders_hide_timer = NULL; // Clear the timer reference
}

static void background_clicked_cb(lv_event_t *e)
{
    show_sliders();
}
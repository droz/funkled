#include "pattern_roller.h"
#include "led_pattern.h"
#include <lvgl.h>
#include <Arduino.h>

//
// Local variables
//
// This timer is used to hide the roller after a while
static lv_timer_t *roller_hide_timer = NULL;

//
// Static prototypes
//
static void animate_roller(lv_obj_t *roller_w, bool show, bool fast);
static void roller_anim_cb(void *var, int32_t v);
static void roller_changed_local_cb(lv_event_t *e);
static void timer_hide_roller_cb(lv_timer_t *timer);

String roller_options() {
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
lv_obj_t *pattern_roller_create(lv_obj_t *parent, lv_event_cb_t pattern_changed_cb, lv_group_t *encoder_group, uint32_t index)
{
    lv_obj_t * roller_w = lv_roller_create(parent);
    const String options = roller_options();
    lv_roller_set_options(roller_w,
                          options.c_str(),
                          LV_ROLLER_MODE_INFINITE);

    lv_roller_set_visible_row_count(roller_w, 5);
    lv_obj_center(roller_w);
    lv_obj_add_event_cb(roller_w, pattern_changed_cb, LV_EVENT_ALL, NULL);
    
    // Register the roller with the encoder group
    lv_group_add_obj(encoder_group, roller_w);
    lv_group_focus_obj(roller_w);
    lv_group_set_editing(encoder_group, true);
    lv_group_focus_freeze(encoder_group, true);

     // Add our own callback to the roller, to take care of the animation
    lv_obj_add_event_cb(roller_w, roller_changed_local_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(roller_w, roller_changed_local_cb, LV_EVENT_PRESSING, NULL);
  
    // Call the callback to set the initial value
    lv_roller_set_selected(roller_w, 0, LV_ANIM_OFF);
    lv_obj_send_event(roller_w, LV_EVENT_KEY, NULL);

    return roller_w;
}

//
// Private callbacks and helper functions
//
static void roller_changed_local_cb(lv_event_t *e)
{
    lv_obj_t *roller_w = (lv_obj_t *)lv_event_get_target(e);
    // Make sure the roller is still focused, even if the button was clicked
    lv_group_set_editing(lv_obj_get_group(roller_w), true);
    // First we check if there is already a timer waiting
    if (roller_hide_timer != NULL)
    {
        lv_obj_t *timer_roller_w = (lv_obj_t *)lv_timer_get_user_data(roller_hide_timer);
        if (timer_roller_w != roller_w)
        {
            // This is a different roller, we need to hide it first and delete the timer
            animate_roller(timer_roller_w, false, true);
            lv_timer_del(roller_hide_timer);
            roller_hide_timer = NULL;
        }
        else
        {
            // This is the same roller, we can just reset the timer
            lv_timer_reset(roller_hide_timer);
        }
    }
    // Check again for the timer, it might have been deleted
    if (roller_hide_timer == NULL)
    {
        // The timer does not exist, create it
        roller_hide_timer = lv_timer_create(timer_hide_roller_cb, 3000, roller_w); // Hide after 3 seconds
        lv_timer_set_repeat_count(roller_hide_timer, 1);                           // Only run once
    }
    // If the roller is already visible, no need to animate it again
    if (lv_obj_get_style_opa(roller_w, 0) == LV_OPA_TRANSP)
    {
        animate_roller(roller_w, true, false);
    }
}

static void animate_roller(lv_obj_t *roller_w, bool show, bool fast)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, roller_w);
    lv_anim_set_exec_cb(&a, roller_anim_cb);
    if (show)
    {
        lv_anim_set_values(&a, 256, 0);
    }
    else
    {
        lv_anim_set_values(&a, 0, 256);
    }
    lv_anim_set_duration(&a, (show || fast) ? 400 : 2000);
    lv_anim_start(&a);
}

static void roller_anim_cb(void *var, int32_t v)
{
    lv_obj_t *roller_w = (lv_obj_t *)var;
    int32_t y = lv_map(v, 0, 256, -20, 40);
    int32_t opa = lv_map(v, 0, 256, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_obj_set_style_opa(roller_w, opa, 0);
    lv_obj_set_y(roller_w, y);
}

static void timer_hide_roller_cb(lv_timer_t *timer)
{
    lv_obj_t *roller_w = (lv_obj_t *)lv_timer_get_user_data(timer);
    animate_roller(roller_w, false, false);
    lv_timer_del(timer);
    roller_hide_timer = NULL; // Clear the timer reference
}
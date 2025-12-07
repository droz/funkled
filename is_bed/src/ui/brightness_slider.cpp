
#include "brightness_slider.h"
#include "slider.h"

//
// Local variables
//
// This timer is used to hide the slider after a while
static lv_timer_t *slider_hide_timer = NULL;

//
// Static prototypes
//
static void animate_slider(lv_obj_t *slider_w, bool show, bool fast);
static void slider_anim_cb(void *var, int32_t v);
static void slider_changed_local_cb(lv_event_t *e);
static void timer_hide_slider_cb(lv_timer_t *timer);

//
// Global functions
//
lv_obj_t *brightness_slider_create(lv_obj_t *parent, lv_event_cb_t slider_changed_cb, lv_group_t *encoder_group, uint32_t index)
{
    // Create the slider widgget.
    lv_obj_t *slider_w = slider_create(parent, lv_color_hex(0x800000), slider_changed_cb);
    // Set the user data to the index of the corresponding LED string
    lv_obj_set_user_data(slider_w, (void *)index);
    // Add our own callback to the slider, to take care of the animation
    lv_obj_add_event_cb(slider_w, slider_changed_local_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(slider_w, slider_changed_local_cb, LV_EVENT_PRESSING, NULL);
    // Some custom formatting
    lv_obj_align(slider_w, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_opa(slider_w, LV_OPA_TRANSP, 0);
    lv_obj_set_height(slider_w, 30);
    lv_obj_set_width(slider_w, LV_PCT(95));
    lv_obj_set_style_bg_opa(slider_w, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_pad_right(slider_w, 15, 0);
    lv_obj_set_style_pad_left(slider_w, 15, 0);
    // Register the slider with the encoder group
    lv_group_add_obj(encoder_group, slider_w);
    lv_group_focus_obj(slider_w);
    lv_group_set_editing(encoder_group, true);
    lv_group_focus_freeze(encoder_group, true);
    // Call the callback to set the initial value
    lv_slider_set_value(slider_w, 255, LV_ANIM_OFF);
    lv_obj_send_event(slider_w, LV_EVENT_KEY, NULL);
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
    // First we check if there is already a timer waiting
    if (slider_hide_timer != NULL)
    {
        lv_obj_t *timer_slider_w = (lv_obj_t *)lv_timer_get_user_data(slider_hide_timer);
        if (timer_slider_w != slider_w)
        {
            // This is a different slider, we need to hide it first and delete the timer
            animate_slider(timer_slider_w, false, true);
            lv_timer_del(slider_hide_timer);
            slider_hide_timer = NULL;
        }
        else
        {
            // This is the same slider, we can just reset the timer
            lv_timer_reset(slider_hide_timer);
        }
    }
    // Check again for the timer, it might have been deleted
    if (slider_hide_timer == NULL)
    {
        // The timer does not exist, create it
        slider_hide_timer = lv_timer_create(timer_hide_slider_cb, 3000, slider_w); // Hide after 3 seconds
        lv_timer_set_repeat_count(slider_hide_timer, 1);                           // Only run once
    }
    // If the slider is already visible, no need to animate it again
    if (lv_obj_get_style_opa(slider_w, 0) == LV_OPA_TRANSP)
    {
        animate_slider(slider_w, true, false);
    }
}

static void animate_slider(lv_obj_t *slider_w, bool show, bool fast)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, slider_w);
    lv_anim_set_exec_cb(&a, slider_anim_cb);
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

static void slider_anim_cb(void *var, int32_t v)
{
    lv_obj_t *slider_w = (lv_obj_t *)var;
    int32_t y = lv_map(v, 0, 256, -20, 40);
    int32_t opa = lv_map(v, 0, 256, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_obj_set_style_opa(slider_w, opa, 0);
    lv_obj_set_y(slider_w, y);
}

static void timer_hide_slider_cb(lv_timer_t *timer)
{
    lv_obj_t *slider_w = (lv_obj_t *)lv_timer_get_user_data(timer);
    animate_slider(slider_w, false, false);
    lv_timer_del(timer);
    slider_hide_timer = NULL; // Clear the timer reference
}
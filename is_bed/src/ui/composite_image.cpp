#include "composite_image.h"
#include <numeric>

//
// Static prototypes
//
static void composite_image_timer_cb(lv_timer_t *timer);
static void composite_image_update(const composite_image_dsc_t *dsc);

//
// Global functions
//
lv_obj_t *composite_image_create(lv_obj_t *parent, lv_event_cb_t callback, const composite_image_dsc_t *dsc)
{
    // Check that all the image formats match
    if (dsc->background_image_dsc.header.cf != LV_COLOR_FORMAT_RGB888)
    {
        LV_LOG_ERROR("Background image must be in rgb888 color format");
        return NULL; // Invalid background image
    }
    for (uint32_t i = 0; i < dsc->layer_count; i++)
    {
        if (dsc->layers[i].image_dsc.header.cf != LV_COLOR_FORMAT_L8)
        {
            LV_LOG_ERROR("Layer %lu must be in 8bit luminosity format", i);
            return NULL; // Invalid layer image
        }
    }
    lv_obj_t *canvas_w = lv_canvas_create(parent);
    lv_memset(dsc->buffer, 0, dsc->background_image_dsc.header.w * dsc->background_image_dsc.header.h * 3);
    lv_canvas_set_buffer(canvas_w, dsc->buffer, dsc->background_image_dsc.header.w, dsc->background_image_dsc.header.h, LV_COLOR_FORMAT_RGB888);

    // Use the user data of the canvas to store the composite image descriptor
    lv_obj_set_user_data(canvas_w, (void *)dsc);

    // Create a timer to update the composite image
    lv_timer_create(composite_image_timer_cb, 20, canvas_w);

    // Register the callback if it exists
    if (callback != NULL) {
        lv_obj_add_flag(canvas_w, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(canvas_w, callback, LV_EVENT_CLICKED, NULL);
    }

    return canvas_w;
}

// Update the canvas with the composite image
void composite_image_update(const composite_image_dsc_t *dsc)
{
    // First, for each layer, compute the average color of the pixels
    // TODO: For now, as a hack, we just recompute the average color of the first half of the LEDs in each string.
    //       we should instead use the color of the LEDs as actually rendered by OctoWS2811.
    lv_color_t layer_colors[dsc->layer_count];
    const uint32_t num_leds = 16;
    for (uint32_t i = 0; i < dsc->layer_count; i++)
    {
        CRGB leds[num_leds];
        led_patterns[led_zones[i].ui_pattern_index].update(
            millis() * (100 + i) / 100, // To create a phase shift between the patterns
            led_zones[i].update_period_ms,
            composed_palette(&led_palettes[led_zones[i].palette_index], led_zones[i].single_color),
            led_zones[i].single_color,
            0,
            0,
            num_leds,
            leds);
        uint32_t total_red = 0;
        uint32_t total_green = 0;
        uint32_t total_blue = 0;
        for (uint32_t j = 0; j < num_leds / 2; j++)
        {
            CRGB color = leds[j];
            total_red += color.red;
            total_green += color.green;
            total_blue += color.blue;
        }
        // Don't use the brightness scaling. That will make sure that the display always shows the pattern even if the
        // zones are currently dimmed.
        //layer_colors[i].red = total_red * led_zones[i].brightness * 2 / 255 / num_leds;
        //layer_colors[i].green = total_green * led_zones[i].brightness * 2 / 255 / num_leds;
        //layer_colors[i].blue = total_blue * led_zones[i].brightness * 2 / 255 / num_leds;
        layer_colors[i].red = total_red * 2 / num_leds;
        layer_colors[i].green = total_green * 2 / num_leds;
        layer_colors[i].blue = total_blue * 2 / num_leds;
    }

    // Then, for each pixel in the background image, composite the layers on top of it
    for (uint32_t y = 0; y < dsc->background_image_dsc.header.h; y++)
    {
        for (uint32_t x = 0; x < dsc->background_image_dsc.header.w; x++)
        {
            uint32_t layer_index = x + y * dsc->background_image_dsc.header.w;
            uint32_t canvas_index = layer_index * 3;
            uint16_t r = dsc->background_image_dsc.data[canvas_index + 2];
            uint16_t g = dsc->background_image_dsc.data[canvas_index + 1];
            uint16_t b = dsc->background_image_dsc.data[canvas_index + 0];
            for (uint32_t i = 0; i < dsc->layer_count; i++)
            {
                composite_image_layer_t *layer = &dsc->layers[i];
                // Get the pixel value from the layer image
                uint8_t layer_pixel = layer->image_dsc.data[layer_index];
                // Scale the layer color by the pixel value
                r += (layer_colors[i].red * layer_pixel) / 255;
                g += (layer_colors[i].green * layer_pixel) / 255;
                b += (layer_colors[i].blue * layer_pixel) / 255;
            }
            // Clamp the color values to the range [0, 255]
            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            // Set the pixel value on the canvas
            dsc->buffer[canvas_index + 2] = r;
            dsc->buffer[canvas_index + 1] = g;
            dsc->buffer[canvas_index + 0] = b;
        }
    }
}

static void composite_image_timer_cb(lv_timer_t *timer)
{
    // Get the user data
    lv_obj_t *canvas_w = (lv_obj_t *)lv_timer_get_user_data(timer);
    composite_image_dsc_t *dsc = (composite_image_dsc_t *)lv_obj_get_user_data(canvas_w);
    // Update the composite image    
# if LV_LOG_LEVEL <= LV_LOG_LEVEL_TRACE
    uint32_t tic = lv_tick_get();
# endif
    composite_image_update(dsc);
    lv_obj_invalidate(canvas_w);
# if LV_LOG_LEVEL <= LV_LOG_LEVEL_TRACE
    uint32_t toc = lv_tick_get();
# endif
    LV_LOG_TRACE("Composite image updated in %lu ms", toc - tic);
}
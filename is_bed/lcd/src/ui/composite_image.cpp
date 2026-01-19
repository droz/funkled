#include "composite_image.h"
#include <Arduino.h>
#include <numeric>

//
// Global variables
//

// The composite image of the bed
LV_IMAGE_DECLARE(background);
LV_IMAGE_DECLARE(center);
LV_IMAGE_DECLARE(front);
LV_IMAGE_DECLARE(headboard);
LV_IMAGE_DECLARE(cage);
composite_image_layer_t composite_layers[] = {
    {.image_dsc = cage,
     .led_color = lv_color_make(0, 0, 0)},
    {.image_dsc = center,
     .led_color = lv_color_make(0, 0, 0)},
    {.image_dsc = front,
     .led_color = lv_color_make(0, 0, 0)},
    {.image_dsc = headboard,
     .led_color = lv_color_make(0, 0, 0)}};
static DMAMEM uint8_t composite_buffer[TFT_HOR_RES * TFT_VER_RES * 3];
static composite_image_dsc_t composite_dsc = {
    .buffer = composite_buffer,
    .background_image_dsc = background,
    .layers = composite_layers,
    .layer_count = sizeof(composite_layers) / sizeof(composite_layers[0])};

//
// Static prototypes
//
static void composite_image_timer_cb(lv_timer_t *timer);
static void composite_image_update(const composite_image_dsc_t *dsc);

//
// Global functions
//
lv_obj_t *composite_image_create(lv_obj_t *parent, lv_event_cb_t callback)
{
    // Check that all the image formats match
    if (composite_dsc.background_image_dsc.header.cf != LV_COLOR_FORMAT_RGB888)
    {
        LV_LOG_ERROR("Background image must be in rgb888 color format");
        return NULL; // Invalid background image
    }
    for (uint32_t i = 0; i < composite_dsc.layer_count; i++)
    {
        if (composite_dsc.layers[i].image_dsc.header.cf != LV_COLOR_FORMAT_L8)
        {
            LV_LOG_ERROR("Layer %lu must be in 8bit luminosity format", i);
            return NULL; // Invalid layer image
        }
    }
    lv_obj_t *canvas_w = lv_canvas_create(parent);
    lv_memset(composite_dsc.buffer, 0, composite_dsc.background_image_dsc.header.w * composite_dsc.background_image_dsc.header.h * 3);
    lv_canvas_set_buffer(canvas_w, composite_dsc.buffer, composite_dsc.background_image_dsc.header.w, composite_dsc.background_image_dsc.header.h, LV_COLOR_FORMAT_RGB888);

    // Use the user data of the canvas to store the composite image descriptor
    lv_obj_set_user_data(canvas_w, (void *)&composite_dsc);
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
    // For each pixel in the background image, composite the layers on top of it
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
                r += (layer->led_color.red * layer_pixel) / 255;
                g += (layer->led_color.green * layer_pixel) / 255;
                b += (layer->led_color.blue * layer_pixel) / 255;
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
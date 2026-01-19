#ifndef LV_COMPOSITE_IMAGE_H
#define LV_COMPOSITE_IMAGE_H

#include "../lvgl.h"

// This describes a layer for a composite image
typedef struct
{
    // The image descriptor for the layer. Should be an 8 bit single channel image.
    lv_image_dsc_t image_dsc;
    // The led color associated with this layer.
    lv_color_t led_color;
} composite_image_layer_t;

// This struct stores the data for a composite image
typedef struct
{
    // The buffer to use for the composite image
    uint8_t *buffer;
    // The background image on which the composite image will be drawn
    lv_image_dsc_t background_image_dsc;
    // An array of layers to compose on top of the background image
    composite_image_layer_t *layers;
    // The number of layers in the composite image
    uint32_t layer_count;
} composite_image_dsc_t;

// The layer for each zone
extern composite_image_layer_t composite_layers[];

// Create a composite image widget
lv_obj_t *composite_image_create(lv_obj_t *parent, lv_event_cb_t callback);

#endif /*LV_COMPOSITE_IMAGE_H*/
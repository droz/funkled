#ifndef LED_PALETTE_H
#define LED_PALETTE_H

#include <FastLED.h>

// A small struct to describe a palette
typedef struct
{
    // The name of the palette
    const char *name;
    // The description of the palette
    const char *desc;
    // Should this palette use the solid color ?
    // If it does, it will multiply the intensity of
    //  the palette with the current color.
    bool compose_colors;
    // The palette itself
    CRGBPalette16 palette;
} led_palette_t;

// All the palettes available
extern led_palette_t led_palettes[];

// Number of palettes available
extern uint32_t num_led_palettes();

// Compute the final palette, potentially doing the composition with
//  the solid color. The pointer returned is valid until the next call.
const CRGBPalette16 *composed_palette(const led_palette_t *palette, CRGB color);

#endif // LED_PALETTE_H

#ifndef LED_PATTERN_H
#define LED_PATTERN_H

#include <FastLED.h>

// An LED pattern function
typedef void (*led_pattern_func_t)(
    // The current time, in ms
    uint32_t time_ms,
    // The period to use for the pattern, in ms
    uint32_t period_ms,
    // The palette to use to render the LEDs. Can be
    // potentially ignored by the pattern if it wants to use a single color
    const CRGBPalette16 *palette,
    // The single color to use for the pattern, If it wants a single color
    CRGB single_color,
    uint32_t string_index,
    uint32_t segment_index,
    // The number of LEDs in the string
    uint32_t num_leds,
    // The array of LEDs to update
    CRGB *leds);

// This struct is used to describe an LED pattern, which will drive a string of LEDs
typedef struct
{
    // The name of the pattern
    const char *name;
    // The description of the pattern
    const char *desc;
    // The function that will be called to update the LEDs
    led_pattern_func_t update;
} led_pattern_t;

// All the available LED patterns
extern led_pattern_t led_patterns[];

// Number of patterns available
extern uint32_t num_led_patterns();


#endif // LED_PATTERN_H

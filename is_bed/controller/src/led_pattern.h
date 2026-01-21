#ifndef LED_PATTERN_H
#define LED_PATTERN_H

#include <FastLED.h>
#include <Arduino.h>
#include <is_bed_protocol.h>
#include "cached_pattern.h"

// Fill the patterns array with the cached patterns
extern void add_cached_patterns();

// Add a strobe pattern to the patterns array
extern void add_strobe_pattern();

// Add a static pattern to the patterns array
extern void add_static_pattern();

// Parameters for the pattern function
typedef struct {
    // The current time, in ms
    uint32_t time_ms;
    // The period to use for the pattern, in ms
    uint32_t period_ms;
    // The palette to use to render the LEDs, if the pattern wants to use a palette
    const CRGBPalette16 *palette;
    // The single color to use, if the pattern wants a single color
    CRGB single_color;
    // The cached pattern to use, if the pattern wants to use a cached pattern
    cached_pattern_t *cached_pattern;
    // The index of the LED string being updated
    uint32_t string_index;
    // The index of the segment being updated
    uint32_t segment_index;
    // The number of LEDs in the string
    uint32_t num_leds;
    // Is this for display only (not the actual LED string)
    bool display_only;
    // The array of LEDs to update
    CRGB *leds;
} led_pattern_params_t;

// An LED pattern function
typedef void (*led_pattern_func_t)(led_pattern_params_t params);

// This struct is used to describe an LED pattern, which will drive a string of LEDs
typedef struct
{
    // The name of the pattern
    String name;
    // The cached pattern to use, if the pattern wants to use a cached pattern
    cached_pattern_t *cached_pattern;
    // The function that will be called to update the LEDs
    led_pattern_func_t update;
} led_pattern_t;

// The maximum number of LED patterns supported
#define MAX_LED_PATTERNS 64
// A table of all the LED patterns available
extern led_pattern_t led_patterns[];
extern uint32_t num_led_patterns;

// Return the type of a pattern
extern is_bed_pattern_type_t pattern_type(led_pattern_t* pattern);

#endif // LED_PATTERN_H

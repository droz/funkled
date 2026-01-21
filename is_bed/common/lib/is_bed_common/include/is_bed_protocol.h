#ifndef IS_BED_PROTOCOL_H
#define IS_BED_PROTOCOL_H

#include "zones.h"
#include <stdint.h>

enum is_bed_pattern_type_t : uint8_t {
    IS_BED_PATTERN_UNKNOWN = 0,
    IS_BED_PATTERN_STATIC = 1,
    IS_BED_PATTERN_STROBE = 2,
    IS_BED_PATTERN_ROTATE = 3,
    IS_BED_PATTERN_FADE = 4,
    IS_BED_PATTERN_BLINK = 5,
    IS_BED_PATTERN_CACHED = 6,
};

struct [[gnu::packed]] color_rgb_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Struct used for controller -> LCD communication
struct [[gnu::packed]] is_bed_controller_to_lcd_t {
    // The controller is going to enumerate patterns by sending one at a time.
    // This is the index of the pattern being sent.
    uint8_t pattern_index;
    // The name of that pattern.
    char pattern_name[16];
    // The type of that pattern.
    is_bed_pattern_type_t pattern_type;
    // The colors that will be displayed on the screen composite image for each zone
    color_rgb_t zone_color[NUM_ZONES];
};

// Struct used for LCD -> controller communication
struct [[gnu::packed]] is_bed_lcd_to_controller_t {
    // The index of the currently selected pattern
    uint8_t selected_pattern_index;
    // the index of the currently displayed pattern
    uint8_t displayed_pattern_index;
    // The brightnesses of each zone
    uint8_t zone_brightness[NUM_ZONES];
    // The currently selected color (for static pattern for example)
    color_rgb_t selected_color;
    // The current frequency (for strobe pattern for example)
    // The value in hz is computed as frequency_hz = frequency / 10
    uint8_t frequency;
};

#endif // IS_BED_PROTOCOL_H

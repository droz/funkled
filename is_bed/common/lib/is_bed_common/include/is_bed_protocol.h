#ifndef IS_BED_PROTOCOL_H
#define IS_BED_PROTOCOL_H

#include "zones.h"
#include <stdint.h>

struct [[gnu::packed]] color_rgb_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Struct used for controller -> LCD communication
struct [[gnu::packed]] is_bed_controller_to_lcd_t {
    // The name of one pattern. Will be added to the currently known patterns.
    // The controller can enumerate all the patterns by rotating through them.
    char pattern_name[16];
    // The index of the pattern above
    uint8_t pattern_index;
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
};

#endif // IS_BED_PROTOCOL_H

#include "led_pattern.h"
#include "led_array.h"
#include <Arduino.h>

// All the available LED patterns
led_pattern_t led_patterns[MAX_LED_PATTERNS];
uint32_t num_led_patterns = 0;

// The time at which the last strobe happened, both for real and on the LCD
static uint32_t last_strobe_time_ms = 0;
static uint32_t last_display_strobe_time_ms = 0;

// Set all the LEDs to the palette color, regardless of time
void static_pattern(led_pattern_params_t p) {
    for (uint32_t i = 0; i < p.num_leds; i++)
    {
        p.leds[i] = p.single_color;
    }
}

// Rotate all the LEDs on a palette
void rotate_pattern(led_pattern_params_t p) {
    uint32_t offset = 255 - p.time_ms * 255 / p.period_ms;
    for (uint32_t i = 0; i < p.num_leds; i++)
    {
        uint8_t palette_index = i * 255 / p.num_leds + offset;
        p.leds[i] = ColorFromPalette(*p.palette, palette_index);
    }
}

// Fade all the LEDs on a palette
void fade_pattern(led_pattern_params_t p) {
    uint32_t fade_u32 = (p.time_ms * 512 / p.period_ms) % 512;
    if (fade_u32 >= 256)
    {
        fade_u32 = 511 - fade_u32;
    }
    uint8_t fade_u8 = fade_u32;
    for (uint32_t i = 0; i < p.num_leds; i++)
    {
        uint8_t palette_index = i * 255 / p.num_leds;
        p.leds[i] = ColorFromPalette(*p.palette, palette_index, fade_u8);
    }
}

// Blink all the LEDs on a palette
void blink_pattern(led_pattern_params_t p) {
    bool on = p.time_ms % p.period_ms < p.period_ms / 2;
    for (uint32_t i = 0; i < p.num_leds; i++)
    {
        uint8_t palette_index = i * 255 / p.num_leds;
        if (on) {
            p.leds[i] = ColorFromPalette(*p.palette, palette_index);
        } else {
            p.leds[i] = CRGB::Black;
        }
    }
}

// Strobe all the LEDs on a palette
void strobe_pattern(led_pattern_params_t p) {
    // Figure out if enough time elapsed since last strobe
    uint32_t& last_time_ms = p.display_only ? last_display_strobe_time_ms : last_strobe_time_ms;
    uint32_t time_since_last_strobe_ms = p.time_ms - last_time_ms;
    bool on = false;
    // If this is far enough from the last strobe, turn on
    if (time_since_last_strobe_ms >= p.period_ms) {
        last_time_ms = p.time_ms;
        on = true;
    }
    // If this is the same time as the last strobe, keep on
    if (time_since_last_strobe_ms == 0) {
        on = true;
    }
    // Set the LEDs
    for (uint32_t i = 0; i < p.num_leds; i++) {
        if (on) {
            p.leds[i] = p.single_color;
        } else {
            p.leds[i] = CRGB::Black;
        }
    }
}

void cached_pattern(led_pattern_params_t p) {
    led_string_t *led_string = &led_strings[p.string_index];
    cached_pattern_t& pattern = *(p.cached_pattern);
    const uint32_t period_ms = pattern.header.animation_period_s * 1000;
    const uint32_t step = (p.time_ms * p.cached_pattern->header.animation_steps / period_ms) % pattern.header.animation_steps;

    // Seek to offset where to start reading LED data
    uint64_t pos = sizeof(cached_pattern_header_t);
    pos += pattern.header.num_pixels * step * sizeof(CRGB);
    for (uint32_t i = 0; i < p.string_index; i++) {
        pos += led_strings[i].num_leds * sizeof(CRGB);
    }
    for (uint32_t i = 0; i < p.segment_index; i++) {
        pos += led_string->segments[i].num_leds * sizeof(CRGB);
    }
    pattern.file.seek(pos);

    // Read LED data for segment.
    for (uint32_t i = 0; i < p.num_leds; i++)
    {
        pattern.file.read(&p.leds[i], sizeof(CRGB));
    }
}

// Add a cached patterns to the patterns array
void add_cached_patterns() {
    for (uint32_t i = 0; i < num_cached_patterns; i++) {
        if (num_led_patterns < MAX_LED_PATTERNS) {
            // We use the filename to name the pattern, but we have to do some sanitizing.
            // Remove the file extension
            String name = cached_patterns[i].filepath;
            name = name.substring(name.lastIndexOf('/') + 1);
            int dot_index = name.lastIndexOf('.');
            if (dot_index != -1) {
                name.remove(dot_index);
            }
            // Replace the underscores with spaces and adjust the case
            name.replace('_', ' ');
            name.toLowerCase();
            // Capitalize the first letter of each word
            for (unsigned int j = 0; j < name.length(); j++) {
                if (j == 0 || name.charAt(j - 1) == ' ') {
                    name.setCharAt(j, toupper(name.charAt(j)));
                }
            }
            // Now we can fill the struct
            led_patterns[num_led_patterns].name = name;
            led_patterns[num_led_patterns].cached_pattern = &cached_patterns[i];
            led_patterns[num_led_patterns].update = cached_pattern;
            num_led_patterns++;
        }
    }
}

// Add a strobe pattern to the patterns array
void add_strobe_pattern() {
    if (num_led_patterns < MAX_LED_PATTERNS) {
        led_patterns[num_led_patterns].name = "Strobe";
        led_patterns[num_led_patterns].cached_pattern = nullptr;
        led_patterns[num_led_patterns].update = strobe_pattern;
        num_led_patterns++;
    }
}

// Add a static pattern to the patterns array
void add_static_pattern() {
    if (num_led_patterns < MAX_LED_PATTERNS) {
        led_patterns[num_led_patterns].name = "Static";
        led_patterns[num_led_patterns].cached_pattern = nullptr;
        led_patterns[num_led_patterns].update = static_pattern;
        num_led_patterns++;
    }
}

// Return the type of a pattern
is_bed_pattern_type_t pattern_type(led_pattern_t* pattern) {
    if (pattern->update == cached_pattern) {
        return IS_BED_PATTERN_CACHED;
    }
    if (pattern->update == strobe_pattern) {
        return IS_BED_PATTERN_STROBE;
    }
    if (pattern->update == static_pattern) {
        return IS_BED_PATTERN_STATIC;
    }
    if (pattern->update == rotate_pattern) {
        return IS_BED_PATTERN_ROTATE;
    }
    if (pattern->update == fade_pattern) {
        return IS_BED_PATTERN_FADE;
    }
    if (pattern->update == blink_pattern) {
        return IS_BED_PATTERN_BLINK;
    }
    return IS_BED_PATTERN_UNKNOWN;
}
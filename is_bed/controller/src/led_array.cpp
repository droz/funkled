#include "led_array.h"
#include <zones.h>
#include <EEPROM.h>

uint32_t current_channel = 0;

static const uint16_t magic = 0xCAFE;

static const uint8_t default_color_ordering = WS2811_GRB;
static const uint32_t default_pattern_index = 0;
static const CRGB default_single_color = CRGB::Red;
static const int32_t default_palette_index = 0;
static const int32_t default_update_period_ms = 3000;
static const uint8_t default_brightness = 255;

CRGB leds_crgb[max_leds_per_channel];

led_zone_t led_zones[] = {
    {
        .name = "Cage",
        .led_pattern_index = default_pattern_index,
        .ui_pattern_index = default_pattern_index,
        .single_color = default_single_color,
        .palette_index = default_palette_index,
        .update_period_ms = default_update_period_ms,
        .max_brightness = 128,
        .brightness = default_brightness,
    },
    {
        .name = "Center",
        .led_pattern_index = default_pattern_index,
        .ui_pattern_index = default_pattern_index,
        .single_color = default_single_color,
        .palette_index = default_palette_index,
        .update_period_ms = default_update_period_ms,
        .max_brightness = 170,
        .brightness = default_brightness,
    },
    {
        .name = "Front",
        .led_pattern_index = default_pattern_index,
        .ui_pattern_index = default_pattern_index,
        .single_color = default_single_color,
        .palette_index = default_palette_index,
        .update_period_ms = default_update_period_ms,
        .max_brightness = 170,
        .brightness = default_brightness,
    },
    {
        .name = "Headboard",
        .led_pattern_index = default_pattern_index,
        .ui_pattern_index = default_pattern_index,
        .single_color = default_single_color,
        .palette_index = default_palette_index,
        .update_period_ms = default_update_period_ms,
        .max_brightness = 255,
        .brightness = default_brightness,
    },
};

led_segment_t post_front_left_segments[] = {
    {
        .name = "Cage Left",
        .num_leds = 12,
        .string_offset = 0,
        .zone = ZONE_CAGE,
    },
    {
        .name = "Cage Right",
        .num_leds = 12,
        .string_offset = 12,
        .zone = ZONE_CAGE,
    },
    {
        .name = "Frame Right",
        .num_leds = 21,
        .string_offset = 24,
        .zone = ZONE_CENTER,
    },
    {
        .name = "Frame Left",
        .num_leds = 21,
        .string_offset = 45,
        .zone = ZONE_CENTER,
    },
};

led_segment_t post_front_right_segments[] = {
    {
        .name = "Cage Left",
        .num_leds = 12,
        .string_offset = 0,
        .zone = ZONE_CAGE,
    },
    {
        .name = "Cage Right",
        .num_leds = 12,
        .string_offset = 12,
        .zone = ZONE_CAGE,
    },
    {
        .name = "Frame Right",
        .num_leds = 21,
        .string_offset = 24,
        .zone = ZONE_CENTER,
    },
    {
        .name = "Frame Left",
        .num_leds = 21,
        .string_offset = 45,
        .zone = ZONE_CENTER,
    },
};

led_segment_t post_rear_left_segments[] = {
    {
        .name = "Cage Left",
        .num_leds = 12,
        .string_offset = 0,
        .zone = ZONE_CAGE,
    },
    {
        .name = "Cage Right",
        .num_leds = 12,
        .string_offset = 12,
        .zone = ZONE_CAGE,
    },
    {
        .name = "Back Right",
        .num_leds = 21,
        .string_offset = 24,
        .zone = ZONE_FRONT,
    },
    {
        .name = "Back Left",
        .num_leds = 21,
        .string_offset = 45,
        .zone = ZONE_FRONT,
    },
    {
        .name = "Frame Right",
        .num_leds = 21,
        .string_offset = 66,
        .zone = ZONE_CENTER,
    },
    {
        .name = "Frame Left",
        .num_leds = 21,
        .string_offset = 87,
        .zone = ZONE_CENTER,
    },
};

led_segment_t post_rear_right_segments[] = {
    {
        .name = "Cage Left",
        .num_leds = 12,
        .string_offset = 0,
        .zone = ZONE_CAGE,
    },
    {
        .name = "Cage Right",
        .num_leds = 12,
        .string_offset = 12,
        .zone = ZONE_CAGE,
    },
    {
        .name = "Back Right",
        .num_leds = 21,
        .string_offset = 24,
        .zone = ZONE_FRONT,
    },
    {
        .name = "Back Left",
        .num_leds = 21,
        .string_offset = 45,
        .zone = ZONE_FRONT,
    },
    {
        .name = "Frame Right",
        .num_leds = 21,
        .string_offset = 66,
        .zone = ZONE_CENTER,
    },
    {
        .name = "Frame Left",
        .num_leds = 21,
        .string_offset = 87,
        .zone = ZONE_CENTER,
    },
};

led_segment_t headboard_segments[] = {
    {
        .name = "Top left",
        .num_leds = 10,
        .string_offset = 0,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Cross Top Left",
        .num_leds = 8,
        .string_offset = 10,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Top Middle",
        .num_leds = 8,
        .string_offset = 18,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Cross Top Right",
        .num_leds = 8,
        .string_offset = 26,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Top Right",
        .num_leds = 10,
        .string_offset = 34,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Right",
        .num_leds = 14,
        .string_offset = 44,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Bottom Right",
        .num_leds = 10,
        .string_offset = 58,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Cross Bottom Right",
        .num_leds = 8,
        .string_offset = 68,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Bottom Middle",
        .num_leds = 8,
        .string_offset = 76,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Cross Bottom Left",
        .num_leds = 8,
        .string_offset = 84,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Bottom left",
        .num_leds = 10,
        .string_offset = 92,
        .zone = ZONE_HEADBOARD,
    },
    {
        .name = "Left",
        .num_leds = 14,
        .string_offset = 102,
        .zone = ZONE_HEADBOARD,
    },
};


led_string_t led_strings[] = {
    {
        .name = "Post Front Left",
        .num_leds = leds_in_string(post_front_left_segments),
        .num_segments = segments_in_string(post_front_left_segments),
        .segments = post_front_left_segments,
        .channel = 6,
    },
    {
        .name = "Post Front Right",
        .num_leds = leds_in_string(post_front_right_segments),
        .num_segments = segments_in_string(post_front_right_segments),
        .segments = post_front_right_segments,
        .channel = 4,
    },
    {
        .name = "Post Rear Left",
        .num_leds = leds_in_string(post_rear_left_segments),
        .num_segments = segments_in_string(post_rear_left_segments),
        .segments = post_rear_left_segments,
        .channel = 7,
    },
    {
        .name = "Post Rear Right",
        .num_leds = leds_in_string(post_rear_right_segments),
        .num_segments = segments_in_string(post_rear_right_segments),
        .segments = post_rear_right_segments,
        .channel = 3,
    },
    {
        .name = "Headboard",
        .num_segments = sizeof(headboard_segments) / sizeof(headboard_segments[0]),
        .segments = headboard_segments,
        .channel = 5,
    },
};

void led_array_init() {
}

void led_array_save() {
    // Write the magic
    EEPROM.write(0, magic & 0xFF);
    EEPROM.write(1, magic >> 8);
    // Write the size
    uint16_t size = sizeof(led_strings);
    EEPROM.write(2, size & 0xFF);
    EEPROM.write(3, size >> 8);
    // Write the data
    EEPROM.put(4, led_strings);
}

void led_array_load() {
    // Read the magic
    uint16_t magic_read = EEPROM.read(0) | (EEPROM.read(1) << 8);
    if (magic_read != magic) {
        return;
    }
    // Read the size
    uint16_t size = EEPROM.read(2) | (EEPROM.read(3) << 8);
    if (size != sizeof(led_strings)) {
        return;
    }
    // Read the data
    EEPROM.get(4, led_strings);
}
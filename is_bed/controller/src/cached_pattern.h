#ifndef CACHED_PATTERN_H
#define CACHED_PATTERN_H

#include <OctoWS2811.h>
#include <FastLED.h>
#include <Arduino.h>
#include <SD.h>


// List the name of the patterns available on the SD card
void load_cached_patterns();

// Struct to describe a cached pattern resource.
struct [[gnu::packed]] cached_pattern_header_t {
    // Two bytes indicating pattern format.
    uint16_t magic;
    // The color ordering used by the pattern
    uint8_t color_ordering;
    // The number of pixels
    uint16_t num_pixels;
    // The number of animation steps stored.
    uint16_t animation_steps;
    // The nominal animation period in seconds.
    uint16_t animation_period_s;
};

typedef struct {
    cached_pattern_header_t header;
    // Size of the image in bytes
    uint32_t data_size;
    String filepath;
    File file;
} cached_pattern_t;

// The cached patterns available on the system
#define MAX_CACHED_PATTERN_NUMBER 64
extern cached_pattern_t cached_patterns[MAX_CACHED_PATTERN_NUMBER];
extern uint32_t num_cached_patterns;

#endif // CACHED_PATTERN_H
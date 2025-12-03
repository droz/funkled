/**
 * @file cached_pattern.h
 *
 */

#ifndef CACHED_PATTERN_H
#define CACHED_PATTERN_H

#include <OctoWS2811.h>
#include <FastLED.h>
#include <SD.h>

// Struct to describe a cached pattern resource.
struct __attribute__((packed)) cached_pattern_header_t
{
    // Two bytes indicating pattern format.
    uint16_t magic;
    // The color ordering used by the pattern
    uint8_t color_ordering;
    // The number of pixels
    uint16_t num_pixels;
    // The number of animation steps stored.
    uint16_t animation_steps;
};

typedef struct
{
    cached_pattern_header_t header;
    // Size of the image in bytes
    uint32_t data_size;
    const char *filepath;
    File file;
} cached_pattern_t;

// Macro to load files
#define CACHED_PATTERN_LOAD(p)                                                    \
    p.file = SD.open(p.filepath);                                                 \
    if (p.file)                                                                   \
    {                                                                             \
        p.file.read(&p.header, sizeof(p.header));                                 \
        Serial.printf("magic: %X\n", p.header.magic);                             \
        Serial.printf("color_ordering: %d\n", p.header.color_ordering);           \
        Serial.printf("num_pixels: %d\n", p.header.num_pixels);                   \
        Serial.printf("animation_steps: %d\n", p.header.animation_steps);         \
    }                                                                             \
    else                                                                          \
    {                                                                             \
        Serial.println("Failed to open file from SD card.");                      \
    }

// Macro to validate data consistency
#define CHECK_CACHED_PATTERN(p)                                     \
    constexpr uint32_t num_leds =                                   \
        total_pattern_leds(p.num_segments, p.num_leds_per_segment); \
    constexpr uint32_t num_pixels = p.animation_steps * num_leds;   \
    static_assert(num_pixels == p.num_pixels);

// Static function to count total number of LEDs addressed by the pattern.
constexpr uint32_t total_pattern_leds(const uint32_t num_segments,
                                      const uint32_t *num_leds_per_segment,
                                      std::size_t i = 0U)
{
    return i < num_segments ? (num_leds_per_segment[i] +
                               total_pattern_leds(num_segments,
                                                  num_leds_per_segment,
                                                  i + 1U))
                            : 0;
}

extern cached_pattern_t fire;
extern cached_pattern_t rainbow;

#endif /*CACHED_PATTERN_H*/

#include "led_array.h"
#include "cached_patterns/cached_pattern.h"
#include <Arduino.h>
#include <OctoWS2811.h>
#include <Wire.h>
#include <SD.h>
#include <MTP_Teensy.h>

const int SD_ChipSelect = BUILTIN_SDCARD;

// Status LED
#define STATUS_RED 33
#define STATUS_GREEN 32
#define STATUS_TEENSY_BUILTIN 13
uint8_t led_beat_counter = 0;

// LEDs
#define LED_REFRESH_RATE_HZ 20
const uint8_t pin_list[] = {28, 24, 15, 7, 5, 3, 2, 1, 25, 14, 8, 6, 4, 22, 23, 0};
const uint32_t bytes_per_led = 3;
DMAMEM uint8_t display_memory[max_leds * bytes_per_led];
uint8_t drawing_memory[max_leds * bytes_per_led];
// OctoWS2811 can do its own RGB reordering, but it may be different for each strip, so we do it ourselves.
const uint8_t config = WS2811_RGB | WS2811_800kHz;
OctoWS2811 leds(max_leds_per_channel, display_memory, drawing_memory, config, num_led_channels, pin_list);

// Function prototypes
static void led_refresh();

// Last time the LEDs were refreshed
unsigned long last_tick = 0;

#define HEARTBEAT_PERIOD_MS 1700

//
// The main setup function
//
void setup()
{
    // Status LED
    pinMode(STATUS_RED, OUTPUT);
    pinMode(STATUS_GREEN, OUTPUT);
    digitalWrite(STATUS_RED, HIGH);
    digitalWrite(STATUS_GREEN, HIGH);

    // Serial port
    Serial.begin(115200);

    // Initialize the led array descriptors
    led_array_init();

    // Start the LEDs
    leds.begin();

    // We are done
    Serial.println("Setup done");
    digitalWrite(STATUS_GREEN, HIGH);
    digitalWrite(STATUS_RED, LOW);

    // Add the SD Card
    if (SD.begin(SD_ChipSelect))
    {
        Serial.println("SD Card initialized");
        CACHED_PATTERN_LOAD(abstract_gradient);
        CACHED_PATTERN_LOAD(blue_light_rays);
        CACHED_PATTERN_LOAD(color_roll);
        CACHED_PATTERN_LOAD(fire);
        CACHED_PATTERN_LOAD(flash);
        CACHED_PATTERN_LOAD(matrix);
        CACHED_PATTERN_LOAD(rainbow);
        CACHED_PATTERN_LOAD(space_warp);

        // Start MTP
        MTP.begin();
        MTP.addFilesystem(SD, "SD_Card");
    }
    
}

void loop() {
    unsigned long now = millis();
    constexpr unsigned long PERIOD_MS = 1000 / LED_REFRESH_RATE_HZ;
    if (now - last_tick >= PERIOD_MS) {
        last_tick = now;
        // Refresh the LEDs
        led_refresh();
        // Update MTP
        MTP.loop();
        // Heartbeat on the serial port
        Serial.print(".");
        // Heartbeat LED
        led_beat_counter++;
        if (led_beat_counter == LED_REFRESH_RATE_HZ) {
            digitalWrite(STATUS_GREEN, LOW);
            digitalWrite(STATUS_TEENSY_BUILTIN, LOW);
        }
        if (led_beat_counter == LED_REFRESH_RATE_HZ * 2) {
            digitalWrite(STATUS_GREEN, HIGH);
            digitalWrite(STATUS_TEENSY_BUILTIN, HIGH);
            led_beat_counter = 0;
        }
    }
}

// Refresh the LEDs
static void led_refresh()
{
    uint32_t now = millis();
    for (uint32_t i = 0; i < num_strings; i++)
    {
        led_string_t *led_string = &led_strings[i];
        for (uint32_t j = 0; j < led_string->num_segments; j++)
        {
            led_segment_t *segment = &led_string->segments[j];
            led_zone_t *zone = &led_zones[segment->zone];
            led_patterns[zone->led_pattern_index]
                .update(
                    now,
                    zone->update_period_ms,
                    composed_palette(&led_palettes[zone->palette_index], zone->single_color),
                    zone->single_color,
                    i,
                    j,
                    segment->num_leds,
                    leds_crgb + segment->string_offset);
            for (uint32_t k = segment->string_offset; k < segment->string_offset + segment->num_leds; k++)
            {
                uint32_t color_u32 = 0x000000;
                leds_crgb[k].nscale8(zone->brightness);
                // The green LEDs are stronger than the other colors. Dim them a little bit to help with color balance.
                leds_crgb[k].g = scale8(leds_crgb[k].g, 200);
                switch (zone->color_ordering)
                {
                case WS2811_RGB:
                    color_u32 = leds_crgb[k].r << 16 | leds_crgb[k].g << 8 | leds_crgb[k].b;
                    break;
                case WS2811_RBG:
                    color_u32 = leds_crgb[k].r << 16 | leds_crgb[k].b << 8 | leds_crgb[k].g;
                    break;
                case WS2811_GRB:
                    color_u32 = leds_crgb[k].g << 16 | leds_crgb[k].r << 8 | leds_crgb[k].b;
                    break;
                case WS2811_GBR:
                    color_u32 = leds_crgb[k].g << 16 | leds_crgb[k].b << 8 | leds_crgb[k].r;
                    break;
                case WS2811_BRG:
                    color_u32 = leds_crgb[k].b << 16 | leds_crgb[k].r << 8 | leds_crgb[k].g;
                    break;
                case WS2811_BGR:
                    color_u32 = leds_crgb[k].b << 16 | leds_crgb[k].g << 8 | leds_crgb[k].r;
                    break;
                default:
                    color_u32 = 0x000000;
                    break;
                }
 
                
                leds.setPixel(led_string->channel * max_leds_per_channel + k, color_u32);
            }
        }
    }
    leds.show();
    // Send a dot character on the serial port
    Serial.print(".");
    Serial.flush();
}

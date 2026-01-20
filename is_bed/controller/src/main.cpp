#include "led_array.h"
#include "led_pattern.h"
#include "cached_pattern.h"
#include <Arduino.h>
#include <OctoWS2811.h>
#include <Wire.h>
#include <SD.h>
#include <MTP_Teensy.h>
#include <USBHost_t36.h>
#include <SerialTransfer.h>
#include <is_bed_protocol.h>

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

// USB host. To talk to the screen controller
USBHost usb_host;
// USB serial port over USB host
USBSerial_BigBuffer usb_host_serial(usb_host);  // BigBuffer helps avoid drops if you burst data
SerialTransfer lcd_transfer;
// Data transfer structs
is_bed_controller_to_lcd_t to_lcd_msg;
is_bed_lcd_to_controller_t from_lcd_msg;

// Function prototypes
static void led_refresh();
static void compute_display_colors(color_rgb_t zone_color[]);

// Last time the LEDs were refreshed
unsigned long last_tick = 0;

// Last pattern that we did output to the LCD
uint8_t to_lcd_pattern_index = 0;

//
// The main setup function
//
void setup()
{
    // Status LED
    pinMode(STATUS_RED, OUTPUT);
    pinMode(STATUS_GREEN, OUTPUT);
    pinMode(STATUS_TEENSY_BUILTIN, OUTPUT);
    digitalWrite(STATUS_RED, HIGH);
    digitalWrite(STATUS_GREEN, HIGH);
    digitalWrite(STATUS_TEENSY_BUILTIN, HIGH);

    // Serial port
    Serial.begin(115200);

    // Initialize the led array descriptors
    led_array_init();

    // Start the LEDs
    leds.begin();

    // Add the SD Card
    if (SD.begin(SD_ChipSelect))
    {
        Serial.println("SD Card initialized");
        // Load the cached patterns from the SD card
        load_cached_patterns();
        add_cached_patterns();

        // Start MTP
        MTP.begin();
        MTP.addFilesystem(SD, "SD_Card");
    }

    // USB host
    usb_host.begin();
    lcd_transfer.begin(usb_host_serial);

    // We are done. Don't change the LEDs yet, we will do that when 
    // we detect connection to the LCD.
    Serial.println("Setup done");
}

void loop() {
    unsigned long now = millis();
    constexpr unsigned long PERIOD_MS = 1000 / LED_REFRESH_RATE_HZ;
    if (now - last_tick >= PERIOD_MS) {
        last_tick = now;
        // Refresh the LEDs
        led_refresh();
        // Send data to the LCD
        if (usb_host_serial) {
            // Connected.
            digitalWrite(STATUS_RED, LOW);
            // Prepare data
            to_lcd_msg.pattern_index = to_lcd_pattern_index;
            led_patterns[to_lcd_pattern_index].name.toCharArray(to_lcd_msg.pattern_name, sizeof(to_lcd_msg.pattern_name));
            to_lcd_pattern_index = (to_lcd_pattern_index + 1) % num_led_patterns;
            compute_display_colors(to_lcd_msg.zone_color);
            // Send data
            uint16_t send_size = lcd_transfer.txObj(to_lcd_msg, 0, sizeof(to_lcd_msg));
            lcd_transfer.sendData(send_size);
        }

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

    // Listen for messages from the LCD
    while (lcd_transfer.available()) {
        lcd_transfer.rxObj(from_lcd_msg);
        // Change the selected pattern
        for (uint32_t i = 0; i < NUM_ZONES; i++) {
            if (from_lcd_msg.selected_pattern_index < num_led_patterns) {
                led_zones[i].led_pattern_index = from_lcd_msg.selected_pattern_index;
            }
        }
        // Change the displayed pattern
        for (uint32_t i = 0; i < NUM_ZONES; i++) {
            if (from_lcd_msg.displayed_pattern_index < num_led_patterns) {
                led_zones[i].ui_pattern_index = from_lcd_msg.displayed_pattern_index;
            }
        }
        // Update the brightness
        for (uint32_t i = 0; i < NUM_ZONES; i++) {
            led_zones[i].brightness = from_lcd_msg.zone_brightness[i];
        }
    }

    // Update MTP
    MTP.loop();

    // USB host
    usb_host.Task();
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
            led_pattern_params_t params;
            led_pattern_t& pattern = led_patterns[zone->led_pattern_index];
            params.time_ms = now;
            params.period_ms = zone->update_period_ms;
            params.palette = composed_palette(&led_palettes[zone->palette_index], zone->single_color);
            params.single_color = zone->single_color;
            params.cached_pattern = pattern.cached_pattern;
            params.string_index = i;
            params.segment_index = j;
            params.num_leds = segment->num_leds;
            params.leds = leds_crgb + segment->string_offset;
            pattern.update(params);
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
}

// Compute the colors to display for each zone on the LCD for the current selected pattern
static void compute_display_colors(color_rgb_t zone_color[]) {
    // Compute the average color of the LEDs in each string.
    const uint32_t num_leds = 16;
    for (uint32_t i = 0; i < NUM_ZONES; i++)
    {
        CRGB leds[num_leds];
        led_pattern_params_t params;
        params.time_ms = millis() * (100 + i) / 100; // To create a phase shift between the patterns
        params.period_ms = led_zones[i].update_period_ms;
        params.palette = composed_palette(&led_palettes[led_zones[i].palette_index], led_zones[i].single_color);
        params.single_color = led_zones[i].single_color;
        params.cached_pattern = led_patterns[led_zones[i].ui_pattern_index].cached_pattern;
        params.string_index = 0;
        params.segment_index = 0;
        params.num_leds = num_leds;
        params.leds = leds;
        led_patterns[led_zones[i].ui_pattern_index].update(params);
        uint32_t total_red = 0;
        uint32_t total_green = 0;
        uint32_t total_blue = 0;
        for (uint32_t j = 0; j < num_leds; j++)
        {
            CRGB color = leds[j];
            total_red += color.red;
            total_green += color.green;
            total_blue += color.blue;
        }
        // Don't use the brightness scaling. That will make sure that the display always shows the pattern even if the
        // zones are currently dimmed.
        zone_color[i].r = total_red / num_leds;
        zone_color[i].g = total_green / num_leds;
        zone_color[i].b = total_blue / num_leds;
    }

}
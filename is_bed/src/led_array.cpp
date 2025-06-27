#include "led_array.h"
#include <EEPROM.h>

led_string_t led_strings[num_led_channels];

uint32_t current_channel = 0;

static const uint16_t magic = 0xCAFE;

void led_array_init()
{
    for (uint32_t i = 0; i < num_led_channels; i++)
    {
        led_strings[i].num_leds = 100;
        led_strings[i].color_ordering = WS2811_GRB;
        led_strings[i].pattern_index = 0;
        led_strings[i].palette_index = 6 + i;
        led_strings[i].single_color = CRGB::Red;
        led_strings[i].update_period_ms = 3000;
        led_strings[i].brightness = 255;
    }
}

void led_array_save()
{
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

void led_array_load()
{
    // Read the magic
    uint16_t magic_read = EEPROM.read(0) | (EEPROM.read(1) << 8);
    if (magic_read != magic)
    {
        return;
    }
    // Read the size
    uint16_t size = EEPROM.read(2) | (EEPROM.read(3) << 8);
    if (size != sizeof(led_strings))
    {
        return;
    }
    // Read the data
    EEPROM.get(4, led_strings);
}
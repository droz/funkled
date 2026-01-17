#include "led_palette.h"
#include <Arduino.h>

led_palette_t led_palettes[] = {
    {
        .name = "1/16",
        .desc = "Only 1/16 of the palette is colored.",
        .compose_colors = true,
        .palette = CRGBPalette16(CRGB::White, CRGB::Black, CRGB::Black, CRGB::Black,
                                 CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
                                 CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
                                 CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black),
    },
    {
        .name = "1/2",
        .desc = "Half of the palette is colored.",
        .compose_colors = true,
        .palette = CRGBPalette16(CRGB::White, CRGB::White, CRGB::White, CRGB::White,
                                 CRGB::White, CRGB::White, CRGB::White, CRGB::White,
                                 CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
                                 CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black),
    },
    {
        .name = "Full",
        .desc = "The palette is all the same color.",
        .compose_colors = true,
        .palette = CRGBPalette16(CRGB::White),
    },
    {
        .name = "Rainbow",
        .desc = "Colors of the rainbow.",
        .palette = RainbowColors_p,
    },
    {
        .name = "Forest",
        .desc = "A walk in the forest...",
        .palette = ForestColors_p,
    },
    {
        .name = "Ocean",
        .desc = "A cruise on the ocean...",
        .palette = OceanColors_p,
    },
    {
        .name = "Lava",
        .desc = "A trip to the volcano...",
        .palette = LavaColors_p,
    },
    {
        .name = "Cloud",
        .desc = "A flight in the clouds...",
        .palette = CloudColors_p,
    },
    {
        .name = "Heat",
        .desc = "It's getting hot in here...",
        .palette = HeatColors_p,
    },
    {
        .name = "Party",
        .desc = "A party at the disco...",
        .palette = PartyColors_p,
    },
    {
        .name = "Rainbow Stripe",
        .desc = "Colors of the rainbow with a stripe moving across.",
        .palette = RainbowStripeColors_p,
    },
};

uint32_t num_led_palettes()
{
    return sizeof(led_palettes) / sizeof(led_palette_t);
}

uint32_t palette_index = 0;

const CRGBPalette16 *composed_palette(const led_palette_t *palette, CRGB color)
{
    static CRGBPalette16 composed_palette;
    if (palette->compose_colors)
    {
        for (uint32_t i = 0; i < 16; i++)
        {
            uint8_t intensity = palette->palette.entries[i].getAverageLight();
            composed_palette.entries[i] = color.scale8(intensity);
        }
        return &composed_palette;
    }
    else
    {
        return &palette->palette;
    }
}
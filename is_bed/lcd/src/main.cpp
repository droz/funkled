#include <Arduino.h>
#include <lvgl.h>
#include "ui/is_bed_ui.h"
#include <TFT_eSPI.h>
#include <FT6336U.h>
#include <Wire.h>
#include <Adafruit_seesaw.h>

// Touchscreen
#define TOUCH_RST_PIN 37
#define TOUCH_INT_PIN 30
#define TOUCH_SCL_PIN 19
#define TOUCH_SDA_PIN 18
FT6336U ft6336u(TOUCH_SDA_PIN, TOUCH_SCL_PIN, TOUCH_RST_PIN, TOUCH_INT_PIN);

// Backlight
#define BACKLIGHT_PIN 29

// Status LED
#define HEARTBEAT_INTERVAL_MS 1000
uint8_t led_beat_counter = 0;

// Quad encoder extension board
#define ENCODERS_ADDR 0x49
#define NUM_ENCODERS 4                                       // Number of encoders
const int SS_ENC_SWITCH_PIN[NUM_ENCODERS] = {12, 14, 17, 9}; // The pins for the encoder switches
Adafruit_seesaw encoders(&Wire);

// LVGL draws into these buffers (This is double buffered)
// Ref: https://docs.lvgl.io/8.0/porting/display.html
// "A larger buffer results in better performance but above 1/10 screen
// sized buffer(s) there is no significant performance improvement.
// Therefore it's recommended to choose the size of the draw buffer(s)
// to be at least 1/10 screen sized."
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10)
static lv_color_t draw_buf[DRAW_BUF_SIZE];

// Function prototypes
static uint32_t lv_tick(void);
static void lv_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);
static void lv_encoder_read(lv_indev_t *indev, lv_indev_data_t *data);

//
// The main setup function
//
void setup()
{
    // Serial port
    Serial.begin(115200);
    String lvgl_ver = "LVGL ";
    lvgl_ver += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(lvgl_ver);

    // Touchscreen
    ft6336u.begin();

    // Quad encoders
    if (!encoders.begin(ENCODERS_ADDR))
    {
        Serial.println("Failed to initialize the quad encoders");
    }
    else
    {
        Serial.println("Quad encoders initialized");
    }
    for (int i = 0; i < NUM_ENCODERS; i++)
    {
        // Set the switch pins to INPUT_PULLUP
        encoders.pinMode(SS_ENC_SWITCH_PIN[i], INPUT_PULLUP);
    }

    // Set the interrupt to polling mode (low when there is a touch, high otherwise)
    ft6336u.write_g_mode(pollingMode);
    String touch_ver = "FT6336U ";
    touch_ver += String('V') + ft6336u.read_firmware_id() + String('M') + ft6336u.read_device_mode();
    Serial.println(touch_ver);

    // Backlight
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, HIGH);

    // Init LVGL core
    lv_init();

    // Set a tick source so that LVGL will know how much time elapsed.
    lv_tick_set_cb(lv_tick);

    // Register TFT_espi as the display driver
    lv_display_t *disp;
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, TFT_ROTATION);

    // There is no native support for the FT6336U chip in LVGL, so we use
    // a dummy driver, and we will implement the touch callback ourselves.
    lv_indev_t *touchpad = lv_indev_create();
    lv_indev_set_type(touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touchpad, lv_touchpad_read);

    // Create an input device for each encoder
    lv_indev_t *encoders[NUM_ENCODERS];
    for (uint32_t i = 0; i < NUM_ENCODERS; i++)
    {
        encoders[i] = lv_indev_create();
        lv_indev_set_type(encoders[i], LV_INDEV_TYPE_ENCODER);
        lv_indev_set_read_cb(encoders[i], lv_encoder_read);
        lv_indev_set_user_data(encoders[i], (void *)i); // Store the encoder index in the user data
    }

    // Build the UI
    is_bed_ui();

    // We are done
    Serial.println("Setup done");
    
}

void loop()
{
    // Main LVGL loop
    lv_timer_handler();
}

// Provides LVGL with access to the timer
static uint32_t lv_tick(void)
{
    return millis();
}

// Provides LVGL with access to the touchpad
// This is polled on a regular basis
static void lv_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (digitalRead(TOUCH_INT_PIN) == HIGH)
    {
        // Nothing new. Return early
        return;
    }

    FT6336U_TouchPointType tp = ft6336u.scan();
    data->state = tp.touch_count ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    switch (TFT_ROTATION)
    {
    case LV_DISPLAY_ROTATION_90:
    case LV_DISPLAY_ROTATION_270:
        data->point.x = TFT_HOR_RES - 1 - tp.tp[0].x;
        data->point.y = TFT_VER_RES - 1 - tp.tp[0].y;
        break;
    case LV_DISPLAY_ROTATION_0:
    case LV_DISPLAY_ROTATION_180:
        data->point.x = tp.tp[0].x;
        data->point.y = tp.tp[0].y;
        break;
    }
}

// Provides LVGL with access to the encoder
// This is polled on a regular basis
static void lv_encoder_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    // Figure out the encoder index from the user data
    uint32_t encoder_index = (uint32_t)lv_indev_get_user_data(indev);
    // Read the switch state
    data->state = encoders.digitalRead(SS_ENC_SWITCH_PIN[encoder_index]) ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
    // Read the encoder delta. Apply some gain to make the encoder more sensitive.
    data->enc_diff = -encoders.getEncoderDelta(encoder_index);
}

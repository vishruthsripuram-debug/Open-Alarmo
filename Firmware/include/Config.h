#pragma once
#include <Arduino.h>

// =====================================================================
// WIFI / TIME
// =====================================================================
#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"
#define NTP_SERVER          "pool.ntp.org"
#define GMT_OFFSET_SEC      (10 * 3600)      // Sydney standard time, adjust as needed
#define DST_OFFSET_SEC      0                 // set 3600 during AEDT if you want auto DST
#define WIFI_CONNECT_TIMEOUT_MS 8000

// =====================================================================
// ROTARY DIAL  (quadrature encoder + push button, mounted on top)
// =====================================================================
// NOTE: deliberately avoiding GPIO34/35/36/39 here -- those are
// input-only pins on the ESP32 with no internal pull-up/pull-down
// hardware, so INPUT_PULLUP silently does nothing on them. 33/32/26
// are regular GPIOs and work correctly with INPUT_PULLUP.
#define ENC_PIN_A            33
#define ENC_PIN_B            32
#define ENC_PIN_BTN          26
#define ENC_BTN_ACTIVE_LOW  true
#define ENC_LONG_PRESS_MS   600
#define ENC_DETENTS_PER_CLICK 4  // most cheap encoders emit 4 edges per detent

// =====================================================================
// SPEAKER  (piezo / small speaker driven through a transistor from a
//           PWM-capable GPIO -- see README wiring section)
// =====================================================================
#define SPEAKER_PIN          25
#define SPEAKER_LEDC_CHANNEL 0
#define SPEAKER_LEDC_RES_BITS 10

// =====================================================================
// TOUCH-FREE BACKLIGHT DIMMING (PWM)
// =====================================================================
#define BL_LEDC_CHANNEL      1
#define BL_LEDC_RES_BITS     8
#define BL_LEDC_FREQ_HZ      5000
#define DEFAULT_BRIGHTNESS   200   // 0-255

// =====================================================================
// RTC
// =====================================================================
#define USE_DS3231            1   // 1 = battery-backed external RTC present, 0 = NTP/internal only
#define I2C_SDA               21
#define I2C_SCL               22

// =====================================================================
// ALARMS
// =====================================================================
#define MAX_ALARMS            8
#define SNOOZE_MINUTES        9
#define ALARM_LABEL_LEN        20
#define ALARM_RING_TIMEOUT_MIN 10   // auto-stop a ringing alarm after this long

// =====================================================================
// COLOUR PALETTE  (warm cream body / soft orange accent, inspired by
// the look of dedicated bedside alarm clocks -- not any copyrighted
// artwork or assets)
// =====================================================================
#define COL_BG           0xFBE0   // warm cream background
#define COL_BG_DARK       0x18E3   // used behind the "ringing" screen
#define COL_CARD          0xFFFF
#define COL_CARD_SHADOW   0xE73C
#define COL_ACCENT        0xFB40   // warm orange
#define COL_ACCENT_DARK   0xC300
#define COL_TEXT          0x2104   // near-black
#define COL_TEXT_MUTED    0x8C71
#define COL_GOOD          0x3E67   // soft green (enabled / connected)
#define COL_WARN          0xFA00

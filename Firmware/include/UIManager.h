#pragma once
#include <Arduino.h>
#include <time.h>
#include <TFT_eSPI.h>
#include "AlarmManager.h"

enum class MenuItem : uint8_t { NEW_ALARM, ALARMS, BRIGHTNESS, BACK, COUNT };

// Which field is focused while editing an alarm. Rotate changes the
// focused field's value; a short click cycles focus; a LONG PRESS from
// anywhere in this screen saves and exits (see main.cpp) -- so there is
// no separate on-screen "Save" button to accidentally click past.
enum class EditField : uint8_t { HOUR, MINUTE, REPEAT, ENABLED, COUNT };

class UIManager {
public:
    void begin();
    void setBrightness(uint8_t value);   // 0-255
    uint8_t brightness() const { return _brightness; }

    // ---- screens ----
    void drawClock(const struct tm &t, bool wifiOk, bool hasNextAlarm,
                    uint8_t nextHour, uint8_t nextMinute, const char *nextRepeat);

    void drawMenu(MenuItem selected);

    void drawAlarmList(AlarmManager &alarms, uint8_t selectedIndex);

    void drawEditAlarm(Alarm &a, EditField field, bool isNew);

    void drawBrightnessScreen(uint8_t value);

    void drawRinging(const Alarm &a, uint32_t animMs, bool snoozeHint);

    void forceFullRedraw() { _lastScreenSignature = 0xFFFFFFFF; }

private:
    TFT_eSPI   _tft = TFT_eSPI();
    TFT_eSprite _timeSprite = TFT_eSprite(&_tft);
    uint8_t     _brightness = 200;
    uint32_t    _lastScreenSignature = 0;

    // cached strings so we only repaint the static parts of the clock face
    // when they actually change (keeps things flicker-free & fast)
    char _lastDateStr[24]  = "";
    char _lastAlarmStr[32] = "";
    bool _lastWifiOk        = false;
    int  _lastDrawnMinute   = -1;
    int  _lastDrawnSecond   = -1;

    void drawCard(int x, int y, int w, int h, uint16_t color);
    void drawWifiDot(bool ok);
    void drawHeaderBar(const char *title);
    void drawDots(uint8_t count, uint8_t selected, int y);
};

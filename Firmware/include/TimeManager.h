#pragma once
#include <Arduino.h>
#include <time.h>
#include "Config.h"
#if USE_DS3231
  #include <RTClib.h>
#endif

class TimeManager {
public:
    void begin();                 // connects WiFi (briefly), syncs, then can drop WiFi
    void loop();                  // call every loop(); handles periodic resync

    bool     wifiConnected() const { return _wifiOk; }
    struct tm now();              // current wall-clock time
    bool     minuteChanged();     // true exactly once when the minute rolls over

private:
    bool connectWiFi();
    void syncFromNTP();

    bool     _wifiOk       = false;
    uint32_t _lastNtpSyncMs = 0;
    int8_t   _lastSeenMinute = -1;

#if USE_DS3231
    RTC_DS3231 _rtc;
    bool       _rtcOk = false;
#endif
};

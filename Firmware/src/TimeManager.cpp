#include "TimeManager.h"
#include "Config.h"
#include <WiFi.h>
#include <Wire.h>

void TimeManager::begin() {
#if USE_DS3231
    Wire.begin(I2C_SDA, I2C_SCL);
    _rtcOk = _rtc.begin();
    if (_rtcOk && _rtc.lostPower()) {
        // No time source yet -- will be set as soon as WiFi/NTP succeeds below.
    }
#endif

    _wifiOk = connectWiFi();
    if (_wifiOk) {
        syncFromNTP();
        // Once synced we don't need WiFi kept alive for a clock; drop it to
        // save power / reduce interference with the speaker's PWM. Comment
        // this out if you want other WiFi features (weather, HA, etc).
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }
    _lastNtpSyncMs = millis();
}

bool TimeManager::connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(100);
    }
    return WiFi.status() == WL_CONNECTED;
}

void TimeManager::syncFromNTP() {
    configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);
    struct tm t;
    if (getLocalTime(&t, 5000)) {
#if USE_DS3231
        if (_rtcOk) {
            _rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                                  t.tm_hour, t.tm_min, t.tm_sec));
        }
#endif
    }
}

void TimeManager::loop() {
    // Resync once a day if you keep WiFi around; harmless no-op otherwise
    // since WiFi is off after begin() in the default configuration above.
    if (millis() - _lastNtpSyncMs > 24UL * 60 * 60 * 1000 && WiFi.status() == WL_CONNECTED) {
        syncFromNTP();
        _lastNtpSyncMs = millis();
    }
}

struct tm TimeManager::now() {
    struct tm t{};
#if USE_DS3231
    if (_rtcOk) {
        DateTime d = _rtc.now();
        t.tm_year = d.year() - 1900;
        t.tm_mon  = d.month() - 1;
        t.tm_mday = d.day();
        t.tm_hour = d.hour();
        t.tm_min  = d.minute();
        t.tm_sec  = d.second();
        t.tm_wday = d.dayOfTheWeek();
        return t;
    }
#endif
    getLocalTime(&t);   // falls back to internal RTC (needs periodic WiFi sync)
    return t;
}

bool TimeManager::minuteChanged() {
    struct tm t = now();
    if (t.tm_min != _lastSeenMinute) {
        _lastSeenMinute = t.tm_min;
        return true;
    }
    return false;
}

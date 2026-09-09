#include "AlarmManager.h"

void AlarmManager::begin() {
    _prefs.begin("alarms", false);

    for (uint8_t i = 0; i < MAX_ALARMS; i++) {
        char key[8];
        snprintf(key, sizeof(key), "a%u", i);
        size_t len = _prefs.getBytesLength(key);
        if (len == sizeof(Alarm)) {
            _prefs.getBytes(key, &_alarms[i], sizeof(Alarm));
        } else {
            _alarms[i] = Alarm{};   // default, disabled
        }
    }
}

void AlarmManager::save() {
    for (uint8_t i = 0; i < MAX_ALARMS; i++) {
        char key[8];
        snprintf(key, sizeof(key), "a%u", i);
        _prefs.putBytes(key, &_alarms[i], sizeof(Alarm));
    }
}

int AlarmManager::checkTrigger(uint8_t weekday, uint8_t hour, uint8_t minute) {
    int minuteOfDay = hour * 60 + minute;
    if (minuteOfDay == _lastFiredMinuteOfDay) return -1;  // already handled this minute

    // snooze takes priority
    if (_snoozeActive && hour == _snoozeHour && minute == _snoozeMinute) {
        _lastFiredMinuteOfDay = minuteOfDay;
        _snoozeActive = false;
        return -2;   // caller: re-ring using the alarm that was snoozed (main.cpp tracks which)
    }

    for (uint8_t i = 0; i < MAX_ALARMS; i++) {
        Alarm &a = _alarms[i];
        if (!a.enabled || a.hour != hour || a.minute != minute) continue;

        bool dayOk = (a.daysMask == 0) || (a.daysMask & (1 << weekday));
        if (!dayOk) continue;

        _lastFiredMinuteOfDay = minuteOfDay;

        if (a.daysMask == 0) a.enabled = false;   // one-off alarm consumes itself

        return i;
    }
    return -1;
}

void AlarmManager::armSnooze(uint8_t fromHour, uint8_t fromMinute) {
    uint16_t total = fromHour * 60 + fromMinute + SNOOZE_MINUTES;
    total %= (24 * 60);
    _snoozeHour   = total / 60;
    _snoozeMinute = total % 60;
    _snoozeActive = true;
}

bool AlarmManager::snoozeDue(uint8_t hour, uint8_t minute) const {
    return _snoozeActive && hour == _snoozeHour && minute == _snoozeMinute;
}

void AlarmManager::clearSnooze() {
    _snoozeActive = false;
}

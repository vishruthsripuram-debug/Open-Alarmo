#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

struct Alarm {
    bool    enabled  = false;
    uint8_t hour     = 7;
    uint8_t minute   = 0;
    // bit0=Sun .. bit6=Sat. 0 == "one-off" (fires once, then auto-disables)
    uint8_t daysMask = 0;
    char    label[ALARM_LABEL_LEN] = "Alarm";
};

class AlarmManager {
public:
    void begin();                 // loads from flash
    void save();                  // persists current in-memory alarms

    Alarm&  at(uint8_t i)        { return _alarms[i]; }
    uint8_t count() const         { return MAX_ALARMS; }

    // Returns index of an alarm that should start ringing right now
    // (called once per minute-rollover by main loop), or -1 if none.
    int checkTrigger(uint8_t weekday, uint8_t hour, uint8_t minute);

    // Snooze bookkeeping for the currently-ringing alarm
    void armSnooze(uint8_t fromHour, uint8_t fromMinute);
    bool snoozeDue(uint8_t hour, uint8_t minute) const;
    void clearSnooze();
    bool isSnoozed() const { return _snoozeActive; }

private:
    Alarm      _alarms[MAX_ALARMS];
    Preferences _prefs;

    // guard against re-triggering within the same minute
    int8_t   _lastFiredMinuteOfDay = -1;

    bool     _snoozeActive = false;
    uint8_t  _snoozeHour = 0, _snoozeMinute = 0;
};

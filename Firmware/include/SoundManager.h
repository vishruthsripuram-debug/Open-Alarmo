#pragma once
#include <Arduino.h>

struct Note {
    uint16_t freq;   // Hz, 0 = rest
    uint16_t ms;     // duration
};

class SoundManager {
public:
    void begin();

    // short UI feedback sounds
    void tickClick();     // rotating the dial one detent
    void confirmBeep();   // selecting / saving
    void backBeep();      // going back / cancel

    // alarm melody -- call startAlarm() once, then update() every loop()
    // until it returns false (or call stopAlarm() to cut it off, e.g. on
    // dismiss/snooze).
    void startAlarm();
    void stopAlarm();
    bool update();          // returns true while still playing/looping
    bool isPlaying() const { return _playing; }

private:
    void tone(uint16_t freq, uint16_t ms);
    void silence();

    bool     _playing   = false;
    size_t   _noteIndex  = 0;
    uint32_t _noteStartMs = 0;
    bool     _noteSounding = false;
};

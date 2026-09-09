// =====================================================================
//  Nintendo-Alarmo-style ESP32 alarm clock
//
//  Hardware: ESP32 dev board + 2.8" SPI TFT (ILI9341) + small speaker
//            driven through a transistor + a rotary encoder w/ push
//            button mounted on top as the main control dial.
//
//  Interaction model (mirrors the "turn the dial" feel of a dedicated
//  bedside alarm clock):
//    Clock face  : rotate -> jump into Alarm List | click -> Main Menu
//    Main Menu   : rotate -> highlight | click -> select | hold -> back
//    Alarm List  : rotate -> highlight | click -> edit | hold -> delete
//    Edit Alarm  : rotate -> change focused field | click -> next field
//                  hold -> save & return to list
//    Brightness  : rotate -> adjust | click/hold -> save & back
//    Ringing     : rotate (either way) -> snooze | click/hold -> dismiss
//
//  See README.md for wiring and library setup.
// =====================================================================
#include <Arduino.h>
#include "Config.h"
#include "UIManager.h"
#include "RotaryInput.h"
#include "SoundManager.h"
#include "AlarmManager.h"
#include "TimeManager.h"

// ---------------------------------------------------------------------
// globals
// ---------------------------------------------------------------------
UIManager     ui;
RotaryInput   dial;
SoundManager  sound;
AlarmManager  alarms;
TimeManager   clock_;

enum class AppState { CLOCK, MENU, ALARM_LIST, EDIT_ALARM, BRIGHTNESS, RINGING };
AppState state = AppState::CLOCK;

MenuItem  menuSelect = MenuItem::NEW_ALARM;
uint8_t   alarmListSelect = 0;

EditField editField = EditField::HOUR;
uint8_t   editingIndex = 0;
bool      isNewAlarm = false;

int       ringingIndex = -1;   // index into alarms[], or -1 if snooze-triggered w/ no stable index
Alarm     ringingAlarmSnapshot;
uint32_t  ringingAnimStart = 0;

uint8_t   pendingBrightness = DEFAULT_BRIGHTNESS;

uint32_t  lastUiTickMs = 0;

// ---------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------
static const char *repeatLabelFor(uint8_t mask) {
    if (mask == 0)          return "Once";
    if (mask == 0x7F)       return "Everyday";
    if (mask == 0b0111110)  return "Weekdays";
    if (mask == 0b1000001)  return "Weekends";
    return "Custom";
}

// Finds the soonest upcoming enabled alarm (today or later this week) for
// the clock-face "Next" chip. Returns false if none are enabled.
static bool findNextAlarm(uint8_t nowWday, uint8_t nowH, uint8_t nowM,
                           uint8_t &outH, uint8_t &outM, const char *&outRepeat) {
    int bestDelta = INT32_MAX;
    int bestIdx = -1;
    int nowMin = nowH * 60 + nowM;

    for (uint8_t i = 0; i < alarms.count(); i++) {
        Alarm &a = alarms.at(i);
        if (!a.enabled) continue;
        int alarmMin = a.hour * 60 + a.minute;

        for (int d = 0; d < 7; d++) {
            uint8_t wday = (nowWday + d) % 7;
            // one-off (daysMask == 0) alarms only ever count for "today, if still ahead"
            if (a.daysMask == 0) {
                if (d > 0) continue;
            } else if (!(a.daysMask & (1 << wday))) {
                continue;
            }

            int delta = d * 1440 + alarmMin - nowMin;
            if (delta < 0) continue;
            if (delta < bestDelta) {
                bestDelta = delta;
                bestIdx = i;
            }
        }
    }

    if (bestIdx < 0) return false;
    outH = alarms.at(bestIdx).hour;
    outM = alarms.at(bestIdx).minute;
    outRepeat = repeatLabelFor(alarms.at(bestIdx).daysMask);
    return true;
}

static void enterRinging(int idx, const Alarm &snapshot) {
    state = AppState::RINGING;
    ringingIndex = idx;
    ringingAlarmSnapshot = snapshot;
    ringingAnimStart = millis();
    sound.startAlarm();
    ui.forceFullRedraw();
}

static void cycleRepeatPreset(Alarm &a, int direction) {
    static const uint8_t presets[] = {0x00, 0x7F, 0b0111110, 0b1000001};
    const int n = 4;
    int cur = 0;
    for (int i = 0; i < n; i++) if (presets[i] == a.daysMask) { cur = i; break; }
    cur = (cur + direction % n + n) % n;
    a.daysMask = presets[cur];
}

// ---------------------------------------------------------------------
// setup / loop
// ---------------------------------------------------------------------
void setup() {
    Serial.begin(115200);

    ui.begin();
    dial.begin();
    sound.begin();
    alarms.begin();
    clock_.begin();     // connects WiFi briefly to sync time, then (by default) drops it

    pendingBrightness = ui.brightness();
    ui.forceFullRedraw();
}

void loop() {
    DialEvent ev = dial.poll();
    struct tm t = clock_.now();
    clock_.loop();

    // ---- global: alarm triggering (checked once per minute, highest priority) ----
    if (state != AppState::RINGING && clock_.minuteChanged()) {
        int idx = alarms.checkTrigger(t.tm_wday, t.tm_hour, t.tm_min);
        if (idx == -2) {
            // snoozed alarm coming back due -- reuse last snapshot's label/time for display
            enterRinging(-1, ringingAlarmSnapshot);
        } else if (idx >= 0) {
            enterRinging(idx, alarms.at(idx));
        }
    }

    switch (state) {
        // -----------------------------------------------------------
        case AppState::CLOCK: {
            if (ev.ticks != 0) {
                state = AppState::ALARM_LIST;
                alarmListSelect = (uint8_t)constrain((int)alarmListSelect + ev.ticks, 0, alarms.count() - 1);
                sound.tickClick();
                ui.forceFullRedraw();
            } else if (ev.clicked) {
                state = AppState::MENU;
                menuSelect = MenuItem::NEW_ALARM;
                sound.confirmBeep();
                ui.forceFullRedraw();
            } else {
                uint8_t nh = 0, nm = 0; const char *nr = "";
                bool has = findNextAlarm(t.tm_wday, t.tm_hour, t.tm_min, nh, nm, nr);
                ui.drawClock(t, clock_.wifiConnected(), has, nh, nm, nr);
            }
            break;
        }

        // -----------------------------------------------------------
        case AppState::MENU: {
            if (ev.ticks != 0) {
                int n = (int)MenuItem::COUNT;
                int cur = ((int)menuSelect + ev.ticks) % n;
                if (cur < 0) cur += n;
                menuSelect = (MenuItem)cur;
                sound.tickClick();
            }
            if (ev.clicked) {
                sound.confirmBeep();
                switch (menuSelect) {
                    case MenuItem::NEW_ALARM: {
                        // find a free (disabled) slot, else reuse slot 0
                        int freeIdx = 0;
                        for (uint8_t i = 0; i < alarms.count(); i++) {
                            if (!alarms.at(i).enabled) { freeIdx = i; break; }
                        }
                        editingIndex = freeIdx;
                        isNewAlarm = true;
                        Alarm &a = alarms.at(editingIndex);
                        a = Alarm{};           // reset to sensible default
                        a.enabled = true;
                        strncpy(a.label, "Alarm", ALARM_LABEL_LEN);
                        editField = EditField::HOUR;
                        state = AppState::EDIT_ALARM;
                        break;
                    }
                    case MenuItem::ALARMS:
                        state = AppState::ALARM_LIST;
                        alarmListSelect = 0;
                        break;
                    case MenuItem::BRIGHTNESS:
                        pendingBrightness = ui.brightness();
                        state = AppState::BRIGHTNESS;
                        break;
                    case MenuItem::BACK:
                    default:
                        state = AppState::CLOCK;
                        break;
                }
                ui.forceFullRedraw();
            }
            if (ev.longPressed) {
                sound.backBeep();
                state = AppState::CLOCK;
                ui.forceFullRedraw();
            }
            if (state == AppState::MENU) ui.drawMenu(menuSelect);
            break;
        }

        // -----------------------------------------------------------
        case AppState::ALARM_LIST: {
            if (ev.ticks != 0) {
                alarmListSelect = (uint8_t)constrain((int)alarmListSelect + ev.ticks, 0, alarms.count() - 1);
                sound.tickClick();
            }
            if (ev.clicked) {
                editingIndex = alarmListSelect;
                isNewAlarm = false;
                editField = EditField::HOUR;
                sound.confirmBeep();
                state = AppState::EDIT_ALARM;
                ui.forceFullRedraw();
                break;
            }
            if (ev.longPressed) {
                // delete the selected alarm
                alarms.at(alarmListSelect) = Alarm{};
                alarms.save();
                sound.backBeep();
                ui.forceFullRedraw();
            }
            if (state == AppState::ALARM_LIST) ui.drawAlarmList(alarms, alarmListSelect);
            break;
        }

        // -----------------------------------------------------------
        case AppState::EDIT_ALARM: {
            Alarm &a = alarms.at(editingIndex);

            if (ev.ticks != 0) {
                switch (editField) {
                    case EditField::HOUR:
                        a.hour = (uint8_t)(((int)a.hour + ev.ticks) % 24 + 24) % 24;
                        break;
                    case EditField::MINUTE:
                        a.minute = (uint8_t)(((int)a.minute + ev.ticks) % 60 + 60) % 60;
                        break;
                    case EditField::REPEAT:
                        cycleRepeatPreset(a, ev.ticks > 0 ? 1 : -1);
                        break;
                    case EditField::ENABLED:
                        a.enabled = !a.enabled;
                        break;
                    default: break;
                }
                sound.tickClick();
            }
            if (ev.clicked) {
                int next = ((int)editField + 1) % (int)EditField::COUNT;
                editField = (EditField)next;
                sound.tickClick();
            }
            if (ev.longPressed) {
                alarms.save();
                sound.confirmBeep();
                state = AppState::ALARM_LIST;
                alarmListSelect = editingIndex;
                ui.forceFullRedraw();
                break;
            }
            if (state == AppState::EDIT_ALARM) ui.drawEditAlarm(a, editField, isNewAlarm);
            break;
        }

        // -----------------------------------------------------------
        case AppState::BRIGHTNESS: {
            if (ev.ticks != 0) {
                int v = (int)pendingBrightness + ev.ticks * 8;
                pendingBrightness = (uint8_t)constrain(v, 10, 255);
                ui.setBrightness(pendingBrightness);
            }
            if (ev.clicked || ev.longPressed) {
                sound.confirmBeep();
                state = AppState::MENU;
                ui.forceFullRedraw();
                break;
            }
            if (state == AppState::BRIGHTNESS) ui.drawBrightnessScreen(pendingBrightness);
            break;
        }

        // -----------------------------------------------------------
        case AppState::RINGING: {
            sound.update();

            if (ev.ticks != 0) {
                // twist the dial to snooze -- classic bedside-clock gesture
                sound.stopAlarm();
                alarms.armSnooze(t.tm_hour, t.tm_min);
                sound.confirmBeep();
                state = AppState::CLOCK;
                ui.forceFullRedraw();
                break;
            }
            if (ev.clicked || ev.longPressed) {
                sound.stopAlarm();
                alarms.clearSnooze();
                sound.backBeep();
                state = AppState::CLOCK;
                ui.forceFullRedraw();
                break;
            }

            // safety timeout so it can never ring forever unattended
            if (millis() - ringingAnimStart > (uint32_t)ALARM_RING_TIMEOUT_MIN * 60UL * 1000UL) {
                sound.stopAlarm();
                alarms.clearSnooze();
                state = AppState::CLOCK;
                ui.forceFullRedraw();
                break;
            }

            ui.drawRinging(ringingAlarmSnapshot, millis() - ringingAnimStart, true);
            break;
        }
    }

    // Ringing screen animates continuously; everything else can idle a
    // touch to keep CPU/power reasonable without feeling laggy.
    if (state != AppState::RINGING) {
        delay(15);
    }
}

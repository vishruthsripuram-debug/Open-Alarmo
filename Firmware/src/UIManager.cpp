#include "UIManager.h"
#include "Config.h"
#include <math.h>

// ---------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------
void UIManager::begin() {
    _tft.init();
    _tft.setRotation(1);              // landscape: 320 x 240
    _tft.fillScreen(COL_BG);

    ledcSetup(BL_LEDC_CHANNEL, BL_LEDC_FREQ_HZ, BL_LEDC_RES_BITS);
    ledcAttachPin(TFT_BL, BL_LEDC_CHANNEL);
    setBrightness(DEFAULT_BRIGHTNESS);

    // sprite big enough for "23:59" in Font7 plus some headroom
    _timeSprite.setColorDepth(16);
    _timeSprite.createSprite(260, 90);
}

void UIManager::setBrightness(uint8_t value) {
    _brightness = value;
    ledcWrite(BL_LEDC_CHANNEL, value);
}

// ---------------------------------------------------------------------
// small shared helpers
// ---------------------------------------------------------------------
void UIManager::drawCard(int x, int y, int w, int h, uint16_t color) {
    _tft.fillRoundRect(x + 2, y + 3, w, h, 10, COL_CARD_SHADOW);   // soft shadow
    _tft.fillRoundRect(x, y, w, h, 10, color);
}

void UIManager::drawWifiDot(bool ok) {
    int cx = _tft.width() - 16, cy = 14;
    _tft.fillCircle(cx, cy, 5, ok ? COL_GOOD : COL_TEXT_MUTED);
}

void UIManager::drawHeaderBar(const char *title) {
    _tft.fillRect(0, 0, _tft.width(), 34, COL_BG);
    _tft.setTextColor(COL_TEXT, COL_BG);
    _tft.setTextDatum(TL_DATUM);
    _tft.setFreeFont(nullptr);
    _tft.setTextFont(4);
    _tft.drawString(title, 12, 6);
    _tft.drawFastHLine(0, 34, _tft.width(), COL_CARD_SHADOW);
}

void UIManager::drawDots(uint8_t count, uint8_t selected, int y) {
    int totalW = count * 16;
    int startX = (_tft.width() - totalW) / 2;
    for (uint8_t i = 0; i < count; i++) {
        uint16_t c = (i == selected) ? COL_ACCENT : COL_CARD_SHADOW;
        _tft.fillCircle(startX + i * 16 + 8, y, i == selected ? 5 : 3, c);
    }
}

// ---------------------------------------------------------------------
// CLOCK FACE
// ---------------------------------------------------------------------
void UIManager::drawClock(const struct tm &t, bool wifiOk, bool hasNextAlarm,
                           uint8_t nextHour, uint8_t nextMinute, const char *nextRepeat) {
    bool fullRedraw = (_lastScreenSignature != 1);
    _lastScreenSignature = 1;

    int W = _tft.width(), H = _tft.height();

    if (fullRedraw) {
        _tft.fillScreen(COL_BG);
        _lastDateStr[0] = '\0';
        _lastAlarmStr[0] = '\0';
        _lastWifiOk = !wifiOk;   // force redraw
    }

    // ---- big time, drawn into a sprite so the colon can blink without
    //      flicker on the rest of the screen ----
    char timeStr[6];
    bool colonOn = (t.tm_sec % 2 == 0);
    snprintf(timeStr, sizeof(timeStr), "%02d%c%02d", t.tm_hour, colonOn ? ':' : ' ', t.tm_min);

    if (fullRedraw || t.tm_sec != _lastDrawnSecond) {
        _timeSprite.fillSprite(COL_BG);
        _timeSprite.setTextColor(COL_TEXT, COL_BG);
        _timeSprite.setTextDatum(MC_DATUM);
        _timeSprite.setTextFont(7);
        _timeSprite.drawString(timeStr, _timeSprite.width() / 2, _timeSprite.height() / 2 - 4);
        _timeSprite.pushSprite((W - _timeSprite.width()) / 2, 46);
        _lastDrawnMinute  = t.tm_min;
        _lastDrawnSecond  = t.tm_sec;
    }

    // ---- date line (only redraw if changed) ----
    static const char *WD[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    static const char *MO[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    char dateStr[24];
    snprintf(dateStr, sizeof(dateStr), "%s, %d %s", WD[t.tm_wday], t.tm_mday, MO[t.tm_mon]);
    if (strcmp(dateStr, _lastDateStr) != 0) {
        _tft.fillRect(0, 140, W, 24, COL_BG);
        _tft.setTextFont(4);
        _tft.setTextColor(COL_TEXT_MUTED, COL_BG);
        _tft.setTextDatum(TC_DATUM);
        _tft.drawString(dateStr, W / 2, 140);
        strncpy(_lastDateStr, dateStr, sizeof(_lastDateStr));
    }

    // ---- next-alarm chip ----
    char alarmStr[32];
    if (hasNextAlarm) {
        snprintf(alarmStr, sizeof(alarmStr), "Next  %02d:%02d  -  %s", nextHour, nextMinute, nextRepeat);
    } else {
        snprintf(alarmStr, sizeof(alarmStr), "No alarms set");
    }
    if (strcmp(alarmStr, _lastAlarmStr) != 0) {
        int cw = W - 40, ch = 40, cx = 20, cy = 176;
        drawCard(cx, cy, cw, ch, COL_CARD);
        _tft.setTextFont(2);
        _tft.setTextColor(hasNextAlarm ? COL_TEXT : COL_TEXT_MUTED, COL_CARD);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString(alarmStr, cx + cw / 2, cy + ch / 2);
        strncpy(_lastAlarmStr, alarmStr, sizeof(_lastAlarmStr));
    }

    // ---- wifi status dot ----
    if (fullRedraw || wifiOk != _lastWifiOk) {
        drawWifiDot(wifiOk);
        _lastWifiOk = wifiOk;
    }

    // ---- hint footer ----
    if (fullRedraw) {
        _tft.setTextFont(1);
        _tft.setTextColor(COL_TEXT_MUTED, COL_BG);
        _tft.setTextDatum(BC_DATUM);
        _tft.drawString("Turn dial for alarms   -   Press for menu", W / 2, H - 6);
    }
}

// ---------------------------------------------------------------------
// MAIN MENU
// ---------------------------------------------------------------------
void UIManager::drawMenu(MenuItem selected) {
    _lastScreenSignature = 2;
    _tft.fillScreen(COL_BG);
    drawHeaderBar("Menu");

    static const char *labels[] = {"New Alarm", "Alarms", "Brightness", "Back"};
    int n = (int)MenuItem::COUNT;
    int y = 48, h = 40, gap = 10, x = 20, w = _tft.width() - 40;

    for (int i = 0; i < n; i++) {
        bool sel = ((int)selected == i);
        drawCard(x, y, w, h, sel ? COL_ACCENT : COL_CARD);
        _tft.setTextFont(4);
        _tft.setTextDatum(ML_DATUM);
        _tft.setTextColor(sel ? COL_CARD : COL_TEXT, sel ? COL_ACCENT : COL_CARD);
        _tft.drawString(labels[i], x + 16, y + h / 2 + 1);
        y += h + gap;
    }
}

// ---------------------------------------------------------------------
// ALARM LIST
// ---------------------------------------------------------------------
static const char *repeatLabel(uint8_t mask) {
    if (mask == 0)    return "Once";
    if (mask == 0x7F) return "Everyday";
    if (mask == 0b0111110) return "Weekdays";
    if (mask == 0b1000001) return "Weekends";
    return "Custom";
}

void UIManager::drawAlarmList(AlarmManager &alarms, uint8_t selectedIndex) {
    _lastScreenSignature = 3;
    _tft.fillScreen(COL_BG);
    drawHeaderBar("Alarms");

    const int visible = 4;
    int count = alarms.count();
    int scroll = selectedIndex - 1;
    if (scroll < 0) scroll = 0;
    if (scroll > count - visible) scroll = max(0, count - visible);

    int y = 42, h = 44, gap = 6, x = 14, w = _tft.width() - 28;

    for (int row = 0; row < visible; row++) {
        int i = scroll + row;
        if (i >= count) break;
        Alarm &a = alarms.at(i);
        bool sel = (i == (int)selectedIndex);

        uint16_t bg = sel ? COL_ACCENT : COL_CARD;
        drawCard(x, y, w, h, bg);

        char timeStr[8];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", a.hour, a.minute);

        _tft.setTextFont(4);
        _tft.setTextDatum(ML_DATUM);
        _tft.setTextColor(sel ? COL_CARD : (a.enabled ? COL_TEXT : COL_TEXT_MUTED), bg);
        _tft.drawString(timeStr, x + 14, y + h / 2);

        _tft.setTextFont(2);
        _tft.setTextDatum(MR_DATUM);
        _tft.setTextColor(sel ? COL_CARD : COL_TEXT_MUTED, bg);
        _tft.drawString(repeatLabel(a.daysMask), x + w - 44, y + h / 2);

        // enabled indicator dot
        _tft.fillCircle(x + w - 18, y + h / 2, 6, a.enabled ? (sel ? COL_CARD : COL_GOOD) : COL_CARD_SHADOW);

        y += h + gap;
    }

    drawDots(count < 8 ? count : 8, selectedIndex % 8, _tft.height() - 18);

    _tft.setTextFont(1);
    _tft.setTextColor(COL_TEXT_MUTED, COL_BG);
    _tft.setTextDatum(BC_DATUM);
    _tft.drawString("Press to edit   -   Hold to delete", _tft.width() / 2, _tft.height() - 4);
}

// ---------------------------------------------------------------------
// EDIT ALARM
// ---------------------------------------------------------------------
void UIManager::drawEditAlarm(Alarm &a, EditField field, bool isNew) {
    _lastScreenSignature = 4;
    _tft.fillScreen(COL_BG);
    drawHeaderBar(isNew ? "New Alarm" : "Edit Alarm");

    int W = _tft.width();

    // Hour / minute, side by side, focused one gets an accent box
    int boxW = 90, boxH = 70, gap = 20;
    int totalW = boxW * 2 + gap;
    int startX = (W - totalW) / 2;
    int y = 48;

    bool hourFocus = (field == EditField::HOUR);
    bool minFocus  = (field == EditField::MINUTE);

    drawCard(startX, y, boxW, boxH, hourFocus ? COL_ACCENT : COL_CARD);
    drawCard(startX + boxW + gap, y, boxW, boxH, minFocus ? COL_ACCENT : COL_CARD);

    char buf[4];
    _tft.setTextFont(7);
    _tft.setTextDatum(MC_DATUM);

    snprintf(buf, sizeof(buf), "%02d", a.hour);
    _tft.setTextColor(hourFocus ? COL_CARD : COL_TEXT, hourFocus ? COL_ACCENT : COL_CARD);
    _tft.drawString(buf, startX + boxW / 2, y + boxH / 2);

    snprintf(buf, sizeof(buf), "%02d", a.minute);
    _tft.setTextColor(minFocus ? COL_CARD : COL_TEXT, minFocus ? COL_ACCENT : COL_CARD);
    _tft.drawString(buf, startX + boxW + gap + boxW / 2, y + boxH / 2);

    // Repeat chip
    int ry = y + boxH + 16, rh = 34;
    bool repFocus = (field == EditField::REPEAT);
    drawCard(startX, ry, boxW * 2 + gap, rh, repFocus ? COL_ACCENT : COL_CARD);
    _tft.setTextFont(4);
    _tft.setTextColor(repFocus ? COL_CARD : COL_TEXT, repFocus ? COL_ACCENT : COL_CARD);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString(repeatLabel(a.daysMask), startX + boxW + gap / 2, ry + rh / 2);

    // Enabled toggle, centred
    int by = ry + rh + 14, bh = 34, bw = boxW * 2 + gap;
    bool enFocus = (field == EditField::ENABLED);
    drawCard(startX, by, bw, bh, enFocus ? COL_ACCENT : (a.enabled ? COL_GOOD : COL_CARD));
    _tft.setTextFont(2);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor((enFocus || a.enabled) ? COL_CARD : COL_TEXT, enFocus ? COL_ACCENT : (a.enabled ? COL_GOOD : COL_CARD));
    _tft.drawString(a.enabled ? "Enabled" : "Disabled", startX + bw / 2, by + bh / 2);

    _tft.setTextFont(1);
    _tft.setTextColor(COL_TEXT_MUTED, COL_BG);
    _tft.setTextDatum(BC_DATUM);
    _tft.drawString("Rotate to change   -   Press to move on   -   Hold to save", W / 2, _tft.height() - 6);
}

// ---------------------------------------------------------------------
// BRIGHTNESS
// ---------------------------------------------------------------------
void UIManager::drawBrightnessScreen(uint8_t value) {
    _lastScreenSignature = 5;
    _tft.fillScreen(COL_BG);
    drawHeaderBar("Brightness");

    int W = _tft.width();
    char pct[8];
    snprintf(pct, sizeof(pct), "%d%%", (value * 100) / 255);

    _tft.setTextFont(7);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COL_TEXT, COL_BG);
    _tft.drawString(pct, W / 2, 90);

    int barW = W - 60, barH = 22, barX = 30, barY = 140;
    _tft.fillRoundRect(barX, barY, barW, barH, 11, COL_CARD_SHADOW);
    int fillW = (int)((float)barW * value / 255.0f);
    if (fillW > 0) _tft.fillRoundRect(barX, barY, fillW, barH, 11, COL_ACCENT);

    _tft.setTextFont(1);
    _tft.setTextColor(COL_TEXT_MUTED, COL_BG);
    _tft.setTextDatum(BC_DATUM);
    _tft.drawString("Rotate to adjust   -   Press to save", W / 2, _tft.height() - 6);
}

// ---------------------------------------------------------------------
// RINGING
// ---------------------------------------------------------------------
void UIManager::drawRinging(const Alarm &a, uint32_t animMs, bool snoozeHint) {
    // This screen animates every frame, so we always redraw (it's cheap:
    // solid fills + text, no card shadows needed here).
    int W = _tft.width(), H = _tft.height();
    _tft.fillScreen(COL_BG_DARK);

    float phase = (animMs % 1400) / 1400.0f;
    float pulse = 0.85f + 0.15f * sinf(phase * 2.0f * PI);
    int baseR = 62;
    int r = (int)(baseR * pulse);

    _tft.fillCircle(W / 2, 92, r + 10, COL_ACCENT_DARK);
    _tft.fillCircle(W / 2, 92, r, COL_ACCENT);

    _tft.setTextFont(4);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COL_CARD, COL_ACCENT);
    _tft.drawString("ALARM", W / 2, 92);

    _tft.setTextFont(4);
    _tft.setTextColor(COL_CARD, COL_BG_DARK);
    _tft.drawString(a.label, W / 2, 172);

    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", a.hour, a.minute);
    _tft.setTextFont(2);
    _tft.setTextColor(0xAD75, COL_BG_DARK);
    _tft.drawString(timeStr, W / 2, 198);

    _tft.setTextFont(1);
    _tft.setTextColor(0xAD75, COL_BG_DARK);
    _tft.setTextDatum(BC_DATUM);
    if (snoozeHint) {
        _tft.drawString("Turn dial to snooze 9 min   -   Press to dismiss", W / 2, H - 6);
    } else {
        _tft.drawString("Press to dismiss", W / 2, H - 6);
    }
}

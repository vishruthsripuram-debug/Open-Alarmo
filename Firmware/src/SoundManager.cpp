#include "SoundManager.h"
#include "Config.h"

// Small original wake-up jingle (gentle -> escalating), NOT reproducing any
// copyrighted game audio -- just a pleasant original arpeggio that loops
// and gets a little more insistent each pass.
static const Note ALARM_MELODY[] = {
    {523, 160}, {0, 40}, {659, 160}, {0, 40}, {784, 160}, {0, 40}, {1047, 260}, {0, 220},
    {784, 160}, {0, 40}, {988, 160}, {0, 40}, {1175, 160}, {0, 40}, {1568, 260}, {0, 400},
};
static const size_t ALARM_MELODY_LEN = sizeof(ALARM_MELODY) / sizeof(Note);

void SoundManager::begin() {
    ledcSetup(SPEAKER_LEDC_CHANNEL, 2000, SPEAKER_LEDC_RES_BITS);
    ledcAttachPin(SPEAKER_PIN, SPEAKER_LEDC_CHANNEL);
    silence();
}

void SoundManager::tone(uint16_t freq, uint16_t /*ms*/) {
    if (freq == 0) {
        silence();
        return;
    }
    ledcWriteTone(SPEAKER_LEDC_CHANNEL, freq);
}

void SoundManager::silence() {
    ledcWriteTone(SPEAKER_LEDC_CHANNEL, 0);
}

void SoundManager::tickClick() {
    if (_playing) return;               // don't collide with alarm audio
    ledcWriteTone(SPEAKER_LEDC_CHANNEL, 2400);
    delay(3);
    silence();
}

void SoundManager::confirmBeep() {
    if (_playing) return;
    ledcWriteTone(SPEAKER_LEDC_CHANNEL, 1400);
    delay(60);
    silence();
}

void SoundManager::backBeep() {
    if (_playing) return;
    ledcWriteTone(SPEAKER_LEDC_CHANNEL, 500);
    delay(60);
    silence();
}

void SoundManager::startAlarm() {
    _playing      = true;
    _noteIndex     = 0;
    _noteStartMs   = millis();
    _noteSounding  = false;
}

void SoundManager::stopAlarm() {
    _playing = false;
    silence();
}

bool SoundManager::update() {
    if (!_playing) return false;

    uint32_t now = millis();
    const Note &n = ALARM_MELODY[_noteIndex];

    if (!_noteSounding) {
        tone(n.freq, n.ms);
        _noteSounding = true;
        _noteStartMs   = now;
    }

    if (now - _noteStartMs >= n.ms) {
        silence();
        _noteIndex = (_noteIndex + 1) % ALARM_MELODY_LEN;   // loop forever until stopped
        _noteSounding = false;
    }

    return true;
}

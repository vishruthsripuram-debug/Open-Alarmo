#include "RotaryInput.h"
#include "Config.h"

volatile int32_t RotaryInput::_rawSteps  = 0;
volatile uint8_t RotaryInput::_lastState = 0;

// Quadrature transition table: index = (prevAB<<2 | newAB), value = -1/0/+1
static const int8_t QUAD_TABLE[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

void RotaryInput::begin() {
    pinMode(ENC_PIN_A, INPUT_PULLUP);
    pinMode(ENC_PIN_B, INPUT_PULLUP);
    pinMode(ENC_PIN_BTN, ENC_BTN_ACTIVE_LOW ? INPUT_PULLUP : INPUT);

    _lastState = (digitalRead(ENC_PIN_A) << 1) | digitalRead(ENC_PIN_B);

    attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), isrA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_PIN_B), isrB, CHANGE);
}

void IRAM_ATTR RotaryInput::handleEdge() {
    uint8_t a = digitalRead(ENC_PIN_A);
    uint8_t b = digitalRead(ENC_PIN_B);
    uint8_t curr = (a << 1) | b;
    uint8_t idx = (_lastState << 2) | curr;
    _rawSteps += QUAD_TABLE[idx & 0x0F];
    _lastState = curr;
}

void IRAM_ATTR RotaryInput::isrA() { handleEdge(); }
void IRAM_ATTR RotaryInput::isrB() { handleEdge(); }

DialEvent RotaryInput::poll() {
    DialEvent ev{0, false, false};

    // ---- rotation ----
    noInterrupts();
    int32_t steps = _rawSteps;
    interrupts();

    int32_t delta = steps - _lastConsumedSteps;
    int ticks = delta / ENC_DETENTS_PER_CLICK;
    if (ticks != 0) {
        ev.ticks = ticks;
        _lastConsumedSteps += ticks * ENC_DETENTS_PER_CLICK;
    }

    // ---- button (debounced, polled) ----
    bool rawDown = digitalRead(ENC_PIN_BTN) == (ENC_BTN_ACTIVE_LOW ? LOW : HIGH);
    uint32_t now = millis();

    if (rawDown && !_btnLast) {          // just pressed
        _btnDownAt = now;
        _longFired = false;
    } else if (rawDown && _btnLast) {     // held
        if (!_longFired && (now - _btnDownAt) >= ENC_LONG_PRESS_MS) {
            _longFired = true;
            ev.longPressed = true;
        }
    } else if (!rawDown && _btnLast) {    // just released
        if (!_longFired && (now - _btnDownAt) > 15) { // debounce floor
            ev.clicked = true;
        }
    }
    _btnLast = rawDown;

    return ev;
}

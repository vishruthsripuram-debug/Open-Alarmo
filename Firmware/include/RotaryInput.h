#pragma once
#include <Arduino.h>

// Simple event model the UI layer consumes each loop().
struct DialEvent {
    int   ticks;        // +N / -N detents since last poll (0 if none)
    bool  clicked;       // short press released
    bool  longPressed;   // long press fired (once)
};

class RotaryInput {
public:
    void begin();
    // Call every loop(); returns accumulated events since the last call.
    DialEvent poll();

private:
    static void IRAM_ATTR isrA();
    static void IRAM_ATTR isrB();
    static void handleEdge();

    static volatile int32_t _rawSteps;   // updated in ISR
    static volatile uint8_t _lastState;

    // button state machine (handled outside ISR, in poll())
    bool     _btnLast          = false;
    uint32_t _btnDownAt        = 0;
    bool     _longFired        = false;

    int32_t  _lastConsumedSteps = 0;
};

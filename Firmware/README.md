# Bedside Dial Alarm Clock — ESP32 firmware

A bedside alarm clock in the spirit of dedicated "turn the dial" alarm clocks:
big time display, a physical rotary dial on top for everything, and a
speaker for the wake-up chime. Built with an ESP32, a 2.8" SPI TFT
(ILI9341, 240×320), a rotary encoder, and a small speaker.

> Note on sound: the wake-up chime is an original arpeggio I wrote for this
> project, not a reproduction of any commercial device's copyrighted audio.
> Swap `ALARM_MELODY[]` in `src/SoundManager.cpp` for your own tune any time.

---

## 1. Hardware

| Part | Notes |
|---|---|
| ESP32 dev board | Any standard "ESP32-DevKitC"-style board |
| 2.8" SPI TFT, ILI9341 driver, 240×320 | The cheap "2.8 SPI TFT" modules on AliExpress/Amazon are almost all this chip |
| Rotary encoder w/ push button (e.g. EC11) | This is your "dial" — mount it on top of the enclosure |
| Small 8Ω speaker (0.5–1W) + NPN transistor (e.g. 2N2222) + 100–220Ω base resistor | Speaker driven from a PWM pin through a transistor switch |
| Optional: DS3231 RTC breakout | Keeps accurate time through power loss without WiFi. Strongly recommended — see §4 |

### Wiring

**TFT (SPI, ILI9341):**

| TFT pin | ESP32 GPIO |
|---|---|
| VCC | 3V3 |
| GND | GND |
| CS | 15 |
| RESET | 4 |
| DC (RS) | 2 |
| SDI (MOSI) | 23 |
| SCK | 18 |
| SDO (MISO) | 19 |
| LED (backlight) | 27 *(PWM-dimmed)* |

**Rotary encoder (the dial):**

| Encoder pin | ESP32 GPIO |
|---|---|
| A | 33 |
| B | 32 |
| Push button | 26 |
| + / GND | 3V3 / GND |

GPIO 33/32/26 are ordinary GPIOs so the firmware's internal pull-ups work
correctly. Deliberately **not** using GPIO34/35/36/39 for this — those are
input-only pins with no internal pull-up hardware on the ESP32, so
`INPUT_PULLUP` silently fails on them (a common first-build gotcha).

**Speaker (via transistor, simple PWM tone):**

```
GPIO25 --[220Ω]-- Base (2N2222)
Collector -- Speaker(-) 
Emitter -- GND
Speaker(+) -- 3V3
```

**DS3231 RTC (optional but recommended):** SDA → GPIO21, SCL → GPIO22, VCC
→ 3V3, GND → GND.

---

## 2. Software setup

This is a **PlatformIO** project (works in VS Code with the PlatformIO
extension, or via `pio` CLI). Just open the folder — dependencies
(`TFT_eSPI`, `RTClib`) install automatically from `platformio.ini`.

`platformio.ini` pins `platform = espressif32@6.9.0` on purpose — newer
platform releases pull in Arduino-ESP32 core 3.x, which replaced the
`ledcSetup`/`ledcAttachPin`/`ledcWriteTone` calls this firmware uses (for
the backlight PWM and speaker tone) with a different API. If you want to
use a newer core, update those calls in `UIManager::begin()` /
`SoundManager` to the new `ledcAttach(pin, freq, resBits)` /
`ledcWrite(pin, duty)` style first.

If you'd rather use the classic Arduino IDE:
1. Install the `TFT_eSPI` and `RTClib` libraries via Library Manager.
2. Copy the `build_flags` block from `platformio.ini` into your
   `TFT_eSPI/User_Setup.h` as `#define` lines instead (Arduino IDE has no
   `build_flags`, so TFT_eSPI needs its config edited directly in that
   case).
3. Rename `src/main.cpp` to `main.ino` inside a folder of the same name,
   and drop the other `src/*.cpp` + `include/*.h` files alongside it —
   Arduino IDE compiles every `.cpp`/`.h` in the sketch folder together.

### Before you flash

Edit `include/Config.h`:
- `WIFI_SSID` / `WIFI_PASSWORD` — used once at boot to fetch the correct
  time over NTP.
- `GMT_OFFSET_SEC` — currently set to `+10:00` (Sydney standard time).
  Change if you're elsewhere, or if you want DST handled automatically,
  set `DST_OFFSET_SEC` to `3600` during daylight saving.
- `USE_DS3231` — leave at `1` if you wired the RTC module; set to `0` if
  you didn't (the clock will then rely on NTP-synced internal time and
  will lose the correct time on power loss until it reconnects to WiFi).

---

## 3. Using it

**Clock face (home screen):**
- **Turn the dial** → jumps into the Alarms list.
- **Press the dial** → opens the main Menu.

**Main Menu** (New Alarm / Alarms / Brightness / Back):
- Turn to highlight, press to select, hold to go back.

**Alarm list:**
- Turn to highlight an alarm, **press to edit it**, **hold to delete it**.

**Editing an alarm:**
- Turn the dial to change the highlighted field's value (hour → minute →
  repeat → enabled, cycling on each press).
- **Press** moves to the next field.
- **Hold** saves and returns to the list, from any field — so there's no
  separate "Save" button you can accidentally click past.
- The "repeat" field cycles: Once → Everyday → Weekdays → Weekends.

**Brightness screen:** turn to adjust, press or hold to save.

**When an alarm rings:**
- **Turn the dial** (either direction) → snoozes for 9 minutes — this
  mirrors the "twist to snooze" feel of dedicated bedside dial clocks.
- **Press (or hold)** → dismisses it completely.
- Auto-dismisses after 10 minutes if left unattended (see
  `ALARM_RING_TIMEOUT_MIN` in `Config.h`).

Alarms, once set, persist across power loss (stored in flash via
`Preferences`/NVS) — you don't need WiFi or the RTC battery for the
*alarm list* to survive a reboot, only for the *current time* to.

---

## 4. Notes & things you'll likely want to tweak

- **Screen mirrored/rotated wrong?** Change `_tft.setRotation(1)` in
  `UIManager::begin()` (0–3) until it looks right for your enclosure
  orientation.
- **Speaker too quiet/harsh?** `SPEAKER_PIN`'s PWM "tone" through a bare
  transistor is a simple square-wave beeper, not a proper audio amp — for
  actually good speaker quality, swap in a MAX98357A I2S amp module and
  replace `SoundManager`'s `ledcWriteTone` calls with an I2S DAC output.
  The rest of the firmware doesn't care how `SoundManager` makes noise.
- **No DS3231 / don't want to enter WiFi credentials?** The internal
  ESP32 RTC drifts a few seconds a day and has no battery backup — fine
  for a plugged-in clock, but you'll want periodic NTP resync (the
  scaffolding for that is already in `TimeManager::loop()`, just don't
  disconnect WiFi at the end of `TimeManager::begin()` if you want it kept
  alive).
- **Custom per-day repeat schedules** (e.g. "Mon/Wed/Fri only") aren't in
  the UI yet — the data model (`Alarm::daysMask`, one bit per weekday)
  already supports it; `cycleRepeatPreset()` in `main.cpp` only cycles
  through 4 common presets for simplicity. Easy to extend into a
  day-by-day toggle screen if you want it.

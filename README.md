# Env Control — ESP8266 + DHT11 + SSD1306 OLED + IR (Mitsubishi AC)

A tiny environment controller for an ESP8266 that:

- Reads **temperature & humidity** (DHT11)  
- Reads **ambient light** on **A0**  
- Shows live readings on a **128×64 I²C SSD1306 OLED**  
- **Turns a Mitsubishi AC ON/OFF** via IR when temperature crosses thresholds  
- Supports **manual override** from the Serial Monitor (`o` = ON, `f` = OFF)

---

## Table of Contents
- [Hardware](#hardware)
- [Pinout](#pinout)
- [Schematic Notes](#schematic-notes)
- [Libraries](#libraries)
- [Build & Upload](#build--upload)
  - [Arduino IDE](#arduino-ide)
  - [PlatformIO](#platformio)
- [Configuration](#configuration)
- [How It Works](#how-it-works)
- [Using the Sketch](#using-the-sketch)
- [Troubleshooting](#troubleshooting)
- [Extending](#extending)
- [IR Frames (Mitsubishi)](#ir-frames-mitsubishi)
- [License](#license)
- [Checklist](#checklist)

---

## Hardware

- **Board:** ESP8266 (NodeMCU / Wemos D1 mini recommended)
- **Sensors / Peripherals:**
  - DHT11 (temp & humidity)
  - SSD1306 OLED 128×64 (I²C, 3.3 V, address `0x3C`)
  - IR LED **with transistor driver** (NPN + resistor)
  - Light sensor (LDR + divider) to A0 (≤ 1.0 V max at pin on many ESP8266 dev boards)

---

## Pinout

| Function           | Board Label | GPIO |
|--------------------|-------------|------|
| IR Transmit (TX)   | D6          | 12   |
| DHT11 Data         | D4          | 2    |
| I²C SDA (OLED)     | D2          | 4    |
| I²C SCL (OLED)     | D1          | 5    |
| Light Sensor Input | A0          | A0   |

> **OLED address:** `0x3C`  
> **Power:** All peripherals from **3.3 V**.  
> **IR LED:** Drive through a **transistor** for proper current and range.

---

## Schematic Notes

- **IR stage**: GPIO12 → resistor (e.g., 220–330 Ω) → NPN base; IR LED + series resistor on collector to 3.3 V; emitter to GND; add diode only if you drive inductive loads (not needed for LED).  
- **Light sensor**: Voltage divider sized so **A0 ≤ 1.0 V** (check your board’s onboard divider; many scale 3.3 V down to ~1.0 V).  
- **DHT11**: Data pin on D4 with pull-up (most breakout boards include this).

---

## Libraries

Install via Arduino Library Manager (or PlatformIO):

- `IRremoteESP8266` (by David Conran / crankyoldgit)
- `DHT sensor library` (Adafruit)
- `Adafruit Unified Sensor`
- `Adafruit GFX Library`
- `Adafruit SSD1306`

Headers already included in the sketch:
```cpp
#include <Arduino.h>
#include <Wire.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

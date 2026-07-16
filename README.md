# F.G.O Monitor — Fire, Gas & Overheat Monitoring System

A smart safety device built on the ESP32 that monitors **Fire, Gas leaks, and Overheating** in real time, with local alerts (LCD, LEDs, buzzer) and remote monitoring via the Blynk IoT cloud dashboard and mobile notifications.

## Problem Statement

Home and lab environments are exposed to multiple, often simultaneous hazards — gas leaks, fire, and equipment overheating — but most low-cost safety devices only monitor one at a time. This project combines three sensor types with rule-based decision logic to detect and distinguish between fire, gas leaks, and overheating, then alerts locally and remotely in real time.

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller, WiFi + processing |
| MQ-2 Gas Sensor | Detects smoke and gas leaks |
| DHT11 | Measures temperature and humidity |
| IR Flame Sensor | Detects fire via light intensity drop |
| 16x2 I2C LCD | Local status display |
| 2x LEDs (Red/Green) | Visual safe/alert indicator |
| 5V Buzzer | Audible alarm |

## System Logic

The system continuously reads all three sensors and classifies the environment into one of four states:

| Situation | Trigger Condition | Output |
|---|---|---|
| **Fire Alert** | Flame reading < 2000, OR (Gas > 900 AND Temp > 45°C) | Buzzer + Red LED ON + LCD alert + mobile notification |
| **Gas Leak** | Gas > 900 while Temp stays below threshold | Buzzer + Red LED ON + LCD alert + mobile notification |
| **Overheating** | Temp > 55°C | Buzzer + Red LED ON + LCD alert + mobile notification |
| **Safe** | All readings within normal range | Buzzer silent + Green LED ON + LCD shows live readings |

Fire and gas leak are deliberately distinguished by combining flame, gas, and temperature signals together, rather than triggering on any single sensor in isolation — reducing false positives (e.g., high heat alone doesn't necessarily mean fire).

## Cloud Monitoring (Blynk)

- Raw sensor values are mapped to a clean **0–100% scale** for readability.
- **Live gauges** for temperature, humidity, gas level, and flame/IR reading.
- **Push notifications** on the phone when status changes (fire, gas leak, overheating).
- **Historical graphs** to visualize sensor trends over time.

## Setup & Usage

### 1. Hardware Wiring
| Pin | Component |
|---|---|
| GPIO 36 | MQ-2 Analog Out |
| GPIO 18 | DHT11 Data |
| GPIO 4 | Buzzer |
| GPIO 12 | Red LED |
| GPIO 14 | Green LED |
| GPIO 34 | Flame Sensor Analog Out |
| I2C (SDA/SCL) | 16x2 LCD (address 0x27) |

### 2. Software Setup
1. Install the Arduino IDE and add ESP32 board support.
2. Install required libraries: `LiquidCrystal_I2C`, `DHT sensor library`, `Blynk`.
3. In place of secrets.h fill in your own Blynk template ID, auth token, and WiFi credentials:
4. Set up a Blynk template with virtual pins:
   - V0: Temperature, V1: Humidity, V2: Gas %, V3: Flame/IR %, V4: Fire flag, V5: Gas leak flag, V6: Overheat flag
5. Flash `ioT_Semester_Project.ino` to the ESP32 via the Arduino IDE.

## Known Issues

- The IR flame sensor can show abnormal readings in the absence of an actual flame, likely due to ambient IR light interference (e.g., sunlight, incandescent bulbs). Currently being investigated — possible fixes include shielding the sensor or adjusting the detection threshold/hysteresis.

## Tech Stack

- ESP32 (Arduino framework, C++)
- Blynk IoT platform
- DHT, LiquidCrystal_I2C Arduino libraries

## Future Improvements

- Resolve IR flame sensor false-positive issue (ambient light interference).
- Add SMS/email fallback alerts for when the phone is offline.
- Log sensor history locally (SD card) as a backup to cloud storage.
- Add a battery backup with low-power sleep mode for continuous monitoring during power outages.

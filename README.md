# Embedded Linux & Microcontroller Development Projects

A curated repository of embedded Linux drivers, RISC-V SoC firmware, microcontroller applications, and Linux hardware testing tools.

---

## 📁 Repository Directory Structure

```text
EMBERDEDLINUX/
├── aries_v2_led_blink/      # C-DAC ARIES v2.0 (THEJAS32 RISC-V) DIP Switch & LED Controller
├── BLEAttendanceSystem/     # BLE-based automated attendance system
├── CPrograms/               # C algorithms and embedded utility programs
├── ESP32WebGraph/           # ESP32 real-time web-based graph visualization
├── GPS/                     # Linux TTY driver testing & NMEA GPS reading (u-blox M8N / NEO-6M)
├── IMPORTANT_LINUXCOMMENTS/ # Linux system command cheatsheets & references
├── MQ2_ArduinoMega/         # MQ-2 gas & smoke sensor with Arduino Mega
├── ServoControl/            # PWM Servo Motor Control Sketch
└── README.md                # Repository documentation
```

---

## 🚀 Projects Overview

| Directory | Hardware / Platform | Description |
| :--- | :--- | :--- |
| [`aries_v2_led_blink/`](./aries_v2_led_blink) | C-DAC ARIES v2.0 (THEJAS32 SoC, RISC-V) | DIP Switch controlled multi-mode LED system with live Serial monitor output |
| [`BLEAttendanceSystem/`](./BLEAttendanceSystem) | ESP32 / BLE Module | Bluetooth Low Energy attendance scanner and logger |
| [`ESP32WebGraph/`](./ESP32WebGraph) | ESP32 Board | Real-time web server displaying dynamic sensor graphing interface |
| [`GPS/`](./GPS) | Fedora Linux, USB-TTL, u-blox M8N / NEO-6M | Linux TTY serial character device enumeration, `gpsd` and `minicom` setup |
| [`MQ2_ArduinoMega/`](./MQ2_ArduinoMega) | Arduino Mega 2560 + MQ-2 | Gas & smoke detection threshold monitoring |
| [`ServoControl/`](./ServoControl) | Arduino / Microcontroller | PWM servo motor sweep and angle control |
| [`CPrograms/`](./CPrograms) | Embedded C / Linux | Fundamental C algorithms (Fibonacci, odd sum calculation) |
| [`IMPORTANT_LINUXCOMMENTS/`](./IMPORTANT_LINUXCOMMENTS) | Linux System | Essential Linux terminal commands reference guide |

---

## 🛠 Project Details

### 1. C-DAC ARIES v2.0 (RISC-V) DIP Switch & LED Controller
Located in [`aries_v2_led_blink/`](./aries_v2_led_blink)

* **Hardware**: C-DAC ARIES v2.0 board (THEJAS32 SoC, VEGA ET1031 RISC-V core).
* **Core**: VEGA RISC-V Arduino Core (`vega:riscv:aries_v2`).
* **Features**:
  * **DIP SW1 ON (`DIP_SW1 = 16`, `DIP_SW2 = 17`)**: Blinks LED1 (RGB), LED2 (GPIO 21), and LED3 (GPIO 20) **sequentially one after another** (500 ms delay).
  * **DIP SW2 ON**: All LEDs blink **together** with a **2 second (2000 ms)** interval.
  * **Both OFF**: LED 1 (RGB) blinks with a **1000 ms interval**, changing color on every blink cycle (Red $\rightarrow$ Green $\rightarrow$ Blue $\rightarrow$ Cyan $\rightarrow$ Purple $\rightarrow$ Yellow $\rightarrow$ White).
  * **Both ON**: Only LED 1 blinks **RED** (500 ms interval).
  * **Live Telemetry**: Streams real-time state logs over Serial at 115200 baud.

#### Compile & Flash via `arduino-cli`:
```bash
# Compile
arduino-cli compile --fqbn vega:riscv:aries_v2 aries_v2_led_blink

# Flash over serial
arduino-cli upload -p /dev/ttyUSB0 --fqbn vega:riscv:aries_v2 aries_v2_led_blink

# Open Serial Monitor
arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
```

---

### 2. Linux GPS Device Subsystem Testing
Located in [`GPS/`](./GPS)

Explores Linux character device creation (`/dev/ttyUSB0`), kernel TTY driver enumeration, `dmesg` event inspection, user permission setup (`dialout` group), and parsing NMEA strings via `gpsd`, `cgps`, `gpsmon`, and `minicom`.

---

### 3. BLE Attendance System
Located in [`BLEAttendanceSystem/`](./BLEAttendanceSystem)

Utilizes Bluetooth Low Energy (BLE) scanning to record attendance based on BLE beacon / device presence.

---

### 4. ESP32 Web Graph Interface
Located in [`ESP32WebGraph/`](./ESP32WebGraph)

An ESP32 HTTP web server that streams live sensor data to a web browser with interactive graphical visualization.

---

### 5. MQ-2 Gas & Smoke Detection
Located in [`MQ2_ArduinoMega/`](./MQ2_ArduinoMega)

Interfaces an MQ-2 combustible gas and smoke sensor with an Arduino Mega 2560 for real-time safety monitoring.

---

### 6. Servo Motor Control
Located in [`ServoControl/`](./ServoControl)

Demonstrates precise position and angle control of standard servo motors using PWM output pins.

---

## 📝 License
Distributed under open-source licenses specified within individual subprojects.

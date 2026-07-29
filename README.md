# GPS Module Testing on Fedora Linux

This guide explains how Linux detects, communicates with, and reads data from UART-based GPS receivers such as the u-blox M8N and NEO-6M.

---

# Hardware Used

- Fedora Linux
- USB-to-TTL Adapter
- ReadyToSky M8N
- GY-GPS6MV2 (NEO-6M)

---

# Linux Device Detection

Connect the USB-TTL adapter.

Check if Linux detects it.

```bash
ls /dev/ttyUSB*
```

Output

```text
/dev/ttyUSB0
```

## What happens internally?

When the USB-TTL adapter is connected:

USB Device
        │
        ▼
USB Driver (cp210x / ch341 / ftdi_sio)
        │
        ▼
TTY Subsystem
        │
        ▼
Character Device
        │
        ▼
/dev/ttyUSB0
```

Linux creates a **character device** inside `/dev`.

Unlike a disk (`/dev/sda`), a TTY transfers data one byte at a time.

---

# Check USB Enumeration

```bash
lsusb
```

Example

```text
Bus 001 Device 006: ID 10c4:ea60 Silicon Labs CP2102
```

## Binary Level Explanation

Linux asks every USB device for its descriptors.

```
USB RESET
     │
GET_DESCRIPTOR
     │
Vendor ID
Product ID
Class
Endpoints
```

The kernel matches the Vendor ID and Product ID with a driver.

Example

```
VID = 10c4
PID = ea60
```

↓

Driver Loaded

```
cp210x
```

↓

Creates

```
/dev/ttyUSB0
```

---

# Check Kernel Messages

```bash
dmesg | tail
```

Example

```text
cp210x converter now attached to ttyUSB0
```

## Explanation

The USB subsystem notifies the kernel.

The cp210x driver registers itself with the Linux TTY subsystem.

TTY allocates a new serial port.

---

# Check Permissions

```bash
ls -l /dev/ttyUSB0
```

Example

```text
crw-rw---- 1 root dialout ...
```

Meaning

```
c
```

Character device

```
rw-
```

Owner permissions

```
rw-
```

Group permissions

```
---
```

Others

---

# Why dialout?

Check

```bash
groups
```

Example

```text
mrponmudi wheel dialout
```

The serial device belongs to the **dialout** group.

Without it:

```
Permission denied
```

Linux checks

```
User
↓

Group

↓

Permission bits

↓

Access Granted
```

---

# Reading GPS Data

```bash
minicom -D /dev/ttyUSB0 -b 9600
```

Explanation

```
Application
        │
TTY Driver
        │
UART
        │
USB Driver
        │
USB Controller
        │
USB-TTL
        │
GPS Module
```

Every byte received is forwarded to Minicom.

---

# Raw Reading

```bash
cat /dev/ttyUSB0
```

Why does this work?

In Linux

```
Everything is a file.
```

The GPS serial port is simply another file.

```
read()

↓

TTY Driver

↓

USB Driver

↓

GPS
```

---

# NMEA Example

```text
$GPRMC
```

Meaning

```
GP
```

GPS constellation

```
RMC
```

Recommended Minimum Navigation Data

```
V
```

No Fix

or

```
A
```

Valid Fix

---

# gpsd

Install

```bash
sudo dnf install gpsd gpsd-clients
```

Run

```bash
sudo gpsd /dev/ttyUSB0 -F /var/run/gpsd.sock
```

Architecture

```
GPS
 │
 ▼
USB-TTL
 │
 ▼
ttyUSB0
 │
 ▼
gpsd
 │
 ├── cgps
 ├── gpsmon
 ├── Python
 └── Applications
```

gpsd acts as a middleware between hardware and applications.

---

# Live Monitor

```bash
cgps
```

Shows

- Latitude
- Longitude
- Altitude
- Satellites
- HDOP
- UTC Time

---

# Useful Commands

```bash
gpspipe -r
```

Raw NMEA

```bash
gpspipe -w
```

JSON Output

```bash
gpsmon
```

Protocol Analyzer

```bash
cat /dev/ttyUSB0
```

Raw Bytes

---

# Result

## ReadyToSky M8N

✅ 3D Fix

✅ Multiple Satellites

✅ gpsd Working

---

## GY-GPS6MV2

Serial Communication Successful

No Satellite Lock During Test

Requires Further RF Investigation

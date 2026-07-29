# GPS Testing on Fedora Linux

This guide explains how to interface and test GPS modules on Fedora Linux using a USB-to-TTL adapter.

## Hardware Used

- Lenovo LOQ (Fedora Linux)
- USB to TTL (CH340/CP2102)
- ReadyToSky u-blox M8N GPS
- GY-GPS6MV2 (u-blox NEO-6M)

## Wiring

| GPS | USB-TTL |
|------|---------|
| VCC | 5V |
| GND | GND |
| TX | RXD |
| RX | TXD |

## Verify Device

```bash
ls /dev/ttyUSB*
```

Output

```text
/dev/ttyUSB0
```

## Check Permissions

```bash
ls -l /dev/ttyUSB0
groups
```

If your user is not in the dialout group:

```bash
sudo usermod -aG dialout $USER
```

Log out and log in again.

## Install Minicom

```bash
sudo dnf install minicom
```

## Read GPS Data

```bash
minicom -D /dev/ttyUSB0 -b 9600
```

or

```bash
cat /dev/ttyUSB0
```

Example Output

```text
$GPRMC,...
$GPGGA,...
$GPGSV,...
```

## Install gpsd

```bash
sudo dnf install gpsd gpsd-clients
```

Run

```bash
sudo gpsd /dev/ttyUSB0 -F /var/run/gpsd.sock
```

View live GPS

```bash
cgps
```

or

```bash
gpsmon
```

## Results

### ReadyToSky M8N

- Communication successful
- 3D Fix acquired
- Multiple satellites detected
- Tested successfully using cgps

### GY-GPS6MV2

- Serial communication successful
- NMEA output received
- No satellite fix obtained during testing
- Further antenna/RF troubleshooting required

## Useful Commands

```bash
gpspipe -r
gpspipe -w
gpspipe -r > gps_log.txt
```

## Linux Groups

```bash
groups
```

Expected:

```text
mrponmudi wheel dialout
```

The `dialout` group allows access to serial devices such as `/dev/ttyUSB0`.

## Tested On

- Fedora Linux
- USB-TTL Adapter
- u-blox M8N
- u-blox NEO-6M

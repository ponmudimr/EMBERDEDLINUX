# Servo Control using Arduino Mega

Embedded systems project using Arduino Mega and Servo motor.

## Hardware Used
- Arduino Mega 2560
- SG90 Servo Motor
- Jumper wires

## Features
- Servo sweep motion
- Linux terminal programming
- Arduino CLI workflow

## Upload Command

```bash
arduino-cli compile --fqbn arduino:avr:mega .
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:mega .

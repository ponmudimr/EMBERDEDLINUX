# Tried to Program Arduino Using Linux Terminal

## Introduction

This project documents my journey of learning embedded systems programming using Linux terminal tools and an Arduino Mega 2560 board. Instead of using graphical IDE software, the entire workflow was done through terminal commands using Arduino CLI, Git, and GitHub.

The purpose of this project is to understand how professional embedded engineers develop firmware using Linux-based environments.

---

# Project Goals

* Learn Arduino programming using Linux terminal
* Understand Arduino CLI workflow
* Learn Git and GitHub for embedded development
* Control a servo motor using Arduino Mega
* Organize embedded project files professionally
* Practice firmware upload through command line
* Build confidence in Linux-based embedded systems development

---

# Hardware Used

## Arduino Mega 2560

The Arduino Mega 2560 is a microcontroller board based on the ATmega2560.

Features:

* ATmega2560 microcontroller
* 54 digital I/O pins
* 16 analog inputs
* 4 hardware serial ports
* USB programming support
* Large flash memory
* Suitable for embedded systems projects

---

## Servo Motor

A servo motor is a rotary actuator that allows precise control of angular position.

This project uses a small servo motor for testing motion control.

Servo wires:

* Brown or Black → Ground
* Red → VCC
* Orange or Yellow → Signal

---

# Linux Environment

The project was developed using Linux terminal commands.

Benefits of Linux for embedded development:

* Fast workflow
* Better control
* Professional environment
* Powerful command line tools
* Git integration
* Automation support
* Lightweight development setup

---

# Installing Arduino CLI

Arduino CLI is a command-line tool used for compiling and uploading Arduino programs.

## Installation Steps

```bash
sudo apt update
sudo apt install curl -y
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
sudo mv bin/arduino-cli /usr/local/bin/
```

---

# Checking Arduino CLI Version

```bash
arduino-cli version
```

---

# Installing Arduino AVR Core

```bash
arduino-cli core update-index
arduino-cli core install arduino:avr
```

---

# Detecting Arduino Board

```bash
arduino-cli board list
```

Detected port:

```text
/dev/ttyUSB0
```

---

# Creating Project Folder

```bash
mkdir ServoControl
cd ServoControl
```

---

# Creating Arduino Sketch

```bash
nano ServoControl.ino
```

---

# Servo Motor Program

```cpp
#include <Servo.h>

Servo myservo;

void setup() {
  myservo.attach(9);
}

void loop() {
  myservo.write(0);
  delay(1000);

  myservo.write(90);
  delay(1000);

  myservo.write(180);
  delay(1000);
}
```

---

# Compiling the Program

```bash
arduino-cli compile --fqbn arduino:avr:mega .
```

---

# Uploading the Program

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:mega .
```

---

# Servo Wiring

| Servo Wire    | Arduino Connection |
| ------------- | ------------------ |
| Brown/Black   | GND                |
| Red           | 5V                 |
| Orange/Yellow | Pin 9              |

---

# Git Setup

Git was used for version control.

## Initialize Repository

```bash
git init
```

## Add Files

```bash
git add .
```

## Commit Changes

```bash
git commit -m "Initial embedded servo project"
```

---

# GitHub Setup

A GitHub repository was created to store and manage the project online.

Repository Name:

```text
EMBERDEDLINUX
```

---

# Adding GitHub Remote

```bash
git remote add origin https://github.com/ponmudimr/EMBERDEDLINUX.git
```

---

# Pushing to GitHub

```bash
git branch -M main
git push -u origin main
```

---

# Learning Outcomes

This project helped in understanding:

* Linux terminal workflow
* Embedded systems basics
* Servo motor control
* Git and GitHub workflow
* Arduino CLI usage
* Hardware interfacing
* Firmware compilation
* Command line development

---

# Problems Faced

## Servo Library Missing

Error:

```text
Servo.h: No such file or directory
```

Solution:

```bash
arduino-cli lib install Servo
```

---

## GitHub Authentication Issues

GitHub password authentication is deprecated.

Solution:

* Use Personal Access Token
* Use SSH authentication

---

# Future Improvements

Future plans for this project include:

* Sensor integration
* OLED display support
* WiFi communication
* IoT dashboard
* PCB design
* Relay control
* Home automation
* Embedded Linux integration
* FreeRTOS support
* Advanced robotics projects

---

# Embedded Systems Knowledge Gained

This project introduced several embedded systems concepts:

* Microcontrollers
* GPIO control
* PWM signal generation
* Serial communication
* Firmware upload process
* Hardware debugging
* Linux terminal commands
* Version control systems

---

# Why Linux for Embedded Systems

Linux is widely used in embedded engineering because:

* Strong terminal tools
* Efficient workflow
* Automation capabilities
* Open-source ecosystem
* Easy package management
* Professional development environment

---

# Conclusion

This project represents an important step in learning embedded systems development using Linux terminal tools.

Programming Arduino using Linux terminal commands provides better understanding of how embedded software development works in real engineering environments.

The project also helped in learning Git, GitHub, Arduino CLI, hardware interfacing, and firmware management.

This experience forms a strong foundation for future embedded systems and IoT projects.

---

# Author

Ponmudi M.R.

Electronics and Communication Engineering Student

Learning Embedded Systems and Linux-based Development.


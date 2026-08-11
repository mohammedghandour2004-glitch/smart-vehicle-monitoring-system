# Smart Vehicle Monitoring System

#### Video Demo: https://youtu.be/PUT_YOUR_VIDEO_LINK_HERE

## Description

Smart Vehicle Monitoring System is my CS50x Final Project. It is an embedded systems application developed using an Arduino Mega 2560 and simulated in Wokwi. The purpose of this project is to monitor important vehicle parameters in real time and alert the driver whenever a dangerous condition is detected.

The system continuously monitors:

- Engine Temperature
- Fuel Level
- Parking Distance

Based on these values, the system determines the condition of each subsystem and calculates an overall vehicle status.

The project also provides visual and audible feedback using LEDs, a buzzer, and a 16x2 I2C LCD display.

---

# Features

- Real-time engine temperature monitoring.
- Real-time fuel level monitoring.
- Real-time parking distance measurement using an HC-SR04 ultrasonic sensor.
- Automatic status evaluation.
- Green, yellow, and red warning LEDs.
- Audible danger alarm using a buzzer.
- Four LCD information pages.
- Push button for page navigation.
- Debounced button handling.
- Non-blocking timing using millis().
- Serial Monitor debugging output.

---

# Hardware Components

- Arduino Mega 2560
- HC-SR04 Ultrasonic Sensor
- NTC Temperature Sensor
- Potentiometer (Fuel Level Simulation)
- 16x2 LCD with I2C Module
- Push Button
- Green LED
- Yellow LED
- Red LED
- Passive Buzzer

---

# Project Structure

The project consists of the following files:

- sketch.ino
- diagram.json
- libraries.txt
- wokwi-project
- README.md

---

# System Logic

The program periodically reads all sensors.

The engine temperature is converted from the NTC analog value into degrees Celsius using the Beta equation.

The fuel level is simulated using a potentiometer and converted into a percentage.

The ultrasonic sensor measures the parking distance in centimeters.

Each subsystem is classified into one of three states:

- NORMAL
- WARNING
- DANGER

The system then calculates the overall vehicle status by selecting the most critical condition.

---

# LCD Pages

The LCD contains four pages.

Page 1
- Engine Temperature

Page 2
- Fuel Level

Page 3
- Parking Distance

Page 4
- Overall System Status

The user switches between pages using the push button.

---

# LEDs

The LEDs represent the overall status of the vehicle.

Green LED
- NORMAL

Yellow LED
- WARNING

Red LED
- DANGER

Only one LED is active at a time.

---

# Buzzer

The buzzer is activated only during the DANGER state.

Instead of using delay(), the buzzer uses millis() to generate a pulsed alarm pattern. This allows the entire system to continue running without blocking sensor readings or LCD updates.

---

# Why I Used millis()

One of the most important design decisions in this project was replacing delay() with millis().

Using delay() would stop the entire program while waiting, making the LCD, button, LEDs, and sensors temporarily unresponsive.

Using millis() allows all parts of the system to work simultaneously.

---

# Button Debouncing

Mechanical push buttons generate multiple unwanted transitions when pressed.

To avoid false page changes, the program implements software debouncing using millis().

Only stable button presses are accepted.

---

# Design Decisions

Several design decisions were made during development.

Instead of writing one long loop(), I separated the project into small functions.

This makes the code easier to read, maintain, and expand.

Status evaluation is divided into three independent modules:

- Engine
- Fuel
- Parking

The overall system status is calculated independently from the sensor-reading functions.

The LCD is refreshed only when necessary, reducing unnecessary writes.

The buzzer uses non-blocking timing.

The LEDs always represent the highest-priority warning.

---

# Challenges

The biggest challenges were:

- Designing a non-blocking program.
- Organizing the project into reusable functions.
- Managing multiple sensors simultaneously.
- Synchronizing the LCD, LEDs, buzzer, and button.
- Implementing reliable button debouncing.

---

# Future Improvements

Possible future improvements include:

- CAN Bus communication
- OBD-II support
- Bluetooth connectivity
- Mobile application
- SD card logging
- GPS integration
- ECU communication
- STM32 implementation
- FreeRTOS support
- AUTOSAR concepts

---

# AI Usage

Artificial intelligence tools were used during this project for:

- Explaining programming concepts.
- Debugging code.
- Reviewing code quality.
- Improving documentation.
- Suggesting software architecture.

The project was personally implemented, tested, modified, and fully understood by the author.

---

# Author

Mohammed Ghandour

Specialized Technician in Diagnostic and Embedded Electronics

OFPPT – Sale, Morocco

- GitHub: https://github.com/mohammedghandour2004-glitch

- LinkedIn: https://www.linkedin.com/in/mohammed-ghandour-auto/
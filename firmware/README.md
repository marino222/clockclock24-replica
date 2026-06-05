# Firmware

This directory contains the source code for the project, targeting both the Raspberry Pi Pico and the ESP32 microcontrollers.

## Overview

- **Master**: Runs on the ESP32.
- **Slave**: Runs on the Raspberry Pi Picos.

The firmware is largely based on the code in [Vallasc's clockclock24-replica repository](https://github.com/Vallasc/clockclock24-replica). Therefore, all credits go to [Vallasc](https://github.com/Vallasc).

## Architecture Details

### Master (ESP32)
The Master acts as the central coordinator for the entire clock and handles the following high-level tasks:
- **Web Server & UI**: Hosts an asynchronous web server that serves the control panel UI. The ESP32 also acts as a DNS server when in Hotspot (AP) mode to redirect users to the interface.
- **Timekeeping**: Connects to the internet via WiFi to synchronize time using an NTP client. It manages timezones and daylight saving time adjustments automatically.
- **Clock Logic & Animations**: Contains the logic defining the target angles for every single clock hand to display numbers, custom patterns, or complex choreography.
- **I2C Master**: The ESP32 sequentially communicates with the 8 slave boards via I2C, sending them their new target motor positions so that all 48 hands move in unison.

### Slave (Raspberry Pi Pico)
The ClockClock 24 uses 8 Slave boards. Each Pico manages 3 custom stepper motor modules, each equipped with a double-shaft stepper motor (totaling 6 hands per slave).
- **I2C Slave**: Listens on its specific I2C address (0x08 to 0x0F) for position commands from the Master. The I2C address is dynamically set at boot based on the board's physical DIP switches.
- **Motor Control**: Uses the `AccelStepper` library to concurrently drive the 6 connected hands. The slave independently manages the acceleration, deceleration, and step timing for each motor, freeing the Master from real-time motor control.

### Web UI Build Process
To optimize flash storage usage and maximize loading speeds on the ESP32, the Web UI is not stored as distinct files on a filesystem (like SPIFFS/LittleFS).
Instead, a custom Node.js script (`platformio/master/web/minimize.js`) is used to bundle the interface. By running `npm run minimize` in the `web` directory, the script:
1. Inlines all external CSS and JavaScript directly into the HTML file.
2. Minifies the resulting code by stripping comments and whitespace.
3. Generates C++ header files (`web_page.h` and `ota_page.h`) containing the fully optimized UI as static C string macros.

These headers are then compiled directly into the ESP32 firmware, allowing the web server to serve them directly from memory.

## Project Structure

### PlatformIO Projects
The main firmware is developed using PlatformIO and contains only the final firmware.

- **`master/`**: Source code for the ESP32 (Master). Includes the web UI source code inside the `master/web/` directory.
- **`slave/`**: Source code for the Raspberry Pi Picos (Slaves).

### Arduino Scripts
- **`arduino/`**: Contains Arduino scripts used for testing purposes. These scripts were developed and tested using the Arduino IDE. Arduino test scripts are faster and easier to set up, avoiding the hassle of configuring PlatformIO projects for simple tests. The Arduino IDE can be used for these scripts because the PlatformIO project uses the same Arduino framework, ensuring compatibility.
- For detailed explanations about the Arduino test scripts, refer to the [Arduino README](arduino/README.md).

# Firmware

This directory contains the source code for the project, targeting both the Raspberry Pi Pico and the ESP32 microcontrollers.

## Overview

- **Master**: Runs on the ESP32.
- **Slave**: Runs on the Raspberry Pi Picos.

The firmware is largely based on the code in [Vallasc's clockclock24-replica repository](https://github.com/Vallasc/clockclock24-replica). Therefore, all credits go to [Vallasc](https://github.com/Vallasc).

## Project Structure

### PlatformIO Projects
The main firmware is developed using PlatformIO and contains only the final firmware.

- **`master/`**: Source code for the ESP32 (Master).
- **`slave/`**: Source code for the Raspberry Pi Picos (Slaves).

### Arduino Scripts
- **`arduino/`**: Contains Arduino scripts used for testing purposes. These scripts were developed and tested using the Arduino IDE. The Arduino IDE can be used for these scripts because the PlatformIO project uses the same Arduino framework, ensuring compatibility.
- For detailed explanations about the Arduino test scripts, refer to the [Arduino README](arduino/README.md).



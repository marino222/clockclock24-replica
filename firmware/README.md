# Firmware

This directory contains the source code for the project, targeting both the Raspberry Pi Pico and the ESP32 microcontrollers.

## Overview

- **Master**: Runs on the ESP32.
- **Slave**: Runs on the Raspberry Pi Picos.

The firmware is largely based on the code in [Vallasc's clockclock24-replica repository](https://github.com/Vallasc/clockclock24-replica). Therefore, all credits go to [Vallasc](https://github.com/Vallasc).

## Project Structure

### PlatformIO Projects
The main firmware is developed using PlatformIO.

- **`master/`**: Source code for the ESP32 (Master).
- **`slave/`**: Source code for the Raspberry Pi Picos (Slaves).
- **`test/`**: A separate PlatformIO project designed for debugging individual PCBs.

### Arduino Scripts
- **`arduino/`**: Contains basic Arduino scripts used for initial testing of functionalities. These scripts were developed and tested using the Arduino IDE. Since the PlatformIO projects also utilize the Arduino framework, the core logic remains similar. These were created to avoid the overhead of full PlatformIO projects during early testing.

## Testing & Debugging

The `test/` project is specifically designed to debug a single PCB using a separate Raspberry Pi Pico.

### How to Test a Board
1. **Setup**: The PCB is designed with header pins, allowing the Pico to be easily removed and replaced.
2. **Procedure**: Replace the regular Pico on the board with one running the test firmware.
3. **Functionality**:
    - The test firmware reads the preset BIT address.
    - It runs the corresponding stepper motor based on that address.
    - You can change the BIT address to test different stepper motors as needed.
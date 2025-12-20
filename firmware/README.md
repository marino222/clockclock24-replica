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
- **`arduino/`**: Contains Arduino scripts used for testing purposes. These scripts were developed and tested using the Arduino IDE. They include a test script for the Speedo Drive Motor Controller and a script for testing a single board with BIT address reading.
- The Arduino IDE can be used for these scripts because the PlatformIO project uses the same Arduino framework, ensuring compatibility.

## Testing & Debugging

### Speedo Drive Motor Controller Test
- A dedicated Arduino test script is available to test the Speedo Drive Motor Controller.
- This script allows testing of a single BKA30D-R5 stepper motor.
- A Raspberry Pi Pico running the Speedo Drive Test script needs to be wired to the Speedo Drive.
- Each controller can control two stepper motors.

### Single Board Test with BIT Address Reading
- The Arduino test folder contains a script specifically designed for testing a single board.
- This script reads the BIT address of the board and runs the corresponding stepper motor based on that address.
- By changing the BIT address, different stepper motors can be tested.

### How to Test a Board
1. **Setup**: The PCB is designed with header pins, allowing the Pico to be easily removed and replaced.
2. **Procedure**: Replace the regular Pico on the board with one running the test firmware.
3. **Functionality**:
    - The test firmware reads the preset BIT address.
    - It runs the corresponding stepper motor based on that address.
    - You can change the BIT address to test different stepper motors as needed.
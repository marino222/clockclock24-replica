# ClockClock 24 Replica

## General Description
ClockClock 24 Replica is an open-source project inspired by the kinetic art clock "ClockClock 24" by *Humans Since 1982*. This project aims to replicate the mesmerizing motion of 24 analog clocks working together to display digital time. It combines mechanical design, electronics, and firmware to create a functional and visually stunning piece of art.

This repository is based on the work of Vallasc's ClockClock24 Replica and includes modifications and improvements to the original design. Special thanks to Vallasc and other contributors for their efforts in making this project possible.

---

## Table of Contents
1. [Overview](#overview)
2. [Repository Structure](#repository-structure)
3. [3D Models](#3d-models)
4. [Firmware](#firmware)
5. [PCB Design](#pcb-design)
6. [Getting Started](#getting-started)
7. [Acknowledgments](#acknowledgments)
8. [License](#license)
9. [Additional Resources](#additional-resources)

---

## Overview
- **Project Goal**: To replicate the ClockClock 24 kinetic art clock.
- **Key Components**:
  - 3D-printed mechanical parts.
  - Custom PCBs for motor control and clock synchronization.
  - Firmware for ESP32 (Master) and Raspberry Pi Pico (Slaves).

---

## Repository Structure
The repository is organized as follows:

```
ClockClock 24 Replica/
├── 3D files/
│   ├── final/
│   └── test/
├── docs/
│   ├── datasheets/
│   └── images/
├── firmware/
│   ├── arduino/
│   │   ├── AdvancedBoardTest/
│   │   ├── BoardTest/
│   │   └── SpeedoDriveTest/
│   ├── platformio/
│   │   ├── master/
│   │   └── slave/
│   └── README.md
├── pcb/
│   ├── Clock PCB/
│   └── Motor Driver/
└── README.md
```

## 3D Models
- Contains 3D models for the mechanical components.
- Includes final designs and test files for prototyping.

## Firmware
- **Master**: Code for the ESP32 to manage time synchronization and communication.
- **Slave**: Code for the Raspberry Pi Picos to control individual clock motors.
- **Arduino**: Test scripts for debugging and hardware validation.

## PCB Design
- **Clock PCB**: Modified version of Vallasc's design.
- **Motor Driver PCB**: Separate design for improved performance and easier assembly.
- Includes manufacturing files and instructions.

## Getting Started
1. **3D Printing**: Print the mechanical components using the provided files.
2. **PCB Manufacturing**: Manufacture the PCBs using the provided design files.
3. **Firmware**: Flash the firmware onto the ESP32 and Raspberry Pi Picos.
4. **Assembly**: Assemble the components and test the system.

## Acknowledgments
- **Vallasc**: For the original ClockClock24 Replica design.
- **Humans Since 1982**: For the inspiration behind this project.
- **Contributors**: To all those who contributed to the development of this project.

## License
This project is licensed under the GNU General Public License (GPL v3).

## Additional Resources
- Links to datasheets, libraries, and related projects.

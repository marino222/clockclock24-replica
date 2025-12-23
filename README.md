# ClockClock 24 Replica

This project is a hardware and software replica of the kinetic art piece "ClockClock 24" by *Humans Since 1982*, inspired by and building upon the work of [Vallasc](https://github.com/Vallasc/clockclock24-replica) with several hardware modifications and improvements. The clock features 24 double-shaft stepper motors arranged in an 8x3 matrix, with motors grouped in sets of three on custom PCBs, of which there are eight in total. Each board is controlled by a Raspberry Pi Pico microcontroller acting as a slave, while all slave boards communicate with a central ESP32 master via the I2C protocol. The ESP32 also provides a web interface for remote control and configuration.

---

## Table of Contents
1. [Repository Structure](#repository-structure)
2. [Hardware](#hardware)
    1. [Electronics](#electronics)
        1. [Motors](#motors)
        2. [Motor Drivers](#motor-drivers)
        3. [Microcontrollers](#microcontrollers)
        4. [PCB](#pcbs)
        5. [Power Supply](#power-supply)
    2. [Mechanics](#mechanics)
        1. [Clock hands](#clock-hands)
        2. [Fixation brackets](#fixation-brackets)
        3. [Front panel](#front-panel)
    3. [BOM](#bom)
3. [Software](#software)
    1. [Master](#master)
    2. [Slave](#slave)
    3. [Web Interface](#web-interface)
4. [How to get started](#get-started)
5. [Credits](#credits)

---

## Repository Structure

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

---

## Hardware

### Electronics

#### Motors
To drive the clock hands, the BKA30D-R5 (equivalent to VID28-05) stepper motors were chosen. They have a total of 4 coils, allowing for two concentric drive shafts which makes them perfect for this application. These motors were originally designed for dashboard applications in the automotive industry. There are a number of other options with concentric shafts, but this one seemed to be most suitable as other similar projects also used it. Furthermore, it is cheaply available (around 4$ on Aliexpress) and draws little current.


<p align="center">
    <img src="docs/images/bka30d-r5.jpg" alt="BKA30D-R5 stepper motor" width="250" />
    <br><em>BKA30D-R5 stepper motor</em>
</p>

The BKA30D-R5 allows for microstepping up to 1/12 degrees per microstep, which is ideal to achieve smooth movement of the clock hands. See datasheet for more details: [BKA30D-R5 Datasheet](docs/datasheets/BKA30D-xx_datasheet.pdf).

*Note: As standard, this motor comes with mechanical endstops. On Aliexpress there are some vendors that sell it without endstops, otherwise you will need to cut off the endstops yourself.*

---

#### Motor Drivers
The steppers could in theory be driven directly from the microcontroller. However, the GPIO pins are limited and it is usually best practice to use a dedicated motor driver. For this reason, the VID6606 motor driver was chosen. It is specifically designed for the BKA30D-R5 (VID28-05) motor. One VID6606 can drive two motors (4 coils) in total. Per motor, it only needs two inputs: `f(SCX)` and `DIR`. As standard, this controller has built-in microstepping, which means with each rising edge of the SCX input the motor will perform one microstep in the direction specified by the DIR input.

The [VID6606 Datasheet](docs/datasheets/VID6606_datasheet.pdf) specifically recommends the use of smoothing capacitors to reduce electrical noise. Therefore, this [Motor-Controller-PCB from Hackaday](https://hackaday.io/project/187630-stepper-driver-for-dashpanel-instruments) was used. I didn't make any changes to it because it was already tested by [doctek](https://hackaday.io/doctek). Thus, it is designed to be used as a breakout board on the custom Clock PCB.

![Rendered Motor Driver PCB](docs/images/motor-driver_render.png)
<p align="center"><em>Rendered image of the motor driver PCB</em></p>

During testing, I noticed the steppers vibrated quite heavily during movement, leading to increased noise levels. Since a noisy clock is unpleasant, I wanted to reduce the noise. I initially thought the vibrations were caused by the motor drivers and tested multiple alternatives (DRV8834, TMC2208), but it didn't seem to have any major impact. So I stuck with the VID6606 drivers. I found out the vibrations come from substantial mechanical play in the motor shafts, so I addressed this issue during the mechanical design. See [Mechanics](#mechanics).

---

#### Microcontrollers
An ESP32 was chosen to run the master code, while multiple Raspberry Pi Picos are used as slave controllers. The ESP32 sends all the commands via I2C protocol to the slave boards. The main reason for this choice is that almost the same controllers were used in the reference project ([Vallasc/clockclock24-replica](https://github.com/Vallasc/clockclock24-replica)). Since I want to copy the software as closely as possible, I stuck with the same controllers. They are also widely available for a reasonable price and are easy to work with.

<p align="center">
    <img src="docs/images/raspberry-pi-pico.png" alt="Raspberry Pi Pico" width="200" />
    <img src="docs/images/esp32.jpg" alt="ESP32" width="200" />
</p>
<p align="center"><em>Raspberry Pi Pico (left) and ESP32 (right)</em></p>

---

#### PCB
For the project, a custom Clock PCB was designed to hold three BKA30D-R5 motors, which is the maximum number of motors that can be driven by one Raspberry Pi Pico regarding available GPIO pins. In total, there are eight of these PCBs, resulting in an 8x3 matrix of 24 motors. Each Clock PCB has two VID6606 motor driver breakout boards, of which one output is unused.

As a design choice, the PCB uses header pins to connect the motor drivers and the microcontroller. This makes it easy to replace components in case they break. While testing, this happened multiple times. Since the motor controllers were directly soldered onto the PCB in the beginning, I ended up throwing away the whole PCB each time a motor driver broke. The header pins solve this problem. As a bonus, the Picos can be switched out easily. This proved useful to quickly run a test program by replacing the "normal" slave controller with a test controller running different firmware.

---

#### Power Supply
All the PCBs are connected by a JST-XH 4-pin connector in a daisy chain. Two wires are used for power (5V and ground) and two wires are used for I2C communication (SDA and SCL). In theory, it should be possible to power all eight PCBs from one Raspberry Pi Pico connected to a power supply via USB-C.



##### Power Consumption Table

| Device                | Normal Current (mA) | Peak Current (mA) | Quantity | Total Normal (mA) | Total Peak (mA) |
|-----------------------|---------------------|-------------------|----------|-------------------|-----------------|
| Raspberry Pi Pico     | 50                  | 100               | 8        | 400               | 800             |
| Stepper Motor         | 20                  | 20                | 24       | 480               | 480             |
| ESP32                 | 260                 | 700               | 1        | 260               | 700             |
| **Total**             |                     |                   |          | **1140**          | **1980**        |

This table summarizes the estimated current draw for each device during normal operation and at peak. The total values are calculated for 8 Raspberry Pi Picos, 24 stepper motors, and 1 ESP32.

A Raspberry Pi Pico can provide up to 2A of current from the VBUS pin when powered via USB-C. It shows that even at peak times, the total current draw should be below 2A.

Even though the power should be enough, the PCBs are designed to have an additional power input via a 2-pin JST connector. If there is a power issue, each PCB can be powered individually by a proper 5V power supply.

### Mechanics

#### Clock hands

---
#### Fixation brackets

---
#### Front panel

---
### BOM

---

## Software

### Master

### Slave

### Web Interface

---

## How to get started

---

## Credits


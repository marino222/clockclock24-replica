# PCB System Overview

This directory contains all hardware design files for the custom PCBs used in the ClockClock 24 Replica project. The electronics are split into two main boards:

- **Clock PCB:** Hosts three stepper motors, a Raspberry Pi Pico, and connectors for two Motor Driver modules.
- **Motor Driver:** A breakout board for the VID6606 stepper driver IC, designed to be easily replaceable and compatible with the Clock PCB.

## Directory Guide

- `Clock PCB/` — KiCad design files, manufacturing files, and a detailed BOM for the main clock board.
- `Motor Driver/` — KiCad design files, manufacturing files, and a detailed BOM for the motor driver breakout board.

## Design Philosophy

The modular approach allows for easy replacement of failed components (especially motor drivers and microcontrollers) without desoldering. Header pins are used for all major modules. This also enables quick swapping of test boards during development.

## Bill of Materials (BOM) Structure

**Main Project BOM:**
- The global BOM in the main project [`README.md`](../README.md) lists only the major assemblies (e.g., "8x Clock PCB", "16x Motor Driver Modules") and other high-level components. It does not enumerate every resistor or capacitor.

**Subfolder BOMs:**
- Each PCB subfolder contains a detailed BOM listing all electronic components required to assemble that board. Use these when ordering or assembling the PCBs.

**Workflow:**
1. Review the main BOM to estimate project cost and gather major parts.
2. When ready to manufacture a PCB, consult the corresponding subfolder README and BOM for the full component list and assembly instructions.

---

For more details on each board, see the respective subfolder README files.
# PCB Design Files

This folder contains the PCB design files for the Clock PCB and a dedicated PCB for the Stepper Motor Controller. Below is an overview of the designs, modifications, and acknowledgments.

## Overview

### Clock PCB
The Clock PCB is based on the design by [Vallasc](https://github.com/Vallasc) from the [clockclock24-replica repository](https://github.com/Vallasc/clockclock24-replica). The original design was a great starting point, but I made several changes to make it work better and be easier to use.

### Motor Controller PCB
A separate Motor Controller PCB was developed to address challenges encountered during testing and assembly. This design is inspired by the project [Stepper Driver for Dashpanel Instruments](https://hackaday.io/project/187630-stepper-driver-for-dashpanel-instruments) by [Doctek](https://hackaday.io/doctek), found on Hackaday.

## Key Modifications

1. **Separate Motor Controller PCB**
   - The original design integrated the VID6606 controller chips directly onto the Clock PCB. However, soldering these chips proved difficult, resulting in damaged boards during assembly.
   - To resolve this, the Motor Controller was redesigned as a separate PCB with header pins, enabling easy replacement of both the Motor Controller and the Raspberry Pi Pico without the need for desoldering.

2. **Enhanced Motor Controller Design**
   - The Motor Controller PCB now includes all the capacitors and resistors recommended in the official datasheet for the BKA30D-R5 stepper motors, ensuring smoother motor operation.
   - The original design by [Vallasc](https://github.com/Vallasc) omitted decoupling capacitors, which have been added to the new Motor Controller PCB for improved performance.
   - For further details, refer to the [BKA30D-R5 Datasheet](../docs/datasheets/BKA30D-xx_datasheet.pdf).

3. **Removed ESP32 Mounting Positions**
   - The mounting positions for the ESP32 were removed to create space for the Motor Drivers.

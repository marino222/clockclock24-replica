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

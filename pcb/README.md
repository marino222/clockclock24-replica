# PCB Design Files

This folder contains the PCB design files for the Clock PCB and a smaller PCB for the Stepper Motor Controller. Below is an overview of the design, modifications, and credits.

## Overview

### Clock PCB
The Clock PCB is based on the design by [Vallasc](https://github.com/Vallasc) in their [clockclock24-replica repository](https://github.com/Vallasc/clockclock24-replica). While the original design served as a foundation, several modifications were made to improve functionality and usability.

### Motor Controller PCB
A separate Motor Controller PCB was introduced to address challenges encountered during testing and assembly. This design is based on this Desing I found on Hackaday: [Stepper Driver for Dashpanel Instruments](https://hackaday.io/project/187630-stepper-driver-for-dashpanel-instruments) by [Doctek](https://hackaday.io/doctek).

## Key Modifications

1. **Separate Motor Controller PCB**
   - The original design integrated the VID6606 controller chips directly onto the Clock PCB. However, soldering these chips proved challenging, leading to damaged boards during assembly.
   - To mitigate this, the Motor Controller was designed as a separate PCB with header pins, allowing for easy replacement of both the Motor Controller and the Raspberry Pi Pico without desoldering.

2. **Improved Motor Controller Design**
   - The Motor Controller PCB includes all the capacitors and resistors recommended in the official datasheet for the BKA30D-R5 stepper motors. This ensures smoother motor operation.
   - The original design by [Vallasc](https://github.com/Vallasc) lacked decoupling capacitors, which are now included in the Motor Controller PCB.

3. **Removed ESP32 Mounting Positions**
   - The mounting positions for the ESP32 were removed to make space for the Motor Drivers.

## Credits

- **Clock PCB Design**: [Vallasc](https://github.com/Vallasc)
- **Motor Controller PCB Design**: [Doctek](https://hackaday.io/doctek)

These modifications aim to enhance the reliability and maintainability of the PCB designs. The separation of the Motor Controller PCB simplifies assembly and ensures compliance with datasheet recommendations for smoother motor performance.

# Motor Driver PCB — VID6606 Breakout

This folder contains the design files, manufacturing resources, and assembly instructions for the Motor Driver PCB used in the ClockClock 24 Replica project. This board is a compact breakout for the VID6606 stepper driver IC, designed to be easily replaceable and compatible with the main Clock PCB.

---

## Folder Contents

- **KiCad Design Files:** Schematic and layout for the Motor Driver PCB.
- **Manufacturing Folder:**
  - Gerber files (ZIP) for PCB fabrication
  - [SpeedoDrive_BOM_SMD.csv](Manufacturing/SpeedoDrive_BOM_SMD.csv): Machine-readable BOM for SMD assembly

---

## Manufacturing 

1. **PCB Fabrication:**
   - The PCB was ordered from [JLCPCB](https://jlcpcb.com/)
   - Upload the Gerber ZIP file from the `Manufacturing` folder when placing your order

2. **SMD Assembly:**
    - The automatic assembly of SMD components (excluding the VID6606) was also ordered from JLCPCB.
    - During the order proces you will be prompted to upload a BOM file, to automatically populate the pcb. Use the CSV file in the `Manufacturing` folder for this purpose
    - Of course, the SMD Components can also be soldered manually if preferred.

3. **Manual Assembly:**
    - Since the VID6606 chip and the pin headers cannot be automatically assembled, these components must be soldered manually after receiving the PCB.

---

## Bill of Materials (BOM)

This BOM lists all components required to assemble one Motor Driver board.

| Quantity | Reference   | Value   | Footprint | Description           |
|----------|-------------|---------|-----------|-----------------------|
| 1        | U1          | VID6606 | —         | Stepper Driver IC     |
| 2        | C1, C6      | 6.8 µF  | 0805      | Ceramic Capacitor     |
| 2        | C2, C5      | 4.7 µF  | 0805      | Ceramic Capacitor     |
| 2        | C3, C4      | 100 nF  | 0603      | Ceramic Capacitor     |
| 9        | R1–R9       | 10 kΩ   | 0603      | Resistor              |
| 1        | J1          | Header 5P | 2.54 mm | Pin Header            |
| 1        | J2          | Header 8P | 2.54 mm | Pin Header            |
| 2        | J3, J4      | Header 6P | 2.54 mm | Pin Header            |




---

## Images



### Soldered PCB Example
Without Pin Headers
<p align="center">
  <img src="../../docs/images/speedo-drive_stepper_controller.JPEG" alt="Soldered Motor Driver PCB" width="400" />
  <br><em>Soldered Motor Driver PCB</em>
</p>

### PCB Schematics
<p align="center">
  <img src="../../docs/images/speedo-drive_schematics.png" alt="Motor Driver Schematic" width="500" />
  <br><em>Motor Driver Schematic</em>
</p>

### Mechanical Pin Numbers
Below is an illustration of the mechanical pin numbers for the Speedo Drive PCB. This helps identify the physical pin layout and connections for assembly and troubleshooting. The pin numbering shown here was used to design the KiCad footprint for this board.

<p align="center">
  <img src="../../docs/images/speedo-drive_mechanical_pin-number.png" alt="Mechanical Pin Numbers" width="400" />
  <br><em>Mechanical Pin Numbers for Speedo Drive PCB</em>
</p>

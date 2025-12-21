# Arduino Test Scripts

This folder contains three Arduino sketches used for testing the ClockClock 24 Replica hardware.

## 1. SpeedoDriveTest

**Purpose:**
This is a basic test script to verify the operation of a single stepper motor driver (specifically designed for the Speedo stepper motors used in this project). It is useful for testing the custom motor driver PCB before assembling the full clock board.

**Origin:**
This script was taken from a project on [Hackaday.io](https://hackaday.io/project/187630-stepper-driver-for-dashpanel-instruments).

**Functionality:**
- Initializes the motor driver.
- Moves the motor to a "zero" position.
- Continuously moves the motor between two positions (3000 and 100) to verify movement and driver control.

**Usage:**
1. Connect your motor driver to the Arduino/Microcontroller as follows:
   - **Enable Pin:** GPIO 20
   - **Direction Pin:** GPIO 6
   - **Step Pin:** GPIO 7
   - **Power:** Connect 5V and GND appropriately.
2. Upload the `SpeedoDriveTest.ino` sketch.
3. The motor should start moving back and forth.

**Test Setup:**
![Test Setup](/docs/images/speedo-drive_test-setup.JPEG)


## 2. BoardTest

**Purpose:**
This script is designed to test the fully assembled main PCB containing 6 motor drivers. It verifies that all motor drivers are functioning and wired correctly to the microcontroller.

**Functionality:**
- Sequentially activates each of the 6 motors (A through F).
- Rotates Motor A forward one full rotation, then backward one full rotation.
- Repeats this process for Motor B, C, D, E, and F.
- Loops continuously.

**Usage:**
1. Flash the `BoardTest.ino` sketch to the main controller board.
2. Power up the board.
3. Observe the motors. They should move one by one in order (A -> F).

---

## 3. AdvancedBoardTest (Board Bit Testing)

**Purpose:**
This is a more advanced test utility that allows you to select and test a specific motor using a 4-bit binary address. This is useful for debugging specific motor channels without waiting for the sequential loop of the standard `BoardTest`.

**Functionality:**
- Reads a 4-bit address from GPIO pins 18, 19, 20, and 21.
- Activates only the motor corresponding to the set address.
- The selected motor rotates forward and backward continuously.
- If the address is changed, the current motor stops and the new motor starts immediately.

**Bit Address Mapping:**

The script uses 4 GPIO pins as a binary input (DIP switch).
- **Bit 0 (LSB):** GPIO 18
- **Bit 1:** GPIO 19
- **Bit 2:** GPIO 20
- **Bit 3 (MSB):** GPIO 21

| Address (Binary) | Hex | Motor Selected |
| :---: | :---: | :---: |
| 0 0 0 0 | 0x0 | **Motor A** |
| 0 0 0 1 | 0x1 | **Motor B** |
| 0 0 1 0 | 0x2 | **Motor C** |
| 0 0 1 1 | 0x3 | **Motor D** |
| 0 1 0 0 | 0x4 | **Motor E** |
| 0 1 0 1 | 0x5 | **Motor F** |

*Note: 0 = OFF, 1 = ON*
/*
 * AdvancedBoardTest (ClockClock 24 Replica)
 *
 * Purpose
 * - Test any single stepper motor on the board by selecting it via a 4-bit DIP address (GPIO 18–21).
 * - While the address is valid and unchanged, the selected motor continuously:
 *     (1) rotates N full rotations forward
 *     (2) then rotates N full rotations backward
 *   repeating forever.
 * - If the address changes, the currently commanded motor is stopped (no further step pulses are issued)
 *   and control immediately switches to the newly addressed motor.
 * - If the address does not match any defined motor, all motors remain stopped.
 *
 * 4-bit address -> stepper mapping (exactly 6 unique addresses)
 * - Bit order: GPIO18 = b0 (LSB), GPIO19 = b1, GPIO20 = b2, GPIO21 = b3 (MSB)
 * - DIP read uses INPUT_PULLUP:
 *     switch ON (to GND)  => reads LOW  => bit = 1
 *     switch OFF (open)   => reads HIGH => bit = 0
 *
 *   Address (b1 b2 b3 b4)   Hex   Motor
 *   ---------------------   ---   -----
 *   0  0  0  0              0x0   A
 *   0  0  0  1              0x1   B
 *   0  0  1  0              0x2   C
 *   0  0  1  1              0x3   D
 *   0  1  0  0              0x4   E
 *   0  1  0  1              0x5   F
 *
 * GPIO usage summary
 * - Motor STEP/DIR pins:
 *     A: STEP=0  DIR=1
 *     B: STEP=2  DIR=7
 *     C: STEP=26 DIR=27
 *     D: STEP=6  DIR=3
 *     E: STEP=8  DIR=9
 *     F: STEP=10 DIR=11
 */

#include <Arduino.h>
#include <AccelStepper.h>

// Reuse the BoardTest notion of "one full rotation".
#define STEPS (360 * 12)

// Motion pattern constants
static const uint8_t N_ROTATIONS = 3;  // N (forward and backward); adjust as desired
static const long FORWARD_STEPS = (long)N_ROTATIONS * (long)STEPS;
static const long BACKWARD_STEPS = (long)N_ROTATIONS * (long)STEPS;

// DIP switch pins (4-bit binary address input)
static const uint8_t ADDR_B0_PIN = 18;
static const uint8_t ADDR_B1_PIN = 19;
static const uint8_t ADDR_B2_PIN = 20;
static const uint8_t ADDR_B3_PIN = 21;

#define A_STEP 0  // f(scx)
#define A_DIR 1   // CW/CCW
#define B_STEP 2  // f(scx)
#define B_DIR 7   // CW/CCW

#define C_STEP 26  // f(scx)
#define C_DIR 27   // CW/CCW
#define D_STEP 6   // f(scx)
#define D_DIR 3    // CW/CCW

#define E_STEP 8   // f(scx)
#define E_DIR 9    // CW/CCW
#define F_STEP 10  // f(scx)
#define F_DIR 11   // CW/CCW

#define RESET 22

// Stepper objects (exactly as in BoardTest.ino)
AccelStepper s_A(AccelStepper::DRIVER, A_STEP, A_DIR);
AccelStepper s_B(AccelStepper::DRIVER, B_STEP, B_DIR);
AccelStepper s_C(AccelStepper::DRIVER, C_STEP, C_DIR);
AccelStepper s_D(AccelStepper::DRIVER, D_STEP, D_DIR);
AccelStepper s_E(AccelStepper::DRIVER, E_STEP, E_DIR);
AccelStepper s_F(AccelStepper::DRIVER, F_STEP, F_DIR);

AccelStepper* motors[] = { &s_A, &s_B, &s_C, &s_D, &s_E, &s_F };
static const uint8_t NUM_MOTORS = sizeof(motors) / sizeof(motors[0]);

enum MotionPhase : uint8_t {
  PHASE_FORWARD = 0,
  PHASE_BACKWARD = 1,
};

static int8_t activeMotorIndex = -1;  // -1 means "none"
static uint8_t lastAddress = 0xFF;    // force initial update
static MotionPhase phase = PHASE_FORWARD;
static bool moveStarted = false;

static uint8_t readAddress4bit() {
  const uint8_t b0 = (digitalRead(ADDR_B0_PIN) == LOW) ? 1 : 0;
  const uint8_t b1 = (digitalRead(ADDR_B1_PIN) == LOW) ? 1 : 0;
  const uint8_t b2 = (digitalRead(ADDR_B2_PIN) == LOW) ? 1 : 0;
  const uint8_t b3 = (digitalRead(ADDR_B3_PIN) == LOW) ? 1 : 0;
  return (uint8_t)((b3 << 3) | (b2 << 2) | (b1 << 1) | b0);
}

static int8_t addressToMotorIndex(uint8_t address) {
  // Exactly 6 unique addresses are valid.
  switch (address) {
    case 0x0: return 0;  // A
    case 0x1: return 1;  // B
    case 0x2: return 2;  // C
    case 0x3: return 3;  // D
    case 0x4: return 4;  // E
    case 0x5: return 5;  // F
    default: return -1;
  }
}

static void stopAllMotors() {
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    AccelStepper* m = motors[i];
    m->stop();
    m->moveTo(m->currentPosition());
    m->setSpeed(0);
  }
}

static void startSelectedMotorPattern(int8_t motorIndex) {
  if (motorIndex < 0 || motorIndex >= (int8_t)NUM_MOTORS) {
    return;
  }

  AccelStepper* m = motors[motorIndex];

  // Make the test deterministic whenever a motor is (re)selected.
  m->setCurrentPosition(0);

  phase = PHASE_FORWARD;
  moveStarted = false;
}

void setup() {
  Serial.begin(115200);
  Serial.println("ClockClock 24 replica advanced board test (DIP-addressed single motor)");

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  // Reset motor controllers
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);

  // DIP address pins
  pinMode(ADDR_B0_PIN, INPUT_PULLUP);
  pinMode(ADDR_B1_PIN, INPUT_PULLUP);
  pinMode(ADDR_B2_PIN, INPUT_PULLUP);
  pinMode(ADDR_B3_PIN, INPUT_PULLUP);

  // Set motor parameters (same as BoardTest.ino)
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    motors[i]->setMaxSpeed(16000);
    motors[i]->setAcceleration(2000);
    motors[i]->setCurrentPosition(0);
  }

  stopAllMotors();
}

void loop() {
  const uint8_t address = readAddress4bit();

  // Address change: stop current motor and switch immediately.
  if (address != lastAddress) {
    lastAddress = address;

    stopAllMotors();

    activeMotorIndex = addressToMotorIndex(address);
    if (activeMotorIndex >= 0) {
      Serial.print("Selected motor ");
      Serial.print((char)('A' + activeMotorIndex));
      Serial.print(" (address 0x");
      Serial.print(address, HEX);
      Serial.println(")");

      startSelectedMotorPattern(activeMotorIndex);
    } else {
      Serial.print("Invalid address 0x");
      Serial.print(address, HEX);
      Serial.println(" -> all motors stopped");

      phase = PHASE_FORWARD;
      moveStarted = false;
    }
  }

  // If no valid motor is selected, keep everything stopped.
  if (activeMotorIndex < 0) {
    return;
  }

  AccelStepper* m = motors[activeMotorIndex];

  // Schedule a move when needed.
  if (!moveStarted) {
    const long delta = (phase == PHASE_FORWARD) ? FORWARD_STEPS : -BACKWARD_STEPS;
    m->move(delta);
    moveStarted = true;

    Serial.print("Motor ");
    Serial.print((char)('A' + activeMotorIndex));
    Serial.print(phase == PHASE_FORWARD ? " forward " : " backward ");
    Serial.println("segment");
  }

  // Non-blocking motion.
  m->run();

  // Segment complete -> flip phase and schedule next segment.
  if (moveStarted && m->distanceToGo() == 0) {
    if (phase == PHASE_FORWARD) {
      phase = PHASE_BACKWARD;
      moveStarted = false;
    } else {
      // Backward complete -> reset position and repeat forward.
      m->setCurrentPosition(0);
      phase = PHASE_FORWARD;
      moveStarted = false;
    }
  }
}

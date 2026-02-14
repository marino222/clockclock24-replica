/*
 * ClockClock 24 Replica Board Test
 * This script sequentially tests six stepper motors by rotating each motor forward and backward one full rotation. 
 * It uses the AccelStepper library to control the motors and provides status updates via the serial monitor.
 */

#include <Arduino.h>
#include <AccelStepper.h>

#define STEPS (360 * 12)

#define A_STEP 0  // f(scx)
#define A_DIR 1   // CW/CCW
#define B_STEP 2  // f(scx)
#define B_DIR 3   // CW/CCW

#define C_STEP 26  // f(scx)
#define C_DIR 27   // CW/CCW
#define D_STEP 6   // f(scx)
#define D_DIR 7    // CW/CCW

#define E_STEP 8   // f(scx)
#define E_DIR 9    // CW/CCW
#define F_STEP 10  // f(scx)
#define F_DIR 11   // CW/CCW

#define RESET 22

// Stepper objects
AccelStepper s_A(AccelStepper::DRIVER, A_STEP, A_DIR);
AccelStepper s_B(AccelStepper::DRIVER, B_STEP, B_DIR);
AccelStepper s_C(AccelStepper::DRIVER, C_STEP, C_DIR);
AccelStepper s_D(AccelStepper::DRIVER, D_STEP, D_DIR);
AccelStepper s_E(AccelStepper::DRIVER, E_STEP, E_DIR);
AccelStepper s_F(AccelStepper::DRIVER, F_STEP, F_DIR);

// Array to iterate over all motors easily
AccelStepper* motors[] = { &s_A, &s_B, &s_C, &s_D, &s_E, &s_F };
const uint8_t NUM_MOTORS = sizeof(motors) / sizeof(motors[0]);

// State variables for the test sequence
uint8_t currentMotor = 0;   // 0 = A, 1 = B, ... 5 = F
bool directionForward = true;
bool moveStarted = false;

void setup() {
  Serial.begin(115200);
  Serial.println("ClockClock 24 replica board test (Sequential motor test)");

  pinMode(25, OUTPUT);  // GPIO25 = Onboard LED
  digitalWrite(25, HIGH);

  // Reset motor controllers
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);


  // Set all motor parameters
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    motors[i]->setMaxSpeed(16000);
    motors[i]->setAcceleration(2000);
    motors[i]->setCurrentPosition(0);
  }
}

void loop() {
  // Select the current motor
  AccelStepper* m = motors[currentMotor];

  // If no movement has started yet, initiate a new move
  if (!moveStarted) {
    long target = directionForward ? STEPS : -STEPS;  // one full rotation
    m->moveTo(target);
    moveStarted = true;

    Serial.print("Motor ");
    Serial.print('A' + currentMotor);
    Serial.print(directionForward ? " forward" : " backward");
    Serial.println(" test");
  }

  // Only run the current motor (others remain stationary)
  m->run();

  // When the target is reached, proceed to the next step in the sequence
  if (moveStarted && m->distanceToGo() == 0) {
    if (directionForward) {
      // Forward test complete -> now test backward
      directionForward = false;
      moveStarted = false;
    } else {
      // Backward test complete -> reset motor and switch to the next one
      m->setCurrentPosition(0);
      directionForward = true;
      moveStarted = false;
      currentMotor = (currentMotor + 1) % NUM_MOTORS;  // after F, go back to A
    }
  }
}

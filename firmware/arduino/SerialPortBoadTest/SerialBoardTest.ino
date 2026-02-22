/*
 * ClockClock 24 Replica Board Test (Serial Control)
 * This script tests specific stepper motors by rotating them forward and backward. 
 * Type 'A', 'B', 'C', 'D', 'E', or 'F' in the Serial Monitor to spin the respective motor.
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
int activeMotor = -1;       // -1 indicates idle (no motor selected)
bool directionForward = true;
bool moveStarted = false;

void setup() {
  Serial.begin(115200);
  Serial.println("ClockClock 24 replica board test (Serial motor control)");
  Serial.println("Type a letter (A - F) and press Enter to test a motor.");

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
  // 1. Check for incoming Serial commands
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    
    // Convert to uppercase to handle both 'a' and 'A' inputs
    inChar = toupper(inChar); 
    
    // Check if the character is valid (A through F)
    if (inChar >= 'A' && inChar <= 'F') {
      activeMotor = inChar - 'A'; // Maps 'A'->0, 'B'->1, etc.
      
      // Reset state variables for the new test
      directionForward = true;
      moveStarted = false;
      
      // Zero out the target position to ensure a clean start
      motors[activeMotor]->setCurrentPosition(0);
      
      Serial.println("------------------------------------");
      Serial.print("Selected Motor: ");
      Serial.println(inChar);
    }
  }

  // 2. Drive the selected motor (if one is active)
  if (activeMotor >= 0 && activeMotor < NUM_MOTORS) {
    AccelStepper* m = motors[activeMotor];

    // Initiate the move if not already started
    if (!moveStarted) {
      long target = directionForward ? STEPS : -STEPS;  // one full rotation
      m->moveTo(target);
      moveStarted = true;

      Serial.print("Running ");
      Serial.print((char)('A' + activeMotor));
      Serial.println(directionForward ? " forward..." : " backward...");
    }

    // Call run() constantly to step the motor
    m->run();

    // Check if the motor has reached its target position
    if (moveStarted && m->distanceToGo() == 0) {
      if (directionForward) {
        // Forward test complete -> now test backward
        directionForward = false;
        moveStarted = false;
      } else {
        // Backward test complete -> return to idle state
        m->setCurrentPosition(0);
        activeMotor = -1; // Go back to waiting for serial input
        Serial.println("Test complete. Enter a letter A-F for the next test.");
      }
    }
  }
}
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

// State variables for synchronized motor test (X command)
bool syncMode = false;
int syncPhase = 0;  // 0 = A,C,E forward, 1 = A,C,E backward, 2 = B,D,F forward, 3 = B,D,F backward

void setup() {
  Serial.begin(115200);
  Serial.println("ClockClock 24 replica board test (Serial motor control)");
  Serial.println("Type a letter (A - F) and press Enter to test a motor.");
  Serial.println("Type X to test motors synchronously (A,C,E then B,D,F).");

  pinMode(25, OUTPUT);  // GPIO25 = Onboard LED
  digitalWrite(25, HIGH);

  // Reset motor controllers
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);

  // Set all motor parametersc
  
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    motors[i]->setMaxSpeed(1500);
    motors[i]->setAcceleration(400);
    motors[i]->setCurrentPosition(0);
  }
}

void loop() {
  // 1. Check for incoming Serial commands
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    
    // Convert to uppercase to handle both 'a' and 'A' inputs
    inChar = toupper(inChar); 
    
    // Check for synchronized test mode
    if (inChar == 'X') {
      syncMode = true;
      syncPhase = 0;
      moveStarted = false;
      activeMotor = -1;  // Disable single motor mode
      
      // Reset positions for motors A, C, E
      s_A.setCurrentPosition(0);
      s_C.setCurrentPosition(0);
      s_E.setCurrentPosition(0);
      
      Serial.println("------------------------------------");
      Serial.println("Selected: Synchronized Motor Test");
    }
    // Check if the character is valid (A through F)
    else if (inChar >= 'A' && inChar <= 'F') {
      activeMotor = inChar - 'A'; // Maps 'A'->0, 'B'->1, etc.
      
      // Reset state variables for the new test
      directionForward = true;
      moveStarted = false;
      syncMode = false;  // Disable sync mode
      
      // Zero out the target position to ensure a clean start
      motors[activeMotor]->setCurrentPosition(0);
      
      Serial.println("------------------------------------");
      Serial.print("Selected Motor: ");
      Serial.println(inChar);
    }
  }

  // 2. Handle synchronized motor test mode
  if (syncMode) {
    if (!moveStarted) {
      long target;
      
      switch (syncPhase) {
        case 0:  // A, C, E forward
          target = STEPS;
          s_A.moveTo(target);
          s_C.moveTo(target);
          s_E.moveTo(target);
          Serial.println("Running A, C, E forward...");
          break;
        
        case 1:  // A, C, E backward
          target = -STEPS;
          s_A.moveTo(target);
          s_C.moveTo(target);
          s_E.moveTo(target);
          Serial.println("Running A, C, E backward...");
          break;
        
        case 2:  // B, D, F forward
          target = STEPS;
          s_B.setCurrentPosition(0);  // Reset positions
          s_D.setCurrentPosition(0);
          s_F.setCurrentPosition(0);
          s_B.moveTo(target);
          s_D.moveTo(target);
          s_F.moveTo(target);
          Serial.println("Running B, D, F forward...");
          break;
        
        case 3:  // B, D, F backward
          target = -STEPS;
          s_B.moveTo(target);
          s_D.moveTo(target);
          s_F.moveTo(target);
          Serial.println("Running B, D, F backward...");
          break;
      }
      moveStarted = true;
    }
    
    // Run the appropriate motors based on current phase
    if (syncPhase <= 1) {
      s_A.run();
      s_C.run();
      s_E.run();
      
      // Check if all motors have reached target
      if (s_A.distanceToGo() == 0 && s_C.distanceToGo() == 0 && s_E.distanceToGo() == 0) {
        syncPhase++;
        moveStarted = false;
        
        if (syncPhase == 2) {
          // Reset A, C, E positions before starting B, D, F
          s_A.setCurrentPosition(0);
          s_C.setCurrentPosition(0);
          s_E.setCurrentPosition(0);
        }
      }
    } else if (syncPhase <= 3) {
      s_B.run();
      s_D.run();
      s_F.run();
      
      // Check if all motors have reached target
      if (s_B.distanceToGo() == 0 && s_D.distanceToGo() == 0 && s_F.distanceToGo() == 0) {
        syncPhase++;
        moveStarted = false;
        
        if (syncPhase > 3) {
          // All synchronized tests complete
          s_B.setCurrentPosition(0);
          s_D.setCurrentPosition(0);
          s_F.setCurrentPosition(0);
          syncMode = false;
          Serial.println("Synchronized test complete. Enter a letter A-F or X for the next test.");
        }
      }
    }
  }
  // 3. Drive the selected motor (if one is active)
  else if (activeMotor >= 0 && activeMotor < NUM_MOTORS) {
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
        Serial.println("Test complete. Enter a letter A-F or X for the next test.");
      }
    }
  }
}
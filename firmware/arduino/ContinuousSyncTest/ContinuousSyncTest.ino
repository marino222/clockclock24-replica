/*
 * ClockClock 24 Replica - Continuous Synchronized Board Test
 * Continuously cycles all motors through the synchronized test sequence:
 *   Phase 0: A, C, E forward one full rotation
 *   Phase 1: A, C, E backward one full rotation
 *   Phase 2: B, D, F forward one full rotation
 *   Phase 3: B, D, F backward one full rotation
 * ...then repeats indefinitely.
 */

#include <Arduino.h>
#include <AccelStepper.h>

#define STEPS (360 * 12)

#define A_STEP 0
#define A_DIR  1
#define B_STEP 2
#define B_DIR  3

#define C_STEP 26
#define C_DIR  27
#define D_STEP 6
#define D_DIR  7

#define E_STEP 8
#define E_DIR  9
#define F_STEP 10
#define F_DIR  11

#define RESET 22

// Stepper objects
AccelStepper s_A(AccelStepper::DRIVER, A_STEP, A_DIR);
AccelStepper s_B(AccelStepper::DRIVER, B_STEP, B_DIR);
AccelStepper s_C(AccelStepper::DRIVER, C_STEP, C_DIR);
AccelStepper s_D(AccelStepper::DRIVER, D_STEP, D_DIR);
AccelStepper s_E(AccelStepper::DRIVER, E_STEP, E_DIR);
AccelStepper s_F(AccelStepper::DRIVER, F_STEP, F_DIR);

int syncPhase = 0;    // 0=ACE fwd, 1=ACE bwd, 2=BDF fwd, 3=BDF bwd
bool moveStarted = false;

void setup() {
  pinMode(25, OUTPUT);  // Onboard LED
  digitalWrite(25, HIGH);

  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);

  AccelStepper* motors[] = { &s_A, &s_B, &s_C, &s_D, &s_E, &s_F };
  for (uint8_t i = 0; i < 6; i++) {
    motors[i]->setMaxSpeed(1500);
    motors[i]->setAcceleration(400);
    motors[i]->setCurrentPosition(0);
  }
}

void loop() {
  if (!moveStarted) {
    long target;

    switch (syncPhase) {
      case 0:  // A, C, E forward
        s_A.setCurrentPosition(0);
        s_C.setCurrentPosition(0);
        s_E.setCurrentPosition(0);
        target = STEPS;
        s_A.moveTo(target);
        s_C.moveTo(target);
        s_E.moveTo(target);
        break;

      case 1:  // A, C, E backward
        target = -STEPS;
        s_A.moveTo(target);
        s_C.moveTo(target);
        s_E.moveTo(target);
        break;

      case 2:  // B, D, F forward
        s_B.setCurrentPosition(0);
        s_D.setCurrentPosition(0);
        s_F.setCurrentPosition(0);
        target = STEPS;
        s_B.moveTo(target);
        s_D.moveTo(target);
        s_F.moveTo(target);
        break;

      case 3:  // B, D, F backward
        target = -STEPS;
        s_B.moveTo(target);
        s_D.moveTo(target);
        s_F.moveTo(target);
        break;
    }

    moveStarted = true;
  }

  // Drive motors and advance phase when complete
  if (syncPhase <= 1) {
    s_A.run();
    s_C.run();
    s_E.run();

    if (s_A.distanceToGo() == 0 && s_C.distanceToGo() == 0 && s_E.distanceToGo() == 0) {
      syncPhase = (syncPhase + 1) % 4;
      moveStarted = false;
    }
  } else {
    s_B.run();
    s_D.run();
    s_F.run();

    if (s_B.distanceToGo() == 0 && s_D.distanceToGo() == 0 && s_F.distanceToGo() == 0) {
      syncPhase = (syncPhase + 1) % 4;
      moveStarted = false;
    }
  }
}

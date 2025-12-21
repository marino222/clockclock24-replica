#include <AccelStepper.h>

// --- Pins (change to your wiring) ---
constexpr uint8_t STEP_PIN = 7;   // -> VID6606 StpA (f(scx)A)
constexpr uint8_t DIR_PIN  = 6;   // -> VID6606 DirA (CW/CCWA)
constexpr uint8_t RST_PIN  = 20;   // -> VID6606 RESET (active-low reset, must be HIGH to run)

// --- Motor model assumptions ---
// VID6606 outputs microsteps (1 pulse = 1 microstep). :contentReference[oaicite:4]{index=4}
// BKA30D datasheet states min output step can reach 1/12 degree. :contentReference[oaicite:5]{index=5}
// => 360 deg / (1/12 deg) = 4320 microsteps per output revolution.
constexpr long MICROSTEPS_PER_REV = 4320;

// Move amplitude for the back-and-forth demo
constexpr long SWEEP_STEPS = MICROSTEPS_PER_REV ; // 180 degrees

// AccelStepper in DRIVER mode = (stepPin, dirPin)
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

static void vid6606ResetSequence()
{
  pinMode(RST_PIN, OUTPUT);

  // Recommended power-up: keep RESET low during VDD rising, then release after ~1ms. :contentReference[oaicite:6]{index=6}
  digitalWrite(RST_PIN, LOW);
  delay(5);               // conservative; datasheet suggests ~1ms is enough
  digitalWrite(RST_PIN, HIGH);
  delay(1);
}

void setup()
{
  vid6606ResetSequence();

  // Ensure clean logic levels (VID6606 is CMOS; floating inputs can break things). :contentReference[oaicite:7]{index=7}
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  // VID6606 requires pulse width >= 450ns. Set 1us to be safe. :contentReference[oaicite:8]{index=8}
  stepper.setMinPulseWidth(1); // microseconds

  // Speed/accel: keep modest first. You can increase later.
  // Units are "steps per second" and "steps per second^2".
  // 2000 steps/s corresponds to 2000/4320 rev/s = 0.463 rev/s = 166 deg/s at the output.
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(2500);

  stepper.setCurrentPosition(0);
  stepper.moveTo(SWEEP_STEPS);
}

void loop()
{
  // Non-blocking motion update (must be called frequently)
  stepper.run();

  // When target reached, reverse direction and go back
  if (stepper.distanceToGo() == 0)
  {
    long nextTarget = (stepper.currentPosition() == 0) ? SWEEP_STEPS : 0;
    stepper.moveTo(nextTarget);

    // Optional: brief pause at the ends
    delay(150);
  }
}

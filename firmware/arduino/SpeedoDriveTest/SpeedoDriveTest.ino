//Test Setup mit DRV8833
// 5V Power Supply mit VCC am DRV8833 verbinden
//GND vom DRV8833 mit GND vom Pico verbinden
//GND vom Power Supply mit anderem GND vom Pico verbinden


#include <AccelStepper.h>
#include <Arduino.h>

// Motor Connections (constant current, step/direction bipolar motor driver)
const int enabPin = 20;
const int dirPin = 6;
const int stepPin = 7;
// This defines the analog input pin for reading the control voltage
// Tested with a 10k linear pot between 5v and GND

// modified for my stepper and controller
AccelStepper myStepper(AccelStepper::DRIVER, stepPin, dirPin);   
const long STEPS_PER_REVOLUTION = 360 * 12;   // for 315 degrees with twelve step microstepping


void setup() {
  //Serial.begin(115200);
  pinMode(enabPin,OUTPUT);
  digitalWrite(enabPin,LOW); 
  delay(100);
  digitalWrite(enabPin,HIGH);    // this enables the controller
  myStepper.setMaxSpeed(2000);
  myStepper.setAcceleration(2400);
  myStepper.runToNewPosition(-4000);   // runs the stepper backwards to 'zero'
  myStepper.setCurrentPosition(0);


}

void loop() {
  myStepper.runToNewPosition(3000);
  myStepper.runToNewPosition(100);
  delay(1000);

}

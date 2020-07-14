// ConstantSpeed.pde
// -*- mode: C++ -*-
//
// Shows how to run AccelStepper in the simplest,
// fixed speed mode with no accelerations
// Requires the Adafruit_Motorshield v2 library
//   https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library
// And AccelStepper with AFMotor support
//   https://github.com/adafruit/AccelStepper

// This tutorial is for Adafruit Motorshield v2 only!
// Will not work with v1 shields

#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include <Wire.h>

bool verboseDebugging = false;

// * Pulling these values out to variables so we can flip the values in the loop
int targetDistance = 200; // * The target distance we want to move to
int targetSpeed = 400;    // * The speed that we want to move at

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myStepper1 = AFMS.getStepper(200, 2);

// you can change these to DOUBLE or INTERLEAVE or MICROSTEP!
void forwardstep1()
{
  myStepper1->onestep(FORWARD, SINGLE);
}
void backwardstep1()
{
  myStepper1->onestep(BACKWARD, SINGLE);
}

AccelStepper Astepper1(forwardstep1, backwardstep1); // use functions to step

void setup()
{
  Serial.begin(9600); // set up Serial library at 9600 bps
  while (verboseDebugging && !Serial.available())
  {
    ;
  }
  Serial.println("Stepper test!");

  AFMS.begin(); // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz

  // ! Note that you need to set the max speed, otherwise the stepper motor will
  //  ! only step once per interval regardless of what `setSpeed` value you give
  Astepper1.setMaxSpeed(1000);

  Astepper1.moveTo(targetDistance);
  Astepper1.setSpeed(targetSpeed);
}

void loop()
{
  if (verboseDebugging)
  {
    Serial.print("Target Distance: ");
    Serial.print(Astepper1.targetPosition());
    Serial.print(" | Distance to go: ");
    Serial.println(Astepper1.distanceToGo());
  }

  if (Astepper1.distanceToGo() == 0)
  {
    Serial.println("CHANGE DIRECTIONS!");

    Astepper1.stop();
    Astepper1.setSpeed(0);

    Astepper1.moveTo(-Astepper1.currentPosition());

    targetSpeed = targetSpeed * -1; // * flipping the sign of the target speed
    Astepper1.setSpeed(targetSpeed);

    if (verboseDebugging)
    {
      Serial.print("New target position: ");
      Serial.println(Astepper1.targetPosition());
      Serial.print("New target Speed: ");
      Serial.println(Astepper1.speed());
    }
  }

  Astepper1.runSpeed();
}

#pragma once

#include <cstdint>

#include "utility/Adafruit_MS_PWMServoDriver.h"

#define MICROSTEPS 16 // 8 or 16

#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR4_A 0
#define MOTOR4_B 6
#define MOTOR3_A 5
#define MOTOR3_B 7

#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

#define SINGLE 1
#define DOUBLE 2
#define INTERLEAVE 3
#define MICROSTEP 4

class Adafruit_MotorShield;

class Adafruit_DCMotor {
 public:
  Adafruit_DCMotor(void);
  friend class Adafruit_MotorShield;
  void run(uint8_t);
  void setSpeed(uint8_t);
  void setSpeedFine(uint16_t speed);
  void fullOn();
  void fullOff();

 private:
  uint8_t PWMpin, IN1pin, IN2pin;
  Adafruit_MotorShield* MC;
  uint8_t motornum;
};

class Adafruit_StepperMotor {
 public:
  Adafruit_StepperMotor(void);
  void setSpeed(uint16_t);
  void step(uint16_t steps, uint8_t dir, uint8_t style = SINGLE);
  uint8_t onestep(uint8_t dir, uint8_t style);
  void release(void);

  friend class Adafruit_MotorShield;

 private:
  uint32_t usperstep;
  uint8_t PWMApin, AIN1pin, AIN2pin;
  uint8_t PWMBpin, BIN1pin, BIN2pin;
  uint16_t revsteps;
  uint8_t currentstep;
  Adafruit_MotorShield* MC;
  uint8_t steppernum;
};

class Adafruit_MotorShield {
 public:
  Adafruit_MotorShield(uint8_t addr = 0x60);

  bool begin(uint16_t freq = 1600, int busNum = 1);
  bool verify();
  Adafruit_DCMotor* getMotor(uint8_t n);
  Adafruit_StepperMotor* getStepper(uint16_t steps, uint8_t n);

  friend class Adafruit_DCMotor;

  void setPWM(uint8_t pin, uint16_t val);
  void setPin(uint8_t pin, bool val);

 private:
  uint8_t _addr;
  uint16_t _freq;
  Adafruit_DCMotor dcmotors[4];
  Adafruit_StepperMotor steppers[2];
  Adafruit_MS_PWMServoDriver _pwm;
};

#include "Adafruit_MotorShield.h"

#include <unistd.h>

#if (MICROSTEPS == 8)
static uint8_t microstepcurve[] = {0, 50, 98, 142, 180, 212, 236, 250, 255};
#elif (MICROSTEPS == 16)
static uint8_t microstepcurve[] = {0,   25,  50,  74,  98,  120, 141, 162, 180,
                                   197, 212, 225, 236, 244, 250, 253, 255};
#endif

Adafruit_MotorShield::Adafruit_MotorShield(uint8_t addr) { _addr = addr; }

bool Adafruit_MotorShield::begin(uint16_t freq, int busNum) {
  _pwm = Adafruit_MS_PWMServoDriver(_addr);
  if (!_pwm.begin(busNum))
    return false;
  _freq = freq;
  _pwm.setPWMFreq(_freq);
  for (uint8_t i = 0; i < 16; i++)
    _pwm.setPWM(i, 0, 0);
  return true;
}

void Adafruit_MotorShield::setPWM(uint8_t pin, uint16_t value) {
  if (value > 4095)
    _pwm.setPWM(pin, 4096, 0);
  else
    _pwm.setPWM(pin, 0, value);
}

void Adafruit_MotorShield::setPin(uint8_t pin, bool value) {
  if (!value)
    _pwm.setPWM(pin, 0, 0);
  else
    _pwm.setPWM(pin, 4096, 0);
}

Adafruit_DCMotor* Adafruit_MotorShield::getMotor(uint8_t num) {
  if (num > 4)
    return nullptr;

  num--;

  if (dcmotors[num].motornum == 0) {
    dcmotors[num].motornum = num;
    dcmotors[num].MC = this;
    uint8_t pwm, in1, in2;
    switch (num) {
      case 0:
        pwm = 8;
        in2 = 9;
        in1 = 10;
        break;
      case 1:
        pwm = 13;
        in2 = 12;
        in1 = 11;
        break;
      case 2:
        pwm = 2;
        in2 = 3;
        in1 = 4;
        break;
      default:
        pwm = 7;
        in2 = 6;
        in1 = 5;
        break;
    }
    dcmotors[num].PWMpin = pwm;
    dcmotors[num].IN1pin = in1;
    dcmotors[num].IN2pin = in2;
  }
  return &dcmotors[num];
}

Adafruit_StepperMotor* Adafruit_MotorShield::getStepper(uint16_t steps,
                                                        uint8_t num) {
  if (num > 2)
    return nullptr;

  num--;

  if (steppers[num].steppernum == 0) {
    steppers[num].steppernum = num;
    steppers[num].revsteps = steps;
    steppers[num].MC = this;
    uint8_t pwma, pwmb, ain1, ain2, bin1, bin2;
    if (num == 0) {
      pwma = 8;
      ain2 = 9;
      ain1 = 10;
      pwmb = 13;
      bin2 = 12;
      bin1 = 11;
    } else {
      pwma = 2;
      ain2 = 3;
      ain1 = 4;
      pwmb = 7;
      bin2 = 6;
      bin1 = 5;
    }
    steppers[num].PWMApin = pwma;
    steppers[num].PWMBpin = pwmb;
    steppers[num].AIN1pin = ain1;
    steppers[num].AIN2pin = ain2;
    steppers[num].BIN1pin = bin1;
    steppers[num].BIN2pin = bin2;
  }
  return &steppers[num];
}

/******************************************
               MOTORS
******************************************/

Adafruit_DCMotor::Adafruit_DCMotor(void) {
  MC = nullptr;
  motornum = 0;
  PWMpin = IN1pin = IN2pin = 0;
}

void Adafruit_DCMotor::run(uint8_t cmd) {
  switch (cmd) {
    case FORWARD:
      MC->setPin(IN2pin, false);
      MC->setPin(IN1pin, true);
      break;
    case BACKWARD:
      MC->setPin(IN1pin, false);
      MC->setPin(IN2pin, true);
      break;
    case RELEASE:
      MC->setPin(IN1pin, false);
      MC->setPin(IN2pin, false);
      break;
  }
}

void Adafruit_DCMotor::setSpeed(uint8_t speed) {
  MC->setPWM(PWMpin, speed * 16);
}

void Adafruit_DCMotor::setSpeedFine(uint16_t speed) {
  MC->setPWM(PWMpin, speed > 4095 ? 4095 : speed);
}

void Adafruit_DCMotor::fullOn() { MC->_pwm.setPWM(PWMpin, 4096, 0); }

void Adafruit_DCMotor::fullOff() { MC->_pwm.setPWM(PWMpin, 0, 4096); }

/******************************************
               STEPPERS
******************************************/

Adafruit_StepperMotor::Adafruit_StepperMotor(void) {
  revsteps = steppernum = currentstep = 0;
}

void Adafruit_StepperMotor::setSpeed(uint16_t rpm) {
  usperstep = 60000000 / ((uint32_t)revsteps * (uint32_t)rpm);
}

void Adafruit_StepperMotor::release(void) {
  MC->setPin(AIN1pin, false);
  MC->setPin(AIN2pin, false);
  MC->setPin(BIN1pin, false);
  MC->setPin(BIN2pin, false);
  MC->setPWM(PWMApin, 0);
  MC->setPWM(PWMBpin, 0);
}

void Adafruit_StepperMotor::step(uint16_t steps, uint8_t dir, uint8_t style) {
  uint32_t uspers = usperstep;

  if (style == INTERLEAVE) {
    uspers /= 2;
  } else if (style == MICROSTEP) {
    uspers /= MICROSTEPS;
    steps *= MICROSTEPS;
  }

  while (steps--) {
    onestep(dir, style);
    usleep(uspers);
  }
}

uint8_t Adafruit_StepperMotor::onestep(uint8_t dir, uint8_t style) {
  uint8_t ocrb, ocra;
  ocra = ocrb = 255;

  if (style == SINGLE) {
    if ((currentstep / (MICROSTEPS / 2)) % 2) {
      if (dir == FORWARD)
        currentstep += MICROSTEPS / 2;
      else
        currentstep -= MICROSTEPS / 2;
    } else {
      if (dir == FORWARD)
        currentstep += MICROSTEPS;
      else
        currentstep -= MICROSTEPS;
    }
  } else if (style == DOUBLE) {
    if (!(currentstep / (MICROSTEPS / 2) % 2)) {
      if (dir == FORWARD)
        currentstep += MICROSTEPS / 2;
      else
        currentstep -= MICROSTEPS / 2;
    } else {
      if (dir == FORWARD)
        currentstep += MICROSTEPS;
      else
        currentstep -= MICROSTEPS;
    }
  } else if (style == INTERLEAVE) {
    if (dir == FORWARD)
      currentstep += MICROSTEPS / 2;
    else
      currentstep -= MICROSTEPS / 2;
  }

  if (style == MICROSTEP) {
    if (dir == FORWARD)
      currentstep++;
    else
      currentstep--;

    currentstep += MICROSTEPS * 4;
    currentstep %= MICROSTEPS * 4;

    ocra = ocrb = 0;
    if (currentstep < MICROSTEPS) {
      ocra = microstepcurve[MICROSTEPS - currentstep];
      ocrb = microstepcurve[currentstep];
    } else if ((currentstep >= MICROSTEPS) && (currentstep < MICROSTEPS * 2)) {
      ocra = microstepcurve[currentstep - MICROSTEPS];
      ocrb = microstepcurve[MICROSTEPS * 2 - currentstep];
    } else if ((currentstep >= MICROSTEPS * 2) &&
               (currentstep < MICROSTEPS * 3)) {
      ocra = microstepcurve[MICROSTEPS * 3 - currentstep];
      ocrb = microstepcurve[currentstep - MICROSTEPS * 2];
    } else if ((currentstep >= MICROSTEPS * 3) &&
               (currentstep < MICROSTEPS * 4)) {
      ocra = microstepcurve[currentstep - MICROSTEPS * 3];
      ocrb = microstepcurve[MICROSTEPS * 4 - currentstep];
    }
  }

  currentstep += MICROSTEPS * 4;
  currentstep %= MICROSTEPS * 4;

  MC->setPWM(PWMApin, ocra * 16);
  MC->setPWM(PWMBpin, ocrb * 16);

  uint8_t latch_state = 0;

  if (style == MICROSTEP) {
    if (currentstep < MICROSTEPS)
      latch_state |= 0x03;
    if ((currentstep >= MICROSTEPS) && (currentstep < MICROSTEPS * 2))
      latch_state |= 0x06;
    if ((currentstep >= MICROSTEPS * 2) && (currentstep < MICROSTEPS * 3))
      latch_state |= 0x0C;
    if ((currentstep >= MICROSTEPS * 3) && (currentstep < MICROSTEPS * 4))
      latch_state |= 0x09;
  } else {
    switch (currentstep / (MICROSTEPS / 2)) {
      case 0:
        latch_state |= 0x1;
        break;
      case 1:
        latch_state |= 0x3;
        break;
      case 2:
        latch_state |= 0x2;
        break;
      case 3:
        latch_state |= 0x6;
        break;
      case 4:
        latch_state |= 0x4;
        break;
      case 5:
        latch_state |= 0xC;
        break;
      case 6:
        latch_state |= 0x8;
        break;
      case 7:
        latch_state |= 0x9;
        break;
    }
  }

  MC->setPin(AIN2pin, latch_state & 0x1);
  MC->setPin(BIN1pin, latch_state & 0x2);
  MC->setPin(AIN1pin, latch_state & 0x4);
  MC->setPin(BIN2pin, latch_state & 0x8);

  return currentstep;
}

#pragma once

#include <cstdint>

#include "linux_i2c.h"

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

class Adafruit_MS_PWMServoDriver {
 public:
  Adafruit_MS_PWMServoDriver(uint8_t addr = 0x40);
  bool begin(int busNum = 1);
  void reset(void);
  void setPWMFreq(float freq);
  void setPWM(uint8_t num, uint16_t on, uint16_t off);
  bool verify();

 private:
  uint8_t _i2caddr;
  LinuxI2CDevice* i2c_dev = nullptr;

  uint8_t read8(uint8_t addr);
  void write8(uint8_t addr, uint8_t d);
};

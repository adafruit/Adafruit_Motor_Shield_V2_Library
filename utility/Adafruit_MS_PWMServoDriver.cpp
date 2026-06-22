#include "Adafruit_MS_PWMServoDriver.h"

#include <unistd.h>

#include <cmath>

Adafruit_MS_PWMServoDriver::Adafruit_MS_PWMServoDriver(uint8_t addr) {
  _i2caddr = addr;
}

bool Adafruit_MS_PWMServoDriver::begin(int busNum) {
  delete i2c_dev;
  i2c_dev = new LinuxI2CDevice(_i2caddr, busNum);
  if (!i2c_dev->begin())
    return false;
  reset();
  return true;
}

void Adafruit_MS_PWMServoDriver::reset(void) {
  write8(PCA9685_MODE1, 0x0);
}

void Adafruit_MS_PWMServoDriver::setPWMFreq(float freq) {
  freq *= 0.9f; // correct for oscillator overshoot (see issue #11)

  float prescaleval = 25000000.0f / 4096.0f / freq - 1.0f;
  uint8_t prescale = static_cast<uint8_t>(std::floor(prescaleval + 0.5f));

  uint8_t oldmode = read8(PCA9685_MODE1);
  write8(PCA9685_MODE1, (oldmode & 0x7F) | 0x10); // sleep
  write8(PCA9685_PRESCALE, prescale);
  write8(PCA9685_MODE1, oldmode);
  usleep(5000);
  write8(PCA9685_MODE1, oldmode | 0xa1); // wake + auto-increment
}

void Adafruit_MS_PWMServoDriver::setPWM(uint8_t num, uint16_t on,
                                        uint16_t off) {
  uint8_t buffer[5];
  buffer[0] = LED0_ON_L + 4 * num;
  buffer[1] = on;
  buffer[2] = on >> 8;
  buffer[3] = off;
  buffer[4] = off >> 8;
  i2c_dev->write(buffer, 5);
}

uint8_t Adafruit_MS_PWMServoDriver::read8(uint8_t addr) {
  uint8_t buffer[1] = {addr};
  i2c_dev->write_then_read(buffer, 1, buffer, 1);
  return buffer[0];
}

void Adafruit_MS_PWMServoDriver::write8(uint8_t addr, uint8_t d) {
  uint8_t buffer[2] = {addr, d};
  i2c_dev->write(buffer, 2);
}

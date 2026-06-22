#include "linux_i2c.h"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>

LinuxI2CDevice::LinuxI2CDevice(uint8_t addr, int busNum)
    : _addr(addr), _busNum(busNum), _fd(-1) {}

LinuxI2CDevice::~LinuxI2CDevice() {
  if (_fd >= 0)
    close(_fd);
}

bool LinuxI2CDevice::begin() {
  char path[20];
  snprintf(path, sizeof(path), "/dev/i2c-%d", _busNum);
  _fd = open(path, O_RDWR);
  if (_fd < 0)
    return false;
  if (ioctl(_fd, I2C_SLAVE, _addr) < 0) {
    close(_fd);
    _fd = -1;
    return false;
  }
  return true;
}

bool LinuxI2CDevice::write(const uint8_t* buf, size_t len) {
  return ::write(_fd, buf, len) == static_cast<ssize_t>(len);
}

bool LinuxI2CDevice::write_then_read(const uint8_t* wbuf, size_t wlen,
                                     uint8_t* rbuf, size_t rlen) {
  // The i2c_msg buf field is non-const by kernel convention; write buffer is
  // not modified.
  struct i2c_msg msgs[2] = {
      {.addr = _addr,
       .flags = 0,
       .len = static_cast<uint16_t>(wlen),
       .buf = const_cast<uint8_t*>(wbuf)},
      {.addr = _addr,
       .flags = I2C_M_RD,
       .len = static_cast<uint16_t>(rlen),
       .buf = rbuf},
  };
  struct i2c_rdwr_ioctl_data data = {.msgs = msgs, .nmsgs = 2};
  return ioctl(_fd, I2C_RDWR, &data) >= 0;
}

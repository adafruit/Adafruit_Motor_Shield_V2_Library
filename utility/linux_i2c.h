#pragma once

#include <cstddef>
#include <cstdint>

class LinuxI2CDevice {
 public:
  LinuxI2CDevice(uint8_t addr, int busNum);
  ~LinuxI2CDevice();
  bool begin();
  bool write(const uint8_t* buf, size_t len);
  bool write_then_read(const uint8_t* wbuf, size_t wlen, uint8_t* rbuf,
                       size_t rlen);

 private:
  uint8_t _addr;
  int _busNum;
  int _fd;
};

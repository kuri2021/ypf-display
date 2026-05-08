#pragma once
#include <Arduino.h>

class DWIN_DGUS {
public:
  DWIN_DGUS(): _ser(nullptr), _useCRC(false) {}
  void begin(Stream& s, uint32_t baud, bool useCRC=false);

  void writeRaw(uint16_t addr, const uint8_t* data, uint16_t lenBytes);
  void readRaw(uint16_t addr, uint16_t wordsToRead);

  void writeU16(uint16_t addr, uint16_t v);
  void writeS16(uint16_t addr, int16_t v);
  void writeAscii(uint16_t addr, const char* s);

private:
  Stream* _ser;
  bool _useCRC;
  uint16_t crc16(const uint8_t* p, uint16_t n);
  void sendCRC(const uint8_t* cmdPlusData, uint16_t n);
};
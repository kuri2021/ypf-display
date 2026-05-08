#include "DWIN_DGUS.h"

void DWIN_DGUS::begin(Stream& s, uint32_t /*baud*/, bool useCRC) {
  _ser = &s; _useCRC = useCRC;
}

uint16_t DWIN_DGUS::crc16(const uint8_t* p, uint16_t n){
  uint16_t crc = 0xFFFF;
  while(n--){
    crc ^= *p++;
    for (uint8_t i=0;i<8;i++) crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
  }
  return crc;
}

void DWIN_DGUS::sendCRC(const uint8_t* buf, uint16_t n){
  uint16_t c = crc16(buf, n);
  uint8_t lo = c & 0xFF, hi = c >> 8;
  _ser->write(lo); _ser->write(hi);
}

void DWIN_DGUS::writeRaw(uint16_t addr, const uint8_t* data, uint16_t len){
  uint8_t H[4] = {0x5A, 0xA5, (uint8_t)(1+2+len), 0x82};
  uint8_t A[2] = { (uint8_t)(addr>>8), (uint8_t)addr };
  _ser->write(H, sizeof(H));
  _ser->write(A, sizeof(A));
  _ser->write(data, len);
  if(_useCRC){
    uint8_t tmp[1+2+255]; uint16_t n=0;
    tmp[n++]=H[3]; tmp[n++]=A[0]; tmp[n++]=A[1];
    for(uint16_t i=0;i<len;i++) tmp[n++]=data[i];
    sendCRC(tmp, n);
  }
}

void DWIN_DGUS::readRaw(uint16_t addr, uint16_t words){
  uint8_t H[4] = {0x5A, 0xA5, (uint8_t)(1+2+2), 0x83};
  uint8_t A[2] = { (uint8_t)(addr>>8), (uint8_t)addr };
  uint8_t C[2] = { (uint8_t)(words>>8), (uint8_t)words };
  _ser->write(H, sizeof(H)); _ser->write(A, sizeof(A)); _ser->write(C, sizeof(C));
  if(_useCRC){ uint8_t t[5]={H[3],A[0],A[1],C[0],C[1]}; sendCRC(t,5); }
}

void DWIN_DGUS::writeU16(uint16_t addr, uint16_t v){
  uint8_t b[2] = {(uint8_t)(v>>8),(uint8_t)v}; writeRaw(addr,b,2);
}
void DWIN_DGUS::writeS16(uint16_t addr, int16_t v){
  writeU16(addr, (uint16_t)v);
}
void DWIN_DGUS::writeAscii(uint16_t addr, const char* s){
  uint16_t n = strlen(s);
  writeRaw(addr, (const uint8_t*)s, n);
}
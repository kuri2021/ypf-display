#include "sys.h"
#include "color.h"

void SetTextColorRed(u16 sp_addr) {
    u16 color = 0xC064;  // RGB565 빨강
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorBlack(u16 sp_addr) {
    u16 color = 0x18E3;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorYellow(u16 sp_addr) {
    u16 color = 0xFF60;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorBlue(u16 sp_addr) {
    u16 color = 0x051D;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorGray(u16 sp_addr) {
    u16 color = 0xCE59;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorWhite(u16 sp_addr) {
    u16 color = 0xFFFF;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}
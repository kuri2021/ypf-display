#ifndef __NORFLASH_H__
#define __NORFLASH_H__
#include "sys.h"
#include <string.h>
#define NORFLASH_READ 0x5A
#define NORFLASH_WRITE 0xA5

#define CACHE_ADDR 0xC000    // DGUS缓冲区
#define NORFLASH_ADDR 0x0008 // 系统变量地址
void Nor_Flash_write(u32 addr, u8 *buff, u16 len);
void vp_nor_flash_write(u32 nor_addr, u16 vp_addr, u16 num);
void Nor_Flash_read(u32 addr, u8 *buff, u16 len);
void vp_nor_flash_read(u32 nor_addr, u16 vp_addr, u16 num);
#endif
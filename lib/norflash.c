#include "include.h"

xdata u8 norflash_cmd[8];

// 写入到norflash中
void vp_nor_flash_write(u32 nor_addr, u16 vp_addr, u16 num)
{
    u8 nor_flash_cmd[8];
    nor_flash_cmd[0] = 0xA5;
    nor_flash_cmd[1] = (nor_addr >> 16) & 0xFF;
    nor_flash_cmd[2] = (nor_addr >> 8) & 0xFF;
    nor_flash_cmd[3] = nor_addr& 0xFF;
    nor_flash_cmd[4] = (vp_addr >> 8) & 0xFF;
    nor_flash_cmd[5] = vp_addr & 0xFF;
    nor_flash_cmd[6] = (num >> 8) & 0xFF;
    nor_flash_cmd[7] = num & 0xFF;

    write_dgus_vp(0x08, nor_flash_cmd, 4);

    while (1) {
        read_dgus_vp(0x08, nor_flash_cmd, 2);
        if (nor_flash_cmd[0] == 0) {
            break;
        }
        delay_ms(1);
    }
}

void Nor_Flash_write(u32 addr, u8 *buff, u16 len)
{
    write_dgus_vp(CACHE_ADDR, buff, len); // 将数据从缓冲区写到CACHE_ADDR中
    vp_nor_flash_write(addr, CACHE_ADDR, len);
  
}

// 从norflash中读取
void vp_nor_flash_read(u32 nor_addr, u16 vp_addr, u16 num)
{
    u8 nor_flash_cmd[8];

    nor_flash_cmd[0] = 0x5A;
    nor_flash_cmd[1] = (nor_addr >> 16) & 0xFF;
    nor_flash_cmd[2] = (nor_addr >> 8) & 0xFF;
    nor_flash_cmd[3] = nor_addr& 0xFF;
    nor_flash_cmd[4] = (vp_addr >> 8) & 0xFF;
    nor_flash_cmd[5] = vp_addr & 0xFF;
    nor_flash_cmd[6] = (num >> 8) & 0xFF;
    nor_flash_cmd[7] = num & 0xFF;

    write_dgus_vp(0x08, nor_flash_cmd, 4);

    while (1) {
        read_dgus_vp(0x08, nor_flash_cmd, 2);
        if (nor_flash_cmd[0] == 0) {
            break;
        }
        delay_ms(1);
    }
}

void Nor_Flash_read(u32 addr, u8 *buff, u16 len)
{
    vp_nor_flash_read(addr, CACHE_ADDR, len);
    read_dgus_vp(CACHE_ADDR, buff, len); // 将数据从CACHE_ADDR中读到buff中
}




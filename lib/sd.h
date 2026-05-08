///////////////////////////hal.h//////////////////////////
#include "sys.h"

#define SD_SCL  P30 //SD卡同步时钟  输入
#define SD_SI  P31 //SD卡同步数据  输入
#define SD_CS  P32 //SD卡片选 	  输入
#define SD_SO  P33 //SD卡同步数据  输出
#define RESET_OK 5
#define INIT_OK 6

#define DELAY_TIME 30 //SD卡的复位与初始化时SPI的延时参数，根据实际速率修改其值，否则会造成SD卡复位或初始化失败
#define NORMAL_DELAY 15
#define TRY_TIME 100   //向SD卡写入命令之后，读取SD卡的回应次数，即读TRY_TIME次，如果在TRY_TIME次中读不到回应，产生超时错误，命令写入失败

//错误码定义
//-------------------------------------------------------------
#define INIT_CMD0_ERROR     0x01 //CMD0错误
#define INIT_CMD1_ERROR     0x02 //CMD1错误
#define WRITE_BLOCK_ERROR   0x03 //写块错误
#define READ_BLOCK_ERROR    0x04 //读块错误
//-------------------------------------------------------------

u8 SD_Reset();
u8 SD_Init();
u8 SdReadSector(unsigned long addr,unsigned char *Buffer);
//unsigned long SwapINT32(unsigned long dData);

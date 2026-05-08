#include "sd.h"
#include "debug.h"

// 仅支持SD2.0
// 变量定义，并且只支持读SD。写SD有版权
//--------------------------------------------------------------
bit is_init; // 在初始化的时候设置此变量为1,同步数据传输（SPI）会放慢
//---------------------------------------------------------------

unsigned char bdata _dat;
sbit _dat7 = _dat ^ 7;
sbit _dat6 = _dat ^ 6;
sbit _dat5 = _dat ^ 5;
sbit _dat4 = _dat ^ 4;
sbit _dat3 = _dat ^ 3;
sbit _dat2 = _dat ^ 2;
sbit _dat1 = _dat ^ 1;
sbit _dat0 = _dat ^ 0;

void spi_init(void)
{
     SetPinOut(3, 0);
     SetPinOut(3, 1);
     SetPinOut(3, 2);
     SetPinIn(3, 3);
}

/******************************************************************
 - 功能描述：延时函数
 - 隶属模块：公开函数模块
 - 函数属性：外部，用户可调用
 - 参数说明：time:time值决定了延时的时间长短
 - 返回说明：无
 - 注：.....
 ******************************************************************/

void delay(unsigned int time)
{
     while (time--)
          ;
}

/******************************************************************
 - 功能描述：IO模拟SPI，发送一个字节
 - 隶属模块：SD卡模块
 - 函数属性：内部
 - 参数说明：x是要发送的字节
 - 返回说明：无返回
 - 注：其中is_init为1时，写的速度放慢，初始化SD卡SPI速度不能太高
 ******************************************************************/

void SD_spi_write(unsigned char x)
{
     _dat = x;

     SD_SI = _dat7;
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);

     SD_SI = _dat6;
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);

     SD_SI = _dat5;
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);

     SD_SI = _dat4;
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);

     SD_SI = _dat3;
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);

     SD_SI = _dat2;
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);

     SD_SI = _dat1;
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);

     SD_SI = _dat0;
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
}

/******************************************************************
 - 功能描述：IO模拟SPI，读取一个字节
 - 隶属模块：SD卡模块
 - 函数属性：内部
 - 参数说明：无
 - 返回说明：返回读到的字节
 ******************************************************************/

unsigned char SD_spi_read() // SPI读一个字节
{
     SD_SO = 1;

     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     _dat7 = SD_SO;

     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     _dat6 = SD_SO;

     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     _dat5 = SD_SO;

     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     _dat4 = SD_SO;

     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     _dat3 = SD_SO;

     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     _dat2 = SD_SO;

     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     _dat1 = SD_SO;

     SD_SCL = 1;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     SD_SCL = 0;
     if (is_init)
          delay(DELAY_TIME);
     else
          delay(NORMAL_DELAY);
     _dat0 = SD_SO;

     return (_dat);
}

/******************************************************************
 - 功能描述：向SD卡写命令
 - 隶属模块：SD卡模块
 - 函数属性：内部
 - 参数说明：SD卡的命令是6个字节，pcmd是指向命令字节序列的指针
 - 返回说明：命令写入后，SD卡的回应值，调用不成功，将返回0xff
 ******************************************************************/

unsigned char SD_Write_Cmd(unsigned char *pcmd) // 向SD卡写命令，pcmd是命令字节序列的首地址
{
     unsigned char temp, time = 0;
     u16 tmp16 = 0;

     SD_CS = 1;
     SD_spi_write(0xff); // 提高兼容性，如果没有这里，有些SD卡可能不支持
     SD_CS = 0;
     SD_spi_write(pcmd[0]);
     SD_spi_write(pcmd[1]);
     SD_spi_write(pcmd[2]);
     SD_spi_write(pcmd[3]);
     SD_spi_write(pcmd[4]);
     SD_spi_write(pcmd[5]);
     do
     {
          temp = SD_spi_read(); // 一直读，直到读到的不是0xff或超时
          time++;
     } while ((temp == 0xff) && (time < TRY_TIME));
     return (temp);
}

/******************************************************************
 - 功能描述：复位SD卡，用到CMD0，使用SD卡切换到SPI模式
 - 隶属模块：SD卡模块
 - 函数属性：内部
 - 参数说明：无
 - 返回说明：调用成功，返回0x00，否则返回INIT_CMD0_ERROR (sd.h中有定义)
 ******************************************************************/

unsigned char SD_Reset() // SD卡复位，进入SPI模式，使用CMD0（0号命令）
{
     unsigned char temp, i;
     unsigned char pcmd[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
     u16 tryTime;

     spi_init();

     is_init = 1; // Set the init flag

     SD_CS = 1;
     for (i = 0; i < 0x0f; i++) // 初始时，首先要发送最少74个时钟信号，这是必须的！！！
     {
          SD_spi_write(0xff); // 120 clk
     }

     SD_CS = 0;
     tryTime = 0;
     do
     {
          temp = SD_Write_Cmd(pcmd); // 写入CMD0
          tryTime++;
          if (tryTime == TRY_TIME)
          {
               SD_CS = 1;
               return (INIT_CMD0_ERROR); // CMD0写入失败
          }
          // DEBUGINFO("第%d次尝试\n",tryTime);
     } while (temp != 0x01);

     SD_CS = 1;
     SD_spi_write(0xff); // 按照SD卡的操作时序在这里补8个时钟
     // DEBUGINFO("执行完40命令\n");
     return RESET_OK; // 返回5,说明复位操作成功
}

/******************************************************************
 - 功能描述：初始化SD卡，使用CMD1
 - 隶属模块：SD卡模块
 - 函数属性：内部
 - 参数说明：无
 - 返回说明：调用成功，返回0x00，否则返回INIT_CMD1_ERROR (sd.h中有定义)
 ******************************************************************/

unsigned char SD_Init() // 初始化，使用CMD1（1号命令）
{
     u16 tmp16 = 0;
     unsigned char time, temp;
     unsigned char pcmd[] = {0x48, 0x00, 0x00, 0x01, 0xaa, 0x87};

     SD_CS = 0;
     time = 0;
     do
     {
          temp = SD_Write_Cmd(pcmd);
          time++;
          if (time == TRY_TIME)
          {
               SD_CS = 1;
               return (INIT_CMD1_ERROR); // CMD8写入失败
          }
     } while (temp != 1);

     time = 0;
     do
     {
          time++;
          SD_spi_read();
          SD_spi_read();
          SD_spi_read();
          SD_spi_read();
          pcmd[0] = 0x77;
          pcmd[3] = 0x00;
          pcmd[4] = 0x00;
          pcmd[5] = 0xff;
          temp = SD_Write_Cmd(pcmd);
          pcmd[0] = 0x69;
          pcmd[1] = 0x40;
          if (time == TRY_TIME)
          {
               SD_CS = 1;
               return (INIT_CMD1_ERROR); // CMD55写入失败
          }
          tmp16 = SD_Write_Cmd(pcmd);
          // DEBUGINFO("69返回值%d\n",tmp16);
     } while (0 != tmp16);

     is_init = 0; // 初始化完毕，将is_init设置为0,为了提高以后的数据传输速度

     SD_CS = 1; // 关装SD卡的片选

     SD_spi_write(0xff); // 按照SD卡的操作时序在这里补8个时钟

     return (INIT_OK); // 返回0,说明初始化操作成功
}

/****************************************************************************
 - 功能描述：读取addr扇区的512个字节到buffer指向的数据缓冲区
 - 隶属模块：SD卡模块
 - 函数属性：内部
 - 参数说明：addr:扇区地址
             buffer:指向数据缓冲区的指针
 - 返回说明：调用成功，返回0x00，否则返回READ_BLOCK_ERROR (sd.h中有定义)
 - 注：SD卡初始化成功后，读写扇区时，尽量将SPI速度提上来，提高效率
 ****************************************************************************/

unsigned char SdReadSector(unsigned long addr, unsigned char *Buffer) // 从SD卡的指定扇区中读出512个字节，使用CMD17（17号命令）
{
     unsigned int j;
     unsigned char time, temp;
     unsigned char pcmd[] = {0x51, 0x00, 0x00, 0x00, 0x00, 0xFF}; // CMD17的字节序列
     u16 tmp16 = 0;
     u32 tmp32 = 0;

     pcmd[1] = ((addr & 0xFF000000) >> 24);
     pcmd[2] = ((addr & 0x00FF0000) >> 16);
     pcmd[3] = ((addr & 0x0000FF00) >> 8);
     pcmd[4] = (addr & 0x000000FF);

     SD_CS = 0; // 打开片选
     time = 0;
     do
     {
          pcmd[0] = 0x77;
          temp = SD_Write_Cmd(pcmd);
          pcmd[0] = 0x51;
          temp = SD_Write_Cmd(pcmd); // 写入CMD17
          time++;
          if (time == TRY_TIME)
          {
               SD_CS = 1;
               return (READ_BLOCK_ERROR); // 读块失败
          }
     } while (temp != 0);

     time = 0;
     do
     {
          time++;
          if (time == TRY_TIME)
          {
               SD_CS = 1;
               return (READ_BLOCK_ERROR); // 读块失败
          }
     } while (SD_spi_read() != 0xfe);

     for (j = 0; j < 512; j++) // 将数据写入到数据缓冲区中
     {
          Buffer[j] = SD_spi_read();
     }

     SD_spi_read();
     SD_spi_read(); // 读取两个字节的CRC校验码，不用关心它们

     SD_CS = 1; // SD卡关闭片选

     SD_spi_write(0xFF); // 按照SD卡的操作时序在这里补8个时钟

     return 0;
}

#include "debug.h"
#include "uart.h"
#define USEUART UART4

#define USE_LOG 1
#define TAG "DEBUG"
#if (USE_LOG && DEBUG)
#define LOG mylog
#else
#define LOG /\
/LOG
#endif

// #define UART485
#ifdef UART485
sbit TR5 = P0 ^ 1;
#endif
void initDebugUart(void)
{
#if USEUART == UART2
    u16 tmp;
    tmp = 1024 - FOSC / 64 / 115200;
    MUX_SEL |= 0X40;
    SetPinOut(0, 4);
    SetPinIn(0, 5);
    P0 |= 0x30;
    ADCON = 0x80;
    SCON0 = 0x50;
    SREL0H = (u8)(tmp >> 8);
    SREL0L = (u8)tmp;
    // IEN0 |= 0x10;
    ES0 = 0; // 关闭发送中断
    SBUF0 = '\r';
#elif USEUART == UART3
    u16 tmp;
    tmp = 1024 - FOSC / 32 / 115200;
    MUX_SEL |= 0X20;
    SetPinOut(0, 6);
    SetPinIn(0, 7);
    P0 |= 0xC0;

    SCON1 = 0X90;
    SREL1H = (u8)(tmp >> 8);
    SREL1L = (u8)tmp;
    SBUF1 = '\r';
#elif (USEUART == UART4)
    u16 tmp;
    SCON2T = 0X80;
    SCON2R = 0X80;
    tmp = FOSC / 8 / 115200;
    BODE2_DIV_H = (u8)(tmp >> 8);
    BODE2_DIV_L = (u8)tmp;
    SBUF2_TX = '\r';
    IEN1 &= 0xF3;
#elif (USEUART == UART5)
    UartInit(UART5, 115200);
    IEN1 &= (~0x10);
    SBUF3_TX = '\r';
#endif
}
#if DEBUG == 1
char putchar(char c)
{
#if USEUART == UART2
    while (TI0 == 0)
        ;
    TI0 = 0;
    SBUF0 = c;
    return (SBUF0 = c);
#elif USEUART == UART3
    while ((SCON1 & 0X02) == 0)
        ;
    SCON1 &= 0XFC;
    SCON1 &= 0XFC;
    return (SBUF1 = c);
#elif (USEUART == UART4)
    while ((SCON2T & 0X01) == 0)
        ;
    SCON2T &= 0XFE;
    return (SBUF2_TX = c);
#elif (USEUART == UART5)
    while ((SCON3T & 0x01) == 0x00)
        ;
    SCON3T &= 0xFE;
#ifdef UART485
    TR5 = 1;
#endif
    return (SBUF3_TX = c);
#endif
}
#else
#include "STRING.H"
char logbuff[1024];
// char logbuff2[2048];
#endif
void mylog(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#if DEBUG == 2
    {
        char *buff = logbuff;
        memset(logbuff, 0, sizeof(logbuff));
        vsprintf(buff, fmt, args);
        do
        {
            if (strstr(buff, "message_id"))
            {
                buff[1023] = '0';
            }
        } while (0);
    }
#elif DEBUG == 1
    vprintf(fmt, args);
#endif
    va_end(args);
}
/* void mylogtag(const char *tag, unsigned long line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#if DEBUG == 2
    {
        char *buff = logbuff;
        memset(logbuff, 0, sizeof(logbuff));
        vsprintf(buff, fmt, args);
        do
        {
            if (strstr(buff, "message_id"))
            {
                buff[1023] = '0';
            }
        } while (0);
    }
#elif DEBUG == 1
    // 可以在此处过滤tag
    printf("[%s][%lu]:", tag, line);
    vprintf(fmt, args);
#endif
    va_end(args);
} */

void abort()
{
    LOG("---------------------------------Abort ING-------------------------------");
    while (1)
    {
    }
}

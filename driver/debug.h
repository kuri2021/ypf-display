#ifndef __DEBUG__H___
#define __DEBUG__H___
#include "sys.h"
#include "stdio.h"
#include <stdarg.h>
#include "ASSERT.H"
/*
DEBUG :0=不使用 1=实机使用 2=仿真使用
仿真使用:在logbuff中可查看打印的信息
LOG_TAG、LOG_STR、LOG_NUM、LOG_HEX需要在.c文件中定义TAG才能使用
*/
#define DEBUG 0
#if DEBUG == 1
#define DEBUGINIT initDebugUart
#elif (DEBUG == 0 || DEBUG == 2)
#define DEBUGINIT()
#endif

#if DEBUG

#define LOG_ASSERT(expr)                                                               \
    do                                                                                 \
    {                                                                                  \
        if (expr)                                                                      \
        {                                                                              \
            ;                                                                          \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            LOG("[%s][%lu]Assert failed: " #expr " \n", TAG, (unsigned long)__LINE__); \
            abort();                                                                   \
        }                                                                              \
    } while (0)
#else
#define LOG /\
/LOG
#define LOG_ASSERT(expr) \
    do                   \
    {                    \
        ;                \
    } while (0)
#endif

#define _LOG_BASE_INFO() LOG("[%s][%lu]:", TAG, (unsigned long)__LINE__)
#define LOG_TAG       \
    _LOG_BASE_INFO(); \
    LOG
#define LOG_STR(str)  \
    _LOG_BASE_INFO(); \
    LOG(#str "=%s\r\n", str);
#define LOG_NUM(num)  \
    _LOG_BASE_INFO(); \
    LOG(#num "=%ld\r\n", (unsigned long)num);
#define LOG_HEX(hex)  \
    _LOG_BASE_INFO(); \
    LOG(#hex "=0x%lx\r\n", (unsigned long)hex);
#if DEBUG == 2
extern char logbuff[];
#endif
/* 初始化debug串口 */
void initDebugUart(void);
/* 打印到缓冲区*/
void mylog(const char *fmt, ...);
/* 异常 */
void abort(void);

#endif
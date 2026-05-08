#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include "T5LOS8051.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sys.h"
#include "timer.h"
#include "umath.h"
#include "uart.h"
#include "iic.h"
#include "rtc.h"
#include "norflash.h"

// #include "modbus.h"
#include "dwin8283.h"
#include "canbus.h"
// #include "const.h"
#include "ui.h"
#include "debug.h"
#include "history.h"

#define RTCUSE_RX8130 1 //(0: SD2058  1: RX8130)
/***********
 * 软定时器
 * 0-500ms RTC读取
 * 1-20ms fun功能函数
 * 2-5s 屏保
 * 3-200ms   看门狗喂狗
 **********/
#define RTC_Timer 0
#define RTC_timer_ms 500
#define FUNC_Timer 1
#define FUNC_Timer_ms 20
#define PB_Timer 2
#define PB_Timer_ms 5000 // 计时1s
#define WTDRESET_Timer 3
#define WTDRESET_Timer_ms 200
/***********NorFlash**********/
#define USER_ID 0 // Norflash存储块对应地址0x800倍数
#define USER_VP 0xB000
#define NOR_FLASH_LEN 2048 // 字长
#define NorFlashFirstPowerOn 0x1234
/***********DWIN8283**********/
#define NUM 3
#define MAX_BYTE_LEN 512
#define MIN_CHECK_LEN 6
#define FRAME_HEAD 0x5AA5
#define WRITE_VP 0x82
#define READ_VP 0x83
#define WRITE_CURVE 0x84
#define WRITE_DW_VP 0x86
#define READ_DW_VP 0x87

// #define ENABLE_CRC
#define AUTO_UPLOAD
// #define CAN8283
// #define CAN8283_FRAME_ID 1
#define UART28283
#define UART28283_FRAME_ID 0
// #define UART38283
// #define UART38283_FRAME_ID 1
#define UART48283
#define UART48283_FRAME_ID 1
#define UART58283
#define UART58283_FRAME_ID 2
extern u8 SendTmp[MAX_BYTE_LEN];
/*********************/
#define WTDONstart 1 // 看门狗启用，0-不用，1用
#if WTDONstart
extern u16 WTD_start_time;
extern u16 WTD_start_state;
#define WTD_start_timelim 2000 // 上电后2000ms开启看门狗
#endif

#endif

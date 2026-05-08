#ifndef __HISTORY_H__
#define __HISTORY_H__
#include "include.h"

#define DATA_NUM 720 // 历史数据个数
typedef struct _HISTORICAL_DATA_
{
    u16 flag;  // 标志
    u16 hnum;  // 数量计数
    u8 hyear;  // 时间戳-年
    u8 hmonth; // 时间戳-月
    u8 hdate;  // 时间戳-日
    u8 hhour;  // 时间戳-时
    u8 hmin;   // 时间戳-分
    u8 hsec;   // 时间戳-秒
    u16 tempt; // 温度
    u16 humi;  // 湿度
} HISTORICAL_DATA;

// 1块flash 4096字节
typedef struct _HISTORICAL_ERR_
{
    u16 flag;        // 标志
    u16 hnum;        // 数量计数
    u8 hyear;        // 时间戳-年
    u8 hmonth;       // 时间戳-月
    u8 hdate;        // 时间戳-日
    u8 hhour;        // 时间戳-时
    u8 hmin;         // 时间戳-分
    u8 hsec;         // 时间戳-秒
    u16 faultNumber; // 故障编号
} HISTORICAL_ERR;
extern HISTORICAL_DATA hisData[];
extern HISTORICAL_ERR hisErr[];

#define ErrVP 0x2000 // 报警地址
#define ErrTxtVP 0x1000 // 报警表格起始地址
#define ErrMaxNum 30 // 报警最大条数
extern u8 ErrBuf;    // 实时故障数值
extern u8 ErrBufOld; // 故障地址上一次数值
extern u16 ErrNum;   // 故障数量

void historyErrInit(void);
void ErrFun(void);

#endif

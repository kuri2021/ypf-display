#ifndef __HMI_H__
#define __HMI_H__

#include "sys.h"

//#pragma once

// ===== Board -> Display (측정값: 0x8000~) =====
#define VP_TT        0x8000
#define VP_TB        0x8002-*
#define VP_P         0x8004
#define VP_NTT       0x8006
#define VP_NTB       0x8008
#define VP_ST        0x800A
#define VP_FLAGS     0x800C
#define VP_ERR_CODE  0x8010
#define VP_ERR_ARG   0x8012

// ===== 당신 UI의 표시용 VP (필요에 맞게 바꾸세요) =====
#define UI_VP_SHOW_TT     0x2820
#define UI_VP_SHOW_TB     0x2830
#define UI_VP_SHOW_P_X10  0x2810
#define UI_VP_SHOW_STATE  0x2840
#define UI_VP_SHOW_ERR    0x2870

s16 ad_top_tep(void);
s16 ad_bottom_tep(void);
void UI_UpdateTempsFromArduino(void);
void HMI_Service_200ms(void);

typedef struct {
    s16 t_top_x10;
    s16 t_bot_x10;
    u16 p_x100;
    s16 ntc_top_x10;
    s16 ntc_bot_x10;
    u16 stage;
    u16 flags;
} HmiStatus_t;

typedef struct {
    u16 set_t_x10;
    u16 set_p_x100;
    u16 hold_sec;
} HmiSet_t;

extern HmiStatus_t g_hmiStat;
extern HmiSet_t    g_hmiSet;

void HMI_ReadStatus(void);
void HMI_ReadSettings(void);
void HMI_WriteSettings(u16 setT_x10, u16 setP_x100, u16 hold_s);
/* 
   void HMI_Poll_200ms(u32 nowMs);
*/
#endif
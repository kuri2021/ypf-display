#ifndef __PAGE_H__
#define __PAGE_H__
#include "sys.h"


void Picture1213_Init(void);
void Picture12_Change(u16 n);
void Picture13_Change(u16 n);
void Page_Change_Handler(u8 n);
void encoder_page_change(u16 state);
void Text_Select_BySP(u16 sp_addr);
void Text_Color_Change(u16 color_rgb565);
void UI_Service(void);
void UI_Read500ms_Start(void);
void SecCnt_TickTask(void);
void SecCnt_Start120(u16 time);
void write_dgus_vp_check(void);
void test_vp(void);
extern u8 page_number;
#endif


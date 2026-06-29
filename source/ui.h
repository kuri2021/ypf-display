#ifndef __PAGE_H__
#define __PAGE_H__
#include "sys.h"


void Init(void);
void Page_Change_Handler(u8 n);
void encoder_page_change(u16 state);
void SecCnt_TickTask(void);
void write_dgus_vp_check(void);
extern u8 page_number;
#endif


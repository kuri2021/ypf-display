#ifndef __ENCODER_H__
#define __ENCODER_H__
#include "sys.h"

sbit ENCODER_PUSH=P1^5;

u16 Encoder_recevie(void);
u16 Encoder_Set_Value(u16 unit_value,u16 upper_limit_value,u16 lower_limiting_value,u16 set_value);
#endif
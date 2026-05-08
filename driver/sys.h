
/******************************************************************************

                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : sys.h
  �� �� ��   : V1.0
  ��    ��   : chenmeishu
  ��������   : 2019.9.2
  ��������   : 
  �޸���ʷ   :
  1.��    ��   : 
    ��    ��   : 
    �޸�����   : 
******************************************************************************/
#ifndef __SYS_H__
#define __SYS_H__
#include "T5LOS8051.h"
#include <stdio.h>

#define UnusedFunction	0	//ûʹ�õĺ���


/*****************************************
*����0X16��ַ�жϻ����Լ�����¼�
*****************************************/
#define IDLE_PRESS     0//δ������Ļ
#define FIRST_PRESS    1//�״ΰ���
#define UNDER_PRESS  	 2//������ѹ
#define RELEASE_PRESS  3//̧��


#define UNTOUCH 0              //δ���
#define UNSLIDE 1              //δ����
#define CLICKTOUCH 2           //����¼�
#define VERTICAL_SLIDE 3       //���򻬶��¼�
#define HORIZONTAL_SLIDE 4     //���򻬶��¼�
#define AUTOVERTICAL_SLIDE 5   //���򻬶��¼�����
#define AUTOHORIZONTAL_SLIDE 6 //���򻬶��¼�����
#define SLIDE_THRESHOLD 9 //XY�ı�������ص��㻬��
#define ANGLE_THRESHOLD 2 //���򻬶������򻬶���tanֵ��cotֵ

/*************************************************************
  memory assignment(0x0000 ~ 0xffff)

  0x0000~0x0fff  system reserved
	0x1000~0xffff  user   area
**********************************************************/
/*****************************************
*	    ϵͳ�ӿڱ�����ַ�궨�� (0x0000 0x03ff  )   
*****************************************/
#define		NOR_FLASH				0x0008
#define		SOFT_VERSION		0x000F
#define		RTC						  0x0010
#define		PIC_NOW					0x0014
#define		TP_STATUS				0x0016
#define		LED_NOW					0x0031
#define		AD_VALUE				0x0032
#define   LCD_HOR         0x007A
#define   LCD_VER         0x007B
#define		LED_CONFIG			0x0082
#define		PIC_SET					0x0084
#define 	RTC_Set					0x009C   
#define   RWFLASH         0x00AA
#define   SIMULATE_TP     0x00D4
#define   ENABLE_CURSOR   0x00D8
#define   SYS_CONFIG      0x0080
#define   AUDIO_PLAY      0X00F0


/*****************************************
*			wifi�ӿڱ�����ַ�궨��        *
*****************************************/
#define   WIFI_SWITCH            0x0400
#define		RMA						         0x0401
#define		EQUIPMENT_MODEL			   0x0416
#define		QR_CODE					       0x0450
#define		DISTRIBUTION_NETWORK	 0x0498
#define		MAC_ADDR				       0x0482
#define		WIFI_VER				       0x0487
#define		DISTRIBUTION_STATUS		 0x04A1
#define		NETWORK_STATUS			   0x04A2
#define		RTC_NETWORK				     0x04AC
#define		SSID					         0x04B0
#define		WIFI_PASSWORD			     0x04C0
#define   FLASH_ACCESS_CYCLE     50

/*****************************************
*			�ж����Ƿ����VP      *
*****************************************/
//#define INTVPACTION


/***********************************************************
0x1000     0xffff �û������ռ�
******************************************************/
/*****************************************
*	     NORFLASH(eeprom)    read to RAM ,RAM address          *
*****************************************/
/***************************************************
	UI 
**************************************************/

#define FOSC     206438400UL
#define T1MS    (65536-FOSC/12/1000)

#define   READ_CARD_OK      0
#define   READ_CARD_ERR     1
#define   WITHOUT_CARD      2
#define   DATA_READY        1
typedef   signed   char s8;
typedef   signed	 int  s16;
typedef   signed   long s32;
typedef unsigned   char u8;
typedef unsigned   int  u16;
typedef unsigned   long u32;
#ifndef NULL
 #define NULL ((void *) 0)
#endif
	
typedef struct _mNORFLASH
{
	u8 Mode;   //0x5a=�����ݣ�0XA5����д����
	u32 FLAddr;  //flash��ʼ��ַ������Ϊż��
	u16	VPAddr;	 //VP��ʼ��ַ������Ϊż��
	u16 Len;		 //�����ֳ��ȣ�����Ϊż��
	u8 *Buf;		 //��д���ݻ���ָ��
}MNORFLASH;

typedef struct _mSPIFLASH
{
	u8 Mode;	//5Aʹ��SPI����
	u16 ID;			//������ID�ţ����ڶ�Ϊ16-31
	u32 FLAddr;	//256K�е�ĳ����ַ������Ϊż��
	u16	VPAddr;	//VP��ַ������Ϊż��
	u16 Len;		//��������
	u8 *Buf;		//���ݴ��ָ��
	u16 Delay;	//ִ����д������GUI��ʱʱ��
}MSPIFLASH;

typedef struct _mMUSIC
{
	u8 IdNmu[32];
	u8 PlayTail;
	u8 PlayHead;
	u8 PlayInterrupt;
}MMUSIC;

extern MNORFLASH Nor_Flash;
extern MSPIFLASH Spi_Flash;
extern MMUSIC MusicBuf;
extern s16 PosXChangeSpeed,PosYChangeSpeed,RealPosX,RealPosY;
extern s16 xdistance,ydistance,PressPosX,PressPosY,LastPosX,LastPosY;
extern u16 PressPageID;
extern u8 PressPosChange;
	
void INIT_CPU(void);
void write_dgus_vp(u16 addr,u8* buf,u16 len);
void read_dgus_vp(u16 addr,u8* buf,u16 len);
void write_dgus_vp_new(u16 addr, u8 *buf, u16 len);  /* len = 바이트 수 */
void write_vp_u16_be(u16 vp, u16 value);             /* 16-bit 값을 빅엔디안으로 쓰기 */

void delay_us(unsigned int t);
void delay_ms(unsigned int t);
void SetPinOut(u8 Port,u8 Pin);
void SetPinIn(u8 Port,u8 Pin);
void  PinOutput(u8 Port,u8 Pin,u8 value);
u8 GetPinIn(u8 Port,u8 Pin);
void Page_Change(u16 PageID);
void ClearRAM(void);

/*************************************/
void TouchSwitch(u16 PageID, u8 TouchType, u8 TouchID, u8 Status);
void NorFlash_Action(void);
void SPIFlash_Action(void);
u16 GetPageID();
void ResetT5L(void);
void BackLight_Control(u8 light);
void MusicPlay(u8 MusicId);
void system_config(u8 is_beep,u8 is_sleep);
void system_led_config(u8 sleep_light, u16 await_time);
u8 ScreenTouchOrNot(void);//��ȡ��Ļ�Ƿ񱻴��� 0δ������Ļ   1�д�����Ļ
u8 GetTouchStatus(void);
u8 GetTouchAnction(u16 PageID);
void setPopupMessage(u8 *sourceStr,u16 len);
void PopupMessageTimeOutClose(void);
void PWM0_Set(u8 pwm_psc,u16 pwm0_precision);
void PWM0_Out(u16 duty_cycle,u16 pwm0_precision);
void system_draw_dynamic_graph(u8 chart_id, u16 axit_y);
void modifyInceaseControlPara(u8 page_id,u8 id,u16 minLimit,u16 step,u16 maxLimit);
u16 readADC(u16 channel);
void readSPIFlashToDgus(u32 spiAddr,u16 spiID,u16 vpAddr,u16 len,u8 *buff);
void writeCodeToSPIFlash(u16 spiID,u16 vpAddr);
void analogTouch(u8 touchid);






#endif
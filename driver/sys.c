
/******************************************************************************

                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : sys.c
  �� �� ��   : V1.0
  ��    ��   : chenmeishu
  ��������   : 2019.9.2
  ��������   : 
  �޸���ʷ   :
  1.��    ��   : 
    ��    ��   : 
    �޸�����   : 
******************************************************************************/
#include "sys.h"
MNORFLASH Nor_Flash;
MSPIFLASH Spi_Flash;
MMUSIC MusicBuf;
s16 PosXChangeSpeed,PosYChangeSpeed,RealPosX,RealPosY;
s16 xdistance,ydistance,PressPosX,PressPosY,LastPosX,LastPosY;
u16 PressPageID;
u8 PressPosChange,Touchstate;

/*****************************************************************************
 �� �� ��  : void read_dgus_vp(u32 addr,u8* buf,u16 len)
 ��������  : ��dgus��ַ��ֵ
 �������  :	 addr��dgus��ֵַ  len�������ݳ���
 �������  : buf�����ݱ��滺����
 �޸���ʷ  :
  1.��    ��   : 2019��4��2��
    ��    ��   : chengjing
    �޸�����   : ����
*****************************************************************************/
#define INTVPACTION
void read_dgus_vp(u16 addr, u8 *buf, u16 len)
{
	u16 OS_addr = 0;
	u16 OS_addr_offset = 0;
	u16 OS_len = 0, OS_len_offset = 0;
	u32 LenLimit;
	
	if(0==len)
		return;
	LenLimit = 0xffffU - addr + 1;
	if(LenLimit < len)
	{
		len = LenLimit;
	}
	OS_addr = addr >> 1;
	OS_addr_offset = addr & 0x01;
#ifdef INTVPACTION	
	EA = 0;
#endif
	ADR_H = 0;
	ADR_M = (u8)(OS_addr >> 8);
	ADR_L = (u8)OS_addr;
	ADR_INC = 1;	
	RAMMODE = 0xAF; 
	while (!APP_ACK);			
	if (OS_addr_offset)
	{
		APP_EN = 1;
		while (APP_EN);
		*buf++ = DATA1;
		*buf++ = DATA0;
		len--;
	}
	OS_len = len >> 1;
	OS_len_offset = len & 0x01;
	while (OS_len--)
	{
		APP_EN = 1;
		while (APP_EN);
		*buf++ = DATA3;
		*buf++ = DATA2;
		*buf++ = DATA1;
		*buf++ = DATA0;
	}
	if (OS_len_offset)
	{
		APP_EN = 1;
		while (APP_EN);
		*buf++ = DATA3;
		*buf++ = DATA2;
	}
	RAMMODE = 0x00;
#ifdef INTVPACTION	
	EA = 1;
#endif
}

/*void read_dgus_vp(u32 addr,u8* buf,u16 len)
{
    u32 xdata OS_addr;
	  u32 xdata OS_addr_offset;
    u16 xdata OS_len;
	  u16 xdata OS_len_offset;
	
		if(addr >= 0x10000)
			return;
		EA = 0;
    OS_addr=addr/2;
    OS_addr_offset=addr%2;
    ADR_H=(u8)(OS_addr>>16)&0xFF;
    ADR_M=(u8)(OS_addr>>8)&0xFF;
    ADR_L=(u8)OS_addr&0xFF;
    ADR_INC=1;                 //DGUS  OS�洢����ַ�ڶ�д���Զ���1
    RAMMODE=0xAF;               //������ģʽ
    if(OS_addr_offset==1)       //�׵�ַ��ƫ�ƣ�����
    {
        while(APP_ACK==0);      //�ȴ�
        APP_EN=1;
        while(APP_EN==1); 
        *buf++=DATA1;
        *buf++=DATA0;              
        len--;
        OS_addr_offset=0;
    }
    OS_len=len/2;
    OS_len_offset=len%2;
    if(OS_len_offset==1)
    {
         OS_len++;
    }
    while(OS_len--)
    {
        if((OS_len_offset==1)&&(OS_len==0))
        {          
            while(APP_ACK==0);
            APP_EN=1;
            while(APP_EN==1);       //��дִ�н���
            *buf++=DATA3;
            *buf++=DATA2;           
            break;    
        } 
        while(APP_ACK==0);
        APP_EN=1;
        while(APP_EN==1);       //��дִ�н��� 
        *buf++=DATA3;
        *buf++=DATA2;
        *buf++=DATA1;
        *buf++=DATA0;                          
    }   
    RAMMODE=0x00;           //��д��ɺ�RAMMODE��������
		EA = 1;
		delay_us(100);
}*/


/*****************************************************************************
 �� �� ��  : void write_dgus_vp(u32 addr,u8* buf,u16 len)
 ��������  : дdgus��ַ����
�������  :	 addr��д��ֵַ	buf��д������ݱ��滺���� len���ֳ���
 �������  : 
 �޸���ʷ  :
  1.��    ��   : 2019��4��2��
    ��    ��   : chengjing
    �޸�����   : ����
*****************************************************************************/
void write_dgus_vp(u16 addr, u8 *buf, u16 len)
{
	u16 OS_addr = 0;
	u16 OS_addr_offset = 0;
	u16 OS_len = 0,OS_len_offset = 0;
	u16 LenLimit;
	
	if(0==len)
		return;
	LenLimit = 0xffffU - addr + 1;
	if(LenLimit < len)
	{
		len = LenLimit;
	}
	OS_addr = addr >> 1;
	OS_addr_offset = addr & 0x01;
#ifdef INTVPACTION	
	EA = 0;
#endif
	ADR_H = 0;
	ADR_M = (u8)(OS_addr >> 8);
	ADR_L = (u8)OS_addr;
	ADR_INC = 0x01; 
	RAMMODE = 0x83;
	while (!APP_ACK);
	if (OS_addr_offset)
	{
		DATA1 = *buf++;
		DATA0 = *buf++;
		APP_EN = 1;
		while (APP_EN);
		len--;
	}
	OS_len = len >> 1;
	OS_len_offset = len & 0x01;
	RAMMODE = 0x8F;
	while (OS_len--)
	{
		DATA3 = *buf++;
		DATA2 = *buf++;
		DATA1 = *buf++;
		DATA0 = *buf++;
		APP_EN = 1;
		while (APP_EN);
	}
	if (OS_len_offset)
	{
		RAMMODE = 0x8C;
		DATA3 = *buf++;
		DATA2 = *buf++;
		APP_EN = 1;
		while (APP_EN);
	}
	RAMMODE = 0x00;
#ifdef INTVPACTION	
	EA = 1;
#endif
}


/* sys.c — 새 버전 구현 (len = 바이트 수) */
void write_dgus_vp_new(u16 addr, u8 *buf, u16 len)
{
    u16 LenLimit;
    u16 OS_addr;
    u8  OS_addr_offset;
    u16 OS_len4;
    u8  rem2;

    if (len == 0) return;

    /* 주소 범위 방어 */
    LenLimit = 0xFFFFu - addr + 1u;
    if (LenLimit < len) {
        len = LenLimit;
    }

    /* 워드 단위 주소/오프셋 계산 */
    OS_addr        = (u16)(addr >> 1);   /* word index */
    OS_addr_offset = (u8)(addr & 0x01);  /* byte offset (0 or 1) */

#ifdef INTVPACTION
    EA = 0;                             /* 필요 시 인터럽트 잠금 */
#endif

    ADR_H   = 0;
    ADR_M   = (u8)(OS_addr >> 8);
    ADR_L   = (u8)(OS_addr);
    ADR_INC = 0x01;

    /* 선행 정렬 */
    RAMMODE = 0x83;
    while (!APP_ACK);

    /* 만약 시작 바이트가 홀수면, 먼저 2바이트 처리 */
    if (OS_addr_offset) {
        if (len < 2) {                 /* 비정상 요청 방어 */
            RAMMODE = 0x00;
#ifdef INTVPACTION
            EA = 1;
#endif
            return;
        }
        /* 2바이트(1워드) */
        DATA1 = *buf++;                /* 상위 바이트 */
        DATA0 = *buf++;                /* 하위 바이트 */
        APP_EN = 1; while (APP_EN);
        len -= 2;
    }

    /* 4바이트 블록(= 2워드) 반복 */
    OS_len4 = (u16)(len >> 2);         /* 4-byte block count */
    RAMMODE = 0x8F;
    while (OS_len4--) {
        DATA3 = *buf++;
        DATA2 = *buf++;
        DATA1 = *buf++;
        DATA0 = *buf++;
        APP_EN = 1; while (APP_EN);
    }

    /* 2바이트 남은 경우 처리 */
    rem2 = (u8)((len & 0x02) ? 1 : 0);
    if (rem2) {
        RAMMODE = 0x8C;
        DATA3 = *buf++;
        DATA2 = *buf++;
        APP_EN = 1; while (APP_EN);
    }

    RAMMODE = 0x00;

#ifdef INTVPACTION
    EA = 1;
#endif
}

/* 16-bit 한 개를 빅엔디안(High, Low)으로 안전하게 쓰는 헬퍼 */
void write_vp_u16_be(u16 vp, u16 value)
{
    u8 be[2];
    be[0] = (u8)(value >> 8);
    be[1] = (u8)(value & 0xFF);
    write_dgus_vp_new(vp, be, 2);      /* 2 bytes */
}

/*void write_dgus_vp(u32 addr,u8* buf,u16 len)
{
    u32 xdata OS_addr;
	  u32 xdata OS_addr_offset;
    u16 xdata OS_len;
	  u16 xdata OS_len_offset;
	
		if(addr >= 0x10000)
			return;
    EA=0;
    OS_addr=addr/2;
    OS_addr_offset=addr%2; 
    ADR_H=(u8)(OS_addr>>16)&0xFF;
    ADR_M=(u8)(OS_addr>>8)&0xFF;
    ADR_L=(u8)OS_addr&0xFF;
    ADR_INC=0x01;                 //DGUS  OS�洢����ַ�ڶ�д���Զ���1
    RAMMODE=0x8F;               //����дģʽ 
    if(OS_addr_offset==1)
    {
        ADR_INC=0;
        RAMMODE=0xAF;
        while(APP_ACK==0);
        APP_EN=1;
        while(APP_EN==1);       //��дִ�н���
        ADR_INC=0x01; 
        RAMMODE=0x8F;
        while(APP_ACK==0);      
        DATA1=*buf++;
        DATA0=*buf++;
        APP_EN=1;
        while(APP_EN==1);       //��дִ�н���
        len--;
        OS_addr_offset=0;
    }
    OS_len=len/2;
    OS_len_offset=len%2; 
    if(OS_len_offset==1)
    {
         OS_len++;
    }
    while(OS_len--)
    {
        if((OS_len_offset==1)&&(OS_len==0))
        {
            ADR_INC=0;
            RAMMODE=0xAF;
            while(APP_ACK==0);
            APP_EN=1;                //����һ�������̣�����д����ʱ�Ὣ��һ������д0
            while(APP_EN==1);       //��дִ�н���
            ADR_INC=0x01;
            RAMMODE=0x8F;
            while(APP_ACK==0);           
            DATA3=*buf++;
            DATA2=*buf++;
            APP_EN=1;
            while(APP_EN==1);       //��дִ�н���
            break;
        }
        while(APP_ACK==0);        
        DATA3=*buf++;
        DATA2=*buf++;
        DATA1=*buf++;
        DATA0=*buf++;
        APP_EN=1;
        while(APP_EN==1);       //��дִ�н���
    } 
    RAMMODE=0x00;       //��д��ɺ�RAMMODE��������
    EA=1; 
		delay_us(100);		
}  */


/*****************************************************************************
 �� �� ��  : void INIT_CPU(void)
 ��������  : CPU��ʼ������
			����ʵ��ʹ�������޸Ļ򵥶�����
 �������  :	
 �������  : 
 �޸���ʷ  :
  1.��    ��   : 2019��4��1��
    ��    ��   : chengjing
    �޸�����   : ����
*****************************************************************************/ 
//void INIT_CPU(void)
//{
//    EA=0;
//    RS0=0;
//    RS1=0;

//    CKCON=0x00;
//    T2CON=0x70;
//    DPC=0x00;
//    PAGESEL=0x01;
//    D_PAGESEL=0x02;   //DATA RAM  0x8000-0xFFFF
//    MUX_SEL=0x60;   //UART2,UART2������WDT�ر�
//    RAMMODE=0x00;
//    PORTDRV=0x01;   //����ǿ��+/-8mA
//    IEN0=0x00;      //�ر������ж�
//    IEN1=0x00;
//    IEN2=0x00;
//    IP0=0x00;      //�ж����ȼ�Ĭ��
//    IP1=0x00;
//    WDT_OFF();
//        // UART2����8N1  115200       �б�Ƶ����UART3��һ��
//    ADCON = 0x80;
//    SCON0 = 0x50;
//    SREL0H = 0x03; // FCLK/64*(1024-SREL1)
//    SREL0L = 0xE4;
///*****************
//    WDT_OFF();      //�رտ��Ź�
//************************************/
//	 //ClearRAM();									//��ʼ��RAMΪ0
//}

//void ClearRAM(void)
//{
//	u8 xdata*ptr;
//	u16 data i;
//	
//	ptr = 0x8000;
//	
//	for(i=0;i<0x8000;i++)
//		*ptr++ = 0;
//}


void delay_us(unsigned int t)
{ 
 u8 i;

 while(t)
 {
  for(i=0;i<50;i++)
   {i=i;}
  t--;
 }
}

void Page_Change(u16 PageID)
{
	u8 buf[4];
  
	buf[0] = 0x5a;
	buf[1] = 0x01;
	buf[2] = (u8)(PageID >> 8);
	buf[3] = (u8)PageID;
	write_dgus_vp(0x84,buf,2);
	do
	{
		delay_us(500);
		read_dgus_vp(0x14,buf,2);
	}while(*(u16*)buf!=PageID);
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	write_dgus_vp(0x16,buf,2);
}

void delay_ms(u16 t)
{
	u16 i,j;
	  for(i=0;i<t;i++)
	  {
			for(j=0;j<300;j++)
	     delay_us(1);
		}
}

void SetPinOut(u8 Port,u8 Pin)
{
switch(Port)
{
	case 0: P0MDOUT|=(1<<Pin);
			break;
	case 1: P1MDOUT|=(1<<Pin);
			break;
	case 2: P2MDOUT|=(1<<Pin);
			break;
	case 3: P3MDOUT|=(1<<Pin);
			break;	
	default:break;				
}

}

void SetPinIn(u8 Port,u8 Pin)
{
	switch(Port)
	{
		case 0: P0MDOUT&=~(1<<Pin);
				break;
		case 1: P1MDOUT&=~(1<<Pin);
				break;
		case 2: P2MDOUT&=~(1<<Pin);
				break;
		case 3: P3MDOUT&=~(1<<Pin);
				break;	
		default:break;				
	}
		
}

void PinOutput(u8 Port,u8 Pin,u8 value)
{
	switch(Port)
	{
		case 0: if(value) P0|=(1<<Pin);
				else      P0&=~(1<<Pin);
				break;
		case 1: if(value) P1|=(1<<Pin);
				else      P1&=~(1<<Pin);
				break;
		case 2: if(value) P2|=(1<<Pin);
				else      P2&=~(1<<Pin);
				break;
		case 3: if(value) P3|=(1<<Pin);
				else      P3&=~(1<<Pin);
				break;	
		default:break;				
	}
}

u8 GetPinIn(u8 Port,u8 Pin)
{	 
	u8 value;
	switch(Port)
	{
		case 0: value=P0&(1<<Pin);
				  break;
		case 1: value=P1&(1<<Pin);
				  break;
		case 2: value=P2&(1<<Pin);
				  break;
		case 3: value=P3&(1<<Pin);
				  break;	
		default:
				value=0;
				break;				
	}
	return value;
}


//��ʹ�õĺ���
#if UnusedFunction

void TouchSwitch(u16 PageID, u8 TouchType, u8 TouchID, u8 Status)
{
	u8 k_data[8];
	
	*(u16*)k_data = 0x5aa5;
	*(u16*)&k_data[2] = PageID;
	k_data[4] = TouchID;
	k_data[5] = TouchType;
	if(Status)
		*(u16*)&k_data[6] = 1;
	else
		*(u16*)&k_data[6] = 0;
	write_dgus_vp(0xb0,k_data,4);
	do
	{
		delay_us(500);
		read_dgus_vp(0xb0,k_data,1);
	}while(k_data[0]!=0);
}

void NorFlash_Action(void)
{
	u8 buf[8];	
	
	buf[0] = Nor_Flash.Mode;
	buf[1] = (u8)(Nor_Flash.FLAddr>>16);
	buf[2] = (u8)(Nor_Flash.FLAddr>>8);
	buf[3] = (u8)Nor_Flash.FLAddr;
	buf[4] = (u8)(Nor_Flash.VPAddr>>8);
	buf[5] = (u8)Nor_Flash.VPAddr;
	buf[6] = (u8)(Nor_Flash.Len>>8);
	buf[7] = (u8)Nor_Flash.Len;
	if(Nor_Flash.Mode == 0xa5)//д����
	{
		if(Nor_Flash.Buf != NULL)
			write_dgus_vp(Nor_Flash.VPAddr,Nor_Flash.Buf,Nor_Flash.Len);
	}
	write_dgus_vp(0x08,buf,4);
	do
	{
		delay_us(500);
		read_dgus_vp(0x08,buf,1);
	}while(buf[0]!=0);
	if(Nor_Flash.Mode == 0x5a)//������
	{
		if(Nor_Flash.Buf != NULL)
			read_dgus_vp(Nor_Flash.VPAddr,Nor_Flash.Buf,Nor_Flash.Len);
	}
	delay_ms(FLASH_ACCESS_CYCLE);
}

void SPIFlash_Action(void)//����д������Ҫ��ǰ׼����32K��VP����
{
	u8 buf[12];
	
	buf[0] = 0x5a;
	buf[1] = Spi_Flash.Mode;
	if(Spi_Flash.Mode == 1)
	{
		buf[2] = Spi_Flash.ID;
		buf[3] = (u8)(Spi_Flash.FLAddr>>16);
		buf[4] = (u8)(Spi_Flash.FLAddr>>8);
		buf[5] = (u8)Spi_Flash.FLAddr;
		buf[6] = (u8)(Spi_Flash.VPAddr>>8);
		buf[7] = (u8)Spi_Flash.VPAddr;
		buf[8] = (u8)(Spi_Flash.Len>>8);
		buf[9] = (u8)Spi_Flash.Len;
		buf[10] = 0;
		buf[11] = 0;
	}
	else if(Spi_Flash.Mode == 2)
	{
		buf[2] = (u8)(Spi_Flash.ID>>8);
		buf[3] = (u8)Spi_Flash.ID;
		buf[4] = (u8)(Spi_Flash.VPAddr>>8);
		buf[5] = (u8)Spi_Flash.VPAddr;
		buf[6] = (u8)(Spi_Flash.Delay>>8);
		buf[7] = (u8)Spi_Flash.Delay;
		buf[8] = 0;
		buf[9] = 0;
		buf[10] = 0;
		buf[11] = 0;
	}	
	write_dgus_vp(0xaa,buf,6);
	do
	{
		delay_us(500);
		read_dgus_vp(0xaa,buf,1);
	}while(buf[0]!=0);
	if(Spi_Flash.Mode == 1)//������
	{
		if(Spi_Flash.Buf != NULL)
			read_dgus_vp(Spi_Flash.VPAddr,Spi_Flash.Buf,Spi_Flash.Len);
	}
	delay_ms(FLASH_ACCESS_CYCLE);
}

//��õ�ǰҳ��
u16 GetPageID()
{
	u16  nPage;

	read_dgus_vp(PIC_NOW,(u8*)(&nPage),1);	
	return nPage;
}

void ResetT5L(void)
{
	u32 tmp;
	
	tmp = 0x55aa5aa5;
	write_dgus_vp(0x04,(u8*)&tmp,2);
}

void BackLight_Control(u8 light)
{
	u16 DATA_RAM;
	u16 DATA_RAM1;
	read_dgus_vp(0x0082,(u8*)&DATA_RAM,1);
	DATA_RAM&=0x00FF;
	DATA_RAM1=light;
	DATA_RAM1<<=8;
	DATA_RAM|=DATA_RAM1;
	write_dgus_vp(0x0082,(u8*)&DATA_RAM,1);
}

//���ֲ���
void MusicPlay(u8 MusicId)
{
	u8 k_data[4];
	
	read_dgus_vp(0xa0,k_data,2);
	k_data[0] = MusicId;
	k_data[1] = 0x01;
	k_data[3] = 0;
	write_dgus_vp(0xa0,k_data,2);
}

//���ư�������������  0�ر�  1����  2����
void system_config(u8 is_beep,u8 is_sleep)
{
	u8 config_cmd[4];
	read_dgus_vp(0x0080,config_cmd,2);
	if(is_beep == 1)
	{
		config_cmd[3] |= (1<<3);
	}
	else if(is_beep == 0)
	{
		config_cmd[3] &= ~(1<<3);
	}
	else 
	{
		;
	}
	
	if(is_sleep == 1)
	{
		config_cmd[3] |= (1<<2);
	}
	else if(is_sleep == 0)
	{
		config_cmd[3] &= ~(1<<2);
	}
	else 
	{
		;
	}
	config_cmd[0] = 0x5A;
	write_dgus_vp(0x0080,config_cmd,2);
}

/*
	����ϵͳ����0x0082�����Ʊ�������Ⱥʹ���
	work_light  	����ʱ����
	sleep_light		����ʱ����
	await_time		��ú����
*/
void system_led_config(u8 sleep_light, u16 await_time)
{
	u8 led_config_cmd[4];
	read_dgus_vp(0x0082,(u8*)&led_config_cmd,2);
	//led_config_cmd[0] = work_light;
	led_config_cmd[1] = sleep_light;
	led_config_cmd[2] = (u8)(await_time>>8);
	led_config_cmd[3] = (u8)(await_time); 
	
	write_dgus_vp(0x0082,led_config_cmd,2);
	
}

//��ȡ��Ļ�Ƿ񱻴��� 0δ������Ļ   1�д�����Ļ
u8 ScreenTouchOrNot(void)
{
	u8 k_data[2];
	
	read_dgus_vp(0x16,k_data,1);
	if(k_data[0]==0x5a)
	{
		k_data[0]=0;
		write_dgus_vp(0x16,k_data,1);
		return 1;
	}
	return 0;
}

u8 GetTouchStatus(void)
{
	u8 k_data[8];
	
	read_dgus_vp(0x16,k_data,4);
	if(k_data[0]==0x5a)
	{
		if(k_data[1]==0x01)//��һ�ΰ���
		{
			LastPosX = PressPosX = *(s16*)&k_data[2];
			LastPosY = PressPosY = *(s16*)&k_data[4];
			Touchstate = FIRST_PRESS;
		}
		else if(k_data[1]==0x03)//����
		{
			LastPosY = RealPosY;
			LastPosX = RealPosX;
			Touchstate = UNDER_PRESS;
		}
		else//̧��
		{
			Touchstate = RELEASE_PRESS;
		}
		RealPosX = *(s16*)&k_data[2];
		RealPosY = *(s16*)&k_data[4];
		k_data[0] = 0;
		write_dgus_vp(0x16,k_data,2);
		return Touchstate;
	}
	else
	{
		if((k_data[1]==0x02)||(k_data[1]==0x00))
		{
			Touchstate = IDLE_PRESS;
		}
		return Touchstate;
	}
}

u8 GetTouchAnction(u16 PageID)
{
	u8 status;
	u32 x2distance,y2distance;
	
	status = GetTouchStatus();
	if(IDLE_PRESS==status)
	{
		PressPosChange = UNTOUCH;
		return PressPosChange;
	}
	else if(FIRST_PRESS==status)//�״ΰ��£���¼����ҳ�棬��ֹ�л�������ҳ�����Ӱ��
	{
		read_dgus_vp(0x14,(u8*)&PressPageID,1);
		PressPosChange = UNSLIDE;
		return UNSLIDE;
	}
	else
	{
		if(PageID != PressPageID)
		{
			return UNSLIDE;
		}
		xdistance = (PressPosX - RealPosX);
		x2distance = xdistance * xdistance;
		ydistance = (PressPosY - RealPosY);
		y2distance = ydistance * ydistance;
		if(UNDER_PRESS==status)//������
		{
			if(UNSLIDE == PressPosChange)//δ����
			{
				if((y2distance >= SLIDE_THRESHOLD)||(x2distance >= SLIDE_THRESHOLD))
				{
					if(y2distance*ANGLE_THRESHOLD >= x2distance)//   Y/X > 0.5����30�Ƚ���Ϊ���򻬶��������򻬶��ķֽ���
					{
						PressPosChange = VERTICAL_SLIDE;
						return VERTICAL_SLIDE;//�������򻬶�
					}
					else//���һ���
					{
						PressPosChange = HORIZONTAL_SLIDE;
						return HORIZONTAL_SLIDE;
					}
				}
			}
			return PressPosChange;
		}
		else//̧��״̬
		{
			if(UNSLIDE == PressPosChange)//δ����
			{
				if((y2distance >= SLIDE_THRESHOLD)||(x2distance >= SLIDE_THRESHOLD))//�����ٻ���ʱ���������������볤��״̬��̧��ʱͬ���ж�һ����������ı����
				{
					if(y2distance*ANGLE_THRESHOLD >= x2distance)//   Y/X > 0.5����30�Ƚ���Ϊ���򻬶��������򻬶��ķֽ���
					{
						PressPosChange = VERTICAL_SLIDE;//�������򻬶�
					}
					else//���һ���
					{
						PressPosChange = HORIZONTAL_SLIDE;
					}
				}
				else
				{
					return CLICKTOUCH;//��������¼�
				}
			}
			if(VERTICAL_SLIDE==PressPosChange)
			{
				PosYChangeSpeed = LastPosY-RealPosY;//�ƶ��ٶȣ�����PosX_ChangeSpeed,PosY_ChangeSpeed��������һ��������̧���������߶γ���
				return AUTOVERTICAL_SLIDE;//�����ƶ��ٶȴ�������
			}
			else
			{
				PosXChangeSpeed = LastPosX-RealPosX;//�ƶ��ٶ�,����
				return AUTOHORIZONTAL_SLIDE;//�����ƶ��ٶȴ�������
			}
		}
	}
}

//���õ�����ʾ���
#define TIMERNUM	5	//��ʱ��
#define AddrpopUpCode   0x6FE0
#define PopUpLen	64//�ֽ�
void setPopupMessage(u8 *sourceStr,u16 len)
{
	
	u8 tmp[PopUpLen];
	StrClear(tmp,sizeof(tmp));
	StrCopy(tmp,sourceStr,len);
	write_dgus_vp(AddrpopUpCode,(u8*)&tmp,sizeof(tmp)/2);
	StartTimer(TIMERNUM,2500);
}

//������ʾ��ʱ�ر�
void PopupMessageTimeOutClose(void)
{
	//�˺���Ҫֱ�ӷ���while(1){};��
	u8 tmp[PopUpLen];
	if(GetTimeOutFlag(TIMERNUM))//�жϵ����ĳ�ʱ,����
	{
		StrClear(tmp,sizeof(tmp));
		write_dgus_vp(AddrpopUpCode,(u8*)&tmp,sizeof(tmp)/2);
		KillTimer(TIMERNUM);
	}
}


//����PWM0������
//pwm_psc			��Ƶϵ��
//pwm0_precision	PWM0���� �����ޣ�
//���㹫ʽ��  PWM0�ز�Ƶ�� = 825.7536MHz / (��Ƶϵ�� * PWM0����)
void PWM0_Set(u8 pwm_psc,u16 pwm0_precision)
{
	#define PWM0_SET_ADDR 0x86
	u8 buf[4];
	buf[0] = 0x5A;
	buf[1] = pwm_psc;
	buf[2] = (u8)(pwm0_precision>>8);
	buf[3] = (u8)pwm0_precision;
	
	write_dgus_vp(PWM0_SET_ADDR,buf,2);
}

//����ռ�ձ�
//duty_cycle		ռ�ձ�		ȡֵ1-100
//pwm0_precision	���ȣ����ޣ�
void PWM0_Out(u16 duty_cycle,u16 pwm0_precision)
{
	#define PWM0_OUT_ADDR 0x92
	u8 buf[2];
	u16 tmp;
	tmp = (pwm0_precision/100)*duty_cycle;		//������1%  2%  3% ... 100%
	buf[0] = (u8)(tmp>>8);
	buf[1] = (u8)tmp;
	
	write_dgus_vp(PWM0_OUT_ADDR,buf,1);
}

//���ƶ�̬����
//chart_id:����ͨ��id,��Χ[0,7],ϵͳ�ܹ�֧��8������ͨ��,ÿ������ͨ��ռ��2K���ֿռ�,
//				 �ܻ�������Χ:0x1000-0x4FFF,û��ʹ�õ����߻�����������������;
//axit_y:д��ĳһ�����ݵ������ֵ
void system_draw_dynamic_graph(u8 chart_id, u16 axit_y)
{
	#define CHART_ADDR	0x0310	//��̬���߹�������Ӧ��ϵͳ�����ӿڵ�ַ
	#define	CHART_NUM	1		//һ����д�뼸������ͨ��������,����ֻд��chart_idָ��������ͨ��,��1��
	#define	POINT_NUM	1		//һ����д����ٸ����ݵ�
	
	u8 chart_cmd[6+POINT_NUM*2] = {0x5A,0xA5,CHART_NUM,0x00};
	
	chart_cmd[4] = chart_id;
	chart_cmd[5] = POINT_NUM;
	chart_cmd[6] = (u8)(axit_y>>8);
	chart_cmd[7] = (u8)axit_y;
	
	write_dgus_vp(CHART_ADDR,chart_cmd,3+POINT_NUM);
}

//�޸��������ڵĲ���
void modifyInceaseControlPara(u8 page_id,u8 id,u16 minLimit,u16 step,u16 maxLimit)
{
	u8 tmp[20];
	
	tmp[0] = 0x5a;
	tmp[1] = 0xa5;
	tmp[2] = 0;
	tmp[3] = page_id;
	tmp[4] = id;
	tmp[5] = 2;
	tmp[6] = 0;
	tmp[7] = 2;//��ȡ
	write_dgus_vp(0x00B0,tmp,4);	
	do
	{
		delay_ms(2);
		read_dgus_vp(0x00B0,tmp,1);
	}while(tmp[0]!=0);
	
	write_dgus_vp(0xb4+11,(u8*)&step,1);//����
	write_dgus_vp(0xb4+12,(u8*)&minLimit,1);//����
	write_dgus_vp(0xb4+13,(u8*)&maxLimit,1);//����
	
	tmp[0] = 0x5a;
	tmp[1] = 0xa5;
	tmp[2] = 0;
	tmp[3] = page_id;
	tmp[4] = id;
	tmp[5] = 2;
	tmp[6] = 0;
	tmp[7] = 3;//д��
	write_dgus_vp(0x00B0,tmp,4);	
	do
	{
		delay_ms(2);
		read_dgus_vp(0x00B0,tmp,1);
	}while(tmp[0]!=0);
}


u16 readADC(u16 channel)
{
	read_dgus_vp(0x0032+channel,(u8*)&channel,1);
	return channel;
}

/*
����:��ȡSPIFlash�е�����
����:
	spiAddr		��ǰspiID�еĶ�ȡ��ʼ��ַ,��Χ0x000000-0x01FFFF
	spiID		�ֿ� ID�� 0x10-0x1F��ÿ���ֿ� 256Kbytes����� 4Mbytes��
	vpAddr		��ȡ����dgus��ַ,������ż��
	len			��ȡ���ֳ���,�����ֶ��壬������ż��
	buff		�����buff[]�ĵ�ַ,�����û���,����NULL
*/
void readSPIFlashToDgus(u32 spiAddr,u16 spiID,u16 vpAddr,u16 len,u8 *buff)
{
	Spi_Flash.Mode=1;
	Spi_Flash.ID=spiID;
	Spi_Flash.FLAddr=spiAddr;
	Spi_Flash.VPAddr=vpAddr;
	Spi_Flash.Len=len;
	Spi_Flash.Buf=buff;
	SPIFlash_Action();
}

/*
����:д�����ݵ�SPIFlash��,һ�ΰ���32KBȥд
����:
	spiID		32Kbytes �洢�����ţ�0x0000-0x01FF����Ӧ���� 16Mbytes �洢����
	vpAddr		��Ҫ���µ����ݱ��������ݱ����ռ���׵�ַ��������ż����
*/
void writeCodeToSPIFlash(u16 spiID,u16 vpAddr)
{//ÿ��256K��bin�ļ�,�ֳ�8��,spiID��Ŵ�0x0000-0x1fff
	Spi_Flash.Mode=2;
	Spi_Flash.ID=spiID;//����17.bin��ʵ�����:17*8=136
	Spi_Flash.VPAddr=vpAddr;
	Spi_Flash.Delay=20;
	SPIFlash_Action();
}


//ģ�ⴥ��:touchid:ģ�ⴥ�ص�id
void analogTouch(u8 touchid)
{
    u16 tmp[4];
    tmp[0]=0x5aa5;
    tmp[1]=4;
    tmp[2]=(0xFF00|touchid);
    tmp[3]=1;
    write_dgus_vp(0x00d4,(u8*)&tmp,4);
}


#endif
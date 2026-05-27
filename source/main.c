#include "sys.h"
#include "timer.h"
#include "uart.h"
#include "ui.h"
#include "rtc.h"
#include "encoder.h"
#include "dwin8283.h"
#include "debug.h"
#include "hmi.h"
#include "data_demo.h"
#define USE_LOG 0
#define TAG "MAIN"
#if (USE_LOG && DEBUG)
#define LOG mylog
#else
#define LOG /\
/LOG
#endif

#define TMR_7 7
#define CHECK_TIME 5000
static u16 system_ready = 0;

#define VP_DATA_PUSH          0x8200  // 유지시간 설정



void main()
{
    u16 state;
    u8 did_first_page = 0;  
    u16 test1 = 10;
    u16 test2 = 3567;
    T0_Init();
    T1_Init();
    init_rtc();
    SetPinIn(2, 0);
    StartTimer(0, 60000);
    StartTimer(1, 500);                                                     
    Pro8283Init();
    DEBUGINIT();
    EA = 1;
    Page_Change_Handler(page_number); // �ϵ���ʾ����
    Picture1213_Init();               // �ϵ�12ҳ��13ҳͼ����ʾ
    LOG("start\r\n");
    StartTimer(TMR_7, CHECK_TIME);

    write_dgus_vp(VP_DATA_PUSH, (u8*)&test1, 1);
 
    
    while (1)
    {
        Pro8283Deal();
      if (!system_ready && GetTimeOutFlag(TMR_7)) {
        system_ready = 1;
         page_number = 1;
        Page_Change_Handler(page_number); // �ϵ���ʾ����
    }
     if (!system_ready) continue;  // 준비될 때까지 아래 실행 안함
        SecCnt_TickTask();
        write_dgus_vp_check();
        state = Encoder_recevie();
        encoder_page_change(state);
           if (g_need_upgrade)
        {
            g_need_upgrade = 0;
   //         EnterCCodeUpgrade();
        }
    }

}

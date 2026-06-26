#include "ui.h"
#include "timer.h"
#include <string.h>
#include "hmi.h"
#include "sys.h"

// ===== add prototypes (put these after includes) =====
static void DGUS_WriteWord(u16 vp, u16 val);
static void DGUS_WriteWord_BE(u16 vp, u16 val);
static void DGUS_Color_Sweep_All(u16 sp_addr);
static void DGUS_ApplyColor_SP_8803_BE(u16 sp_addr, u16 rgb565);
static void DGUS_ApplyColor_ATTR_VP(u16 attr_vp, u16 rgb565);
void DGUS_Color_Diagnose_And_Apply(u16 sp_addr, u16 attr_vp, u8 do_sweep);

static void SetTextColorRed(u16 sp_addr);
static void SetTextColorBlack(u16 sp_addr);
static void SetTextColorWhite(u16 sp_addr);
static void SetTextColorYellow(u16 sp_addr);
static void SetTextColorGreen(u16 sp_addr);
static void ChangeImage(u16 vp_addr, u16 index);

#define COL_BLACK 0x0000
#define COL_WHITE 0xFFFF
#define COL_RED   0xC044
#define COL_GREEN 0x07E0
#define COL_BLUE  0x051C
#define COL_GREY  0xCE59

#define TMR_ENCODER  0     // 0~7 중 사용 안 하는 채널 하나 선택
#define ENC_DELAY_MS 30

#define BIT(n)            (1u << (n))
#define FLAG_READY       BIT(0)    
#define FLAG_START        BIT(1)   
#define FLAG_STOP         BIT(2)    
#define FLAG_RUN       BIT(3)   
#define FLAG_PUMP         BIT(4)    
#define FLAG_TOPTEMP        BIT(5)    
#define FLAG_BOTTEMP         BIT(6)   
#define FLAG_TEMPON        BIT(7)   
#define FLAG_DELEAY         BIT(8)    
#define FLAG_TOPFAN        BIT(9)    
#define FLAG_BOTFAN         BIT(10)    
#define FLAG_OUTPUMP        BIT(11)    
#define FLAG_COOLING         BIT(12)    
#define FLAG_SENSOR_ERR       BIT(13)   
#define FLAG_TIME         BIT(14)    


#define VP_TT        0x8000  // 상 히터 온도 (℃)
//#define VP_TB        0x8002  // 하 히터 온도 (℃)
#define VP_P         0x8004  // 현재 압력 (0.1bar)
#define VP_NTT       0x8006  // 상 프레임 NTC
#define VP_NTB       0x8008  // 하 프레임 NTC
#define VP_ST        0x800A  // 상태코드
#define VP_FLAGS     0x800C  // 플래그 비트
#define VP_ERR_CODE  0x8010  // 에러 코드
#define VP_ERR_ARG   0x8012  // 에러 인자

#define VP_SET_TT         0x8100  // 상 히터 온도 설정
#define VP_SET_TB         0x8102  // 하 히터 온도 설정
#define VP_SET_P          0x8104  // 목표압력 설정
#define VP_SET_H          0x8106  // 유지시간 설정
#define VP_SET_CHTT       0x8108  // 상 냉각온도 설정
#define VP_SET_CHTB       0x810A  // 하 냉각온도 설정
#define VP_SET_NTC_BASE_T 0x810C  // 상 프레임 기준 온도 설정
#define VP_SET_NTC_BASE_B 0x8110  // 하 프레임 기준 온도 설정
#define VP_TEST1          0x8200  // 유지시간 설정
#define VP_TEST2          0x8202  // 유지시간 설정

#define VP_ELAPSED_TIME 0x8114 // 유지 경과 시간


#define SP_TXT_TT 0x5000
#define SP_TXT_TB 0x5002

#define TMR_UI_POLL 1       // 새 타이머 채널
#define UI_POLL_MS  200     // 200ms마다 화면 값 갱신

#define VP_NOR_FLASH_RW_CMD 0x0008

static u8  g_cnt_active = 0;
static u16 g_cnt_value  = 0;

static u8 page_number    = 0;

static u8 read500_enable = 0;
u8 xdata page_set[4] = {0};

u16 Page[29] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
};

static u16 start_flag = 0;
static u8  deleay_flag = 0;
static u16 page24_flag = 0;
static u16 error_flag = 0;

static u16 hundreds[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static u16 tens[8]     = { 0, 0, 0, 0, 0, 0, 0, 0 };
static u16 ones[8]     = { 0, 0, 0, 0, 0, 0, 0, 0 };

static u16 TT100, TT10 , TT1 ;
static u16 TC10 = 0, TC1 = 0;
static u16 BT100 = 0, BT10 = 0,BT1 = 0;
static u16 BC10 = 0, BC1 = 0;
static u16 min = 0, second = 0;
static u16 press            = 0;  // 0.1 단위 표현용 (ex. 25 -> 2.5 bar)

static u8 check_active = 0;
#define TMR_6 6
#define CHECK_TIME 700
static u16 legacy_value = 0;
static u16 recoll_value = 0;
static u16 check_addr;
static u16 check_count = 0;

void check_Start(u16 addr, u16 velue){
    check_addr = addr;
    legacy_value = velue;
    check_active = 1;
    write_dgus_vp(check_addr, (u8*)&legacy_value, 1);
    StartTimer(TMR_6, CHECK_TIME); 
}

void check_End(){
    check_active = 0;
    check_count = 0;
}

void write_dgus_vp_check(void) {
     if (!check_active) return; 
     if (GetTimeOutFlag(TMR_6)) {
        read_dgus_vp(check_addr, (u8*)&recoll_value, 1);
    if(legacy_value == recoll_value){
        check_End();
    }else if(check_count < 2){
        check_Start(check_addr, legacy_value);
        check_count++;
    }
    } 
}


void SetTextColorRed(u16 sp_addr) {
    u16 color = 0xC064;  // RGB565 빨강
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorBlack(u16 sp_addr) {
    u16 color = 0x18E3;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorYellow(u16 sp_addr) {
    u16 color = 0xFF60;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorBlue(u16 sp_addr) {
    u16 color = 0x051D;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorGray(u16 sp_addr) {
    u16 color = 0xCE59;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void SetTextColorWhite(u16 sp_addr) {
    u16 color = 0xFFFF;
    write_dgus_vp(sp_addr, (u8*)&color, 1);  // 1워드 쓰기
}

void UI_Read500ms_Stop(void){
  read500_enable = 0;
}

static u8 ui_poll_enable = 0;
static u16 timer_keep = 0;

void SettingHundred(u16 addr_h,u16 addr_t, u16 addr_o, u16 result){
    u16 h = result / 100;
    u16 t = (result / 10) % 10;
    u16 o = result % 10;

    write_dgus_vp(addr_h, (u8*)&h, 1);
    write_dgus_vp(addr_t, (u8*)&t, 1);
    write_dgus_vp(addr_o, (u8*)&o, 1);
}

void SettingTime(u16 addr_m,u16 addr_s, u16 time){
    u16 m = time/60;
    u16 s = time%60;

    write_dgus_vp(addr_m, (u8*)&m, 1);
    write_dgus_vp(addr_s, (u8*)&s, 1);
}



void Picture1_Temp(u16 n) {
    u16 x = 0, y = 1;

    write_dgus_vp(0x2900, (u8*)&x, 1);
    write_dgus_vp(0x3900, (u8*)&y, 1);

    switch (n) {
        case 0: {
            write_dgus_vp(0x2900, (u8*)&x, 1);
            write_dgus_vp(0x3900, (u8*)&y, 1);
        } break;

        case 1: {
            write_dgus_vp(0x2901, (u8*)&x, 1);
            write_dgus_vp(0x3901, (u8*)&y, 1);
        } break;
    }
}

void QuickSettingTextSet(u16 count, u16 select_position){
    switch(count){
        case 1:{
            switch(select_position){
                case 0:{
                     SetTextColorYellow(0x9403);
                    SetTextColorYellow(0x9413);
                    SetTextColorBlue(0x9423);
                }break;
                 case 1:{
                    SetTextColorYellow(0x9403);
                    SetTextColorBlue(0x9413);
                    SetTextColorYellow(0x9423);
                }break;
                 case 2:{
                    SetTextColorBlue(0x9403);
                    SetTextColorYellow(0x9413);
                    SetTextColorYellow(0x9423);
           
                }break;
                
                default : {
                    SetTextColorYellow(0x9403);
                    SetTextColorYellow(0x9413);
                    SetTextColorYellow(0x9423);
                }break;
            }
        }break;

        case 2:{
            SetTextColorBlue(0x9253);
        }break;

        case 3:{
             switch(select_position){
                case 0:{
                       SetTextColorYellow(0x9453);
                    SetTextColorYellow(0x9463);
                    SetTextColorBlue(0x9473);
                }break;
                 case 1:{
                    SetTextColorYellow(0x9453);
                    SetTextColorBlue(0x9463);
                    SetTextColorYellow(0x9473);
                }break;
                 case 2:{
                      SetTextColorBlue(0x9453);
                    SetTextColorYellow(0x9463);
                    SetTextColorYellow(0x9473);
                }break;
                default : {
                    SetTextColorYellow(0x9453);
                    SetTextColorYellow(0x9463);
                    SetTextColorYellow(0x9473);
                }break;
            }
        }break;
         case 4:{
              switch(select_position){
                case 0:{
                    SetTextColorBlue(0x9283);
                    SetTextColorYellow(0x9293);
                }break;
                 case 1:{
                    SetTextColorYellow(0x9283);
                    SetTextColorBlue(0x9293);
                }break;
                default : {
                    SetTextColorYellow(0x9283);
                    SetTextColorYellow(0x9293);
                }break;
            }
        }break;
    }
    

}

void select_num(u16 page, u16 count) { // 텍스트 색 설정
    switch (page) {
        case 13: {
            if (count == 0) {
                SetTextColorBlack(0x9003);
                SetTextColorBlack(0x9013);
                SetTextColorBlue(0x9023);
            } else if (count == 1) {
                SetTextColorBlack(0x9003);
                SetTextColorBlue(0x9013);
                SetTextColorBlack(0x9023);
            } else if (count == 2) {
                   SetTextColorBlue(0x9003);
                SetTextColorBlack(0x9013);
                SetTextColorBlack(0x9023);
            } else {
                SetTextColorBlack(0x9003);
                SetTextColorBlack(0x9013);
                SetTextColorBlack(0x9023);
            }

            if (count == 3) {
                ChangeImage(0x3910, 1);
            } else {
                ChangeImage(0x3910, 0);
            }
        } break;

        case 14: {
            if (count == 0) {
                SetTextColorBlack(0x9053);
                SetTextColorBlue(0x9063);
                
            } else if (count == 1) {
                SetTextColorBlue(0x9053);
                SetTextColorBlack(0x9063);
            }else if (count == 2) {
                SetTextColorBlack(0x9053);
                SetTextColorBlack(0x9063);
            }

            if (count == 2) {
                ChangeImage(0x3920, 1);
            } else {
                ChangeImage(0x3920, 0);
            }
        } break;

        case 15: {
            if (count == 0) {
                SetTextColorBlack(0x9073);
                SetTextColorBlack(0x9083);
                SetTextColorBlue(0x9093);
            } else if (count == 1) {
                SetTextColorBlack(0x9073);
                SetTextColorBlue(0x9083);
                SetTextColorBlack(0x9093);
            } else if (count == 2) {
              
                SetTextColorBlue(0x9073);
                SetTextColorBlack(0x9083);
                SetTextColorBlack(0x9093);
            } else {
                SetTextColorBlack(0x9073);
                SetTextColorBlack(0x9083);
                SetTextColorBlack(0x9093);
            }

            if (count == 3) {
                ChangeImage(0x3930, 1);
            } else {
                ChangeImage(0x3930, 0);
            }
        } break;

        case 16: {
            if (count == 0) {
                SetTextColorBlack(0x9103);
                SetTextColorBlack(0x9113);
                SetTextColorBlue(0x9123);
               
            } else if (count == 1) {
                SetTextColorBlack(0x9103);
                SetTextColorBlue(0x9113);
                SetTextColorBlack(0x9123);
            } else if (count == 2) {
                 SetTextColorBlue(0x9103);
                SetTextColorBlack(0x9113);
                SetTextColorBlack(0x9123);
            } else {
                SetTextColorBlack(0x9103);
                SetTextColorBlack(0x9113);
                SetTextColorBlack(0x9123);
            }

            if (count == 3) {
                ChangeImage(0x3940, 1);
            } else {
                ChangeImage(0x3940, 0);
            }
        } break;

        case 17: {
            if (count == 0) {
                     SetTextColorBlack(0x9143);
                SetTextColorBlue(0x9153);
            } else if (count == 1) {
           SetTextColorBlue(0x9143);
                SetTextColorBlack(0x9153);
            } else if (count == 2) {    
                SetTextColorBlack(0x9143);
                SetTextColorBlack(0x9153);
            }
            if (count == 2) {
                ChangeImage(0x3950, 1);
            } else {
                ChangeImage(0x3950, 0);
            }
        } break;

        case 18: {
            if (count == 0) {
                SetTextColorBlack(0x9163);
                SetTextColorBlack(0x9173);
                SetTextColorBlue(0x9183);
            } else if (count == 1) {
                SetTextColorBlack(0x9163);
                SetTextColorBlue(0x9173);
                SetTextColorBlack(0x9183);
            } else if (count == 2) {
                SetTextColorBlue(0x9163);
                SetTextColorBlack(0x9173);
                SetTextColorBlack(0x9183);
            } else {
                SetTextColorBlack(0x9163);
                SetTextColorBlack(0x9173);
                SetTextColorBlack(0x9183);
            }

            if (count == 3) {
                ChangeImage(0x3960, 1);
            } else {
                ChangeImage(0x3960, 0);
            }
        } break;

        case 19: {
            if (count == 0) {
                SetTextColorBlue(0x9193);
                SetTextColorBlack(0x9203);
            } else if (count == 1) {
                SetTextColorBlack(0x9193);
                SetTextColorBlue(0x9203);
            } else {
                SetTextColorBlack(0x9193);
                SetTextColorBlack(0x9203);
            }

            if (count == 2) {
                ChangeImage(0x3970, 1);
            } else {
                ChangeImage(0x3970, 0);
            }
        } break;

        case 20: {
            if (count == 1) {
                SetTextColorBlack(0x9213);
                ChangeImage(0x3980, 1);
            } else {
                SetTextColorBlue(0x9213);
                ChangeImage(0x3980, 0);
            }
        } break;

        // case 22: {
        //     if (count == 0) {
        //         SetTextColorBlue(0x9403);
        //         SetTextColorWhite(0x9413);
        //         SetTextColorWhite(0x9423);
        //     } else if (count == 1) {
        //         SetTextColorWhite(0x9403);
        //         SetTextColorBlue(0x9413);
        //         SetTextColorWhite(0x9423);
        //     } else if (count == 2) {
        //         SetTextColorWhite(0x9403);
        //         SetTextColorWhite(0x9413);
        //         SetTextColorBlue(0x9423);
        //     }
        // } break;

        case 23: {
            if (count == 0) {
                SetTextColorBlue(0x9433);
            } else if (count == 1) {
                SetTextColorWhite(0x9433);
            }
        } break;
    }
}

// 이미지 변경 (vp 주소 / 아이콘 id)
void ChangeImage(u16 vp, u16 icon_id) {
    u16 value = icon_id;
    write_dgus_vp(vp, (u8*)&value, 1);
}

void Page_Change_Handler(u8 n) {
    // if(!APP_ACK) return;   // 준비 전엔 그냥 무시
    page_set[0] = 0x5A;
    page_set[1] = 0x01;
    page_set[2] = 0x00;
    page_set[3] = Page[n];
    write_dgus_vp(0x0084, page_set, 2);
}

void Page21InIt() {
    //탑 세팅온도
    SetTextColorWhite(0x9403);
    SetTextColorWhite(0x9413);
    SetTextColorWhite(0x9423);
    // 압력
    SetTextColorWhite(0x9223);
    SetTextColorWhite(0x9253);
    //바텀 세팅온도
    SetTextColorWhite(0x9453);
    SetTextColorWhite(0x9463);
    SetTextColorWhite(0x9473);
    // 시간
    SetTextColorWhite(0x9283);
    SetTextColorWhite(0x9293);
    SetTextColorWhite(0x9273);

    ChangeImage(0x3000, 0);
}

void Page21Functioning(u16 count) {
    if (count == 1) {
        SetTextColorYellow(0x9403);
        SetTextColorYellow(0x9413);
        SetTextColorYellow(0x9423);
        ChangeImage(0x3000, 1);
    } else if (count == 2) {
        SetTextColorYellow(0x9223);
        SetTextColorYellow(0x9253);
    } else if (count == 3) {
        SetTextColorYellow(0x9453);
        SetTextColorYellow(0x9463);
        SetTextColorYellow(0x9473);
        ChangeImage(0x3000, 2);
    } else if (count == 4) {
        SetTextColorYellow(0x9283);
        SetTextColorYellow(0x9293);
    }
}

#define TMR_1S 5 // 여유 채널 번호로 바꿔도 됨
#define ONE_SEC 1000 // 1초(ms)
#define TMR_S2 8 // 여유 채널 번호로 바꿔도 됨
#define SEC2 500 // 1초(ms)
static u8 sec_active = 0; 
static u16 sec_value = 0;
static u16 time_result = 0;
static u16 elapsed_time = 0;


static void ShowMMSS(u16 t) { 
    u16 m = t / 60; 
    u16 s = t % 60; 
    write_dgus_vp(0x2850, (u8*)&m, 1); 
    write_dgus_vp(0x2860, (u8*)&s, 1); 
}
static void ShowMMSS2(u16 t) { 
    u16 m = t / 60; 
    u16 s = t % 60; 
    write_dgus_vp(0x3100, (u8*)&m, 1); 
    write_dgus_vp(0x3200, (u8*)&s, 1); 
}

static u16 timer = 0;

void SecCnt_Start() { 
    sec_active = 1; 
    SetTextColorYellow(0x9273);
    SetTextColorYellow(0x9333);
    SetTextColorYellow(0x9343);
    read_dgus_vp(VP_SET_H, (u8*)&timer, 1); 
    StartTimer(TMR_1S, ONE_SEC); 
 }

 

void SecCnt_Stop(void) { 
    sec_active = 0;
    SetTextColorWhite(0x9273);
    SetTextColorWhite(0x9333);
    SetTextColorWhite(0x9343);
}


void SecCnt_TickTask(void) {
     if (!sec_active){
        SetTextColorWhite(0x9273);
        return; 
     } 
     if (GetTimeOutFlag(TMR_1S)) {
       timer--;
       ShowMMSS2(timer);
          if (timer <= 0) { 
            SecCnt_Stop();
         } else { 
            StartTimer(TMR_1S, ONE_SEC); 
        } 
    } 
}


void Picture1213_Init(void) {
    u16 result = 0;

    read_dgus_vp(VP_SET_P, (u8*)&press, 1);

    //상판 설정 온도
    read_dgus_vp(VP_SET_TT, (u8*)&result, 1);
    TT100 = result / 100;
    TT10   = (result / 10) % 10;
    TT1     = result % 10;

    read_dgus_vp(VP_SET_TB, (u8*)&result, 1);
    BT100 = result / 100;
    BT10   = (result / 10) % 10;
    BT1     = result % 10;

    read_dgus_vp(VP_SET_H, (u8*)&time_result, 1); 
    ShowMMSS(time_result);  

    read_dgus_vp(VP_SET_CHTT, (u8*)&result, 1);
    TC10   = (result / 10) % 10;
    TC1     = result % 10;

    read_dgus_vp(VP_SET_CHTB, (u8*)&result, 1);
    BC10   = (result / 10) % 10;
    BC1     = result % 10;

    SetTextColorWhite(0x9273);
}

void encoder_page_change(u16 state)
{
    u16 keep = 0;
    u16 result = 0;

   
    u16 f_ready, f_start, f_tempon, f_cooling, f_deleay, f_toptemp, f_bottemp,f_topcool,f_botcool, f_stop,f_run,f_pump,f_outpump,f_sensor_err;   
    static u8 enc_busy = 0;  
    static u16 hundred_velue = 0;
    static u16 ten_velue     = 0;
    static u16 one_velue     = 0;
    static u8 select_flag    = 0;  // 타입 보강
    static u8 select_position = 0; // 타입 보강


    static settingflag      = 0;
    static u8 page6flag     = 0, page10flag = 0;

    static u8 page21flag    = 0;
    static u16 counting21   = 0;

    static u16 flags = 0;

    if (enc_busy && GetTimeOutFlag(TMR_ENCODER)) {
        enc_busy = 0;
    }

    if ((state == 1 || state == 2) && enc_busy) {
        return;
    }

    read_dgus_vp(VP_FLAGS, (u8*)&flags, 1);

    f_ready = (flags & FLAG_READY)   ? 1 : 0;
    f_start = (flags & FLAG_START)   ? 1 : 0;
    f_stop = (flags & FLAG_STOP)   ? 1 : 0;
    f_run = (flags & FLAG_RUN)   ? 1 : 0;
    f_pump = (flags & FLAG_PUMP)   ? 1 : 0;
    f_tempon = (flags & FLAG_TEMPON)? 1 : 0;
    f_cooling = (flags & FLAG_COOLING)? 1 : 0;
    f_deleay = (flags & FLAG_DELEAY)? 1 : 0;
    f_toptemp = (flags & FLAG_TOPTEMP)? 1 : 0;
    f_bottemp = (flags & FLAG_BOTTEMP)? 1 : 0;
    f_topcool = (flags & FLAG_TOPFAN)? 1:0;
    f_botcool = (flags & FLAG_BOTFAN)? 1:0;
    f_outpump = (flags & FLAG_OUTPUMP)   ? 1 : 0;
    f_sensor_err = (flags & FLAG_SENSOR_ERR)   ? 1 : 0;


    if(f_toptemp == 1){
        u16 color = 0xC044;
        write_dgus_vp(0x9313, (u8*)&color, 1);  
    }else if(f_topcool == 1){
        u16 color = 0x051C;
        write_dgus_vp(0x9313, (u8*)&color, 1);  
    }else{
         u16 color = 0xCE59;
        write_dgus_vp(0x9313, (u8*)&color, 1);  
    }

    if(f_bottemp == 1){
         u16 color = 0xC044;
         write_dgus_vp(0x9323, (u8*)&color, 1);  
    }else if(f_botcool == 1){
         u16 color = 0x051C;
         write_dgus_vp(0x9323, (u8*)&color, 1);  
    }else{
         u16 color = 0xCE59;
        write_dgus_vp(0x9323, (u8*)&color, 1);  
    }

    if(error_flag == 0 &&f_sensor_err==1){
        page_number=27;
        Page_Change_Handler(page_number);
        error_flag = 1;
    }

    if(f_deleay == 1&&deleay_flag==0){
        SecCnt_Start();
        deleay_flag = 1;
    }

    if(page_number == 24 && page24_flag == 0){
        read_dgus_vp(VP_SET_H, (u8*)&timer, 1);
        ShowMMSS2(timer);
        page24_flag = 1;
    }

    if(f_ready==1&& start_flag == 1){
            page_number = 23;
            Page_Change_Handler(page_number);
             read_dgus_vp(VP_SET_H, (u8*)&result,    1);
             min = result / 60;
             second = result % 60;
             write_dgus_vp(0x2850, (u8*)&min,    1);
             write_dgus_vp(0x2860, (u8*)&second, 1);
            page21flag = 0;
            select_flag=0;
            select_position = 0;
            settingflag = 0;
            counting21 = 0;
            start_flag = 0;
            deleay_flag = 0;
            page24_flag = 0;
            SecCnt_Stop();
    }else if(f_tempon==1||f_bottemp==1||f_toptemp==1){
            Page21InIt();
            start_flag = 1;
            page_number = 24;
            Page_Change_Handler(page_number);
            page21flag = 0;
            select_flag=0;
            select_position = 0;
            settingflag = 0;
            counting21 = 0;
            error_flag = 0;
        }else if(f_deleay==1){
            Page21InIt();
            start_flag = 1;
            page_number = 24;
            Page_Change_Handler(page_number);
            page21flag = 0;
            select_flag=0;
            select_position = 0;
            settingflag = 0;
            counting21 = 0;
            error_flag = 0;
        }else if(f_botcool==1||f_topcool == 1){
            Page21InIt();
            page_number = 25;
            Page_Change_Handler(page_number);
            SecCnt_Stop();
            if(timer == 1){
                timer = 0;
                ShowMMSS2(timer);
            }
            page21flag = 0;
            select_flag=0;
            select_position = 0;
            settingflag = 0;
            counting21 = 0;
            start_flag = 1;
    }
    switch (state) {
        case 1: {
            // 데이터 설정 모드
            if (settingflag == 1) {
                switch (page_number) {
                    case 13: {
                         if (select_position == 0) {
                                keep = TT1;
                                if (TT1 == 0) {
                                    TT1 = 9;
                                } else {
                                    TT1--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2020, (u8*)&TT1, 1);
                                } else {
                                    TT1 = keep;
                                    write_dgus_vp(0x2020, (u8*)&TT1, 1);
                                }
                              
                            } else if (select_position == 1) {
                                keep = TT10;
                                if (TT10 == 0) {
                                    TT10 = 9;
                                } else {
                                    TT10--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2010, (u8*)&TT10, 1);
                                } else {
                                    TT10 = keep;
                                    write_dgus_vp(0x2010, (u8*)&TT10, 1);
                                }
                            } else if (select_position == 2) {
                              keep = TT100;
                                if (TT100 == 0) {
                                    TT100 = 2;
                                } else {
                                    TT100--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2000, (u8*)&TT100, 1);
                                } else {
                                    TT100 = keep;
                                    write_dgus_vp(0x2000, (u8*)&TT100, 1);
                                }
                        }
                    } break;

                    case 14: {
                            if (select_position == 0) {
                                keep = TC1;
                                if (TC1 == 0) {
                                    TC1 = 9;
                                } else {
                                    TC1--;
                                }
                                result = (u16)(TC10 * 10 + TC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2120, (u8*)&TC1, 1);
                                } else {
                                    TC1 = keep;
                                    write_dgus_vp(0x2120, (u8*)&TC1, 1);
                                }
                             
                            } else if (select_position == 1) {
                                keep = TC10;
                                if (TC10 == 0) {
                                    TC10 = 8;
                                } else {
                                    TC10--;
                                }
                                result = (u16)(TC10 * 10 + TC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2110, (u8*)&TC10, 1);
                                } else {
                                    TC10 = keep;
                                    write_dgus_vp(0x2110, (u8*)&TC10, 1);
                                }
                            }
                    } break;

                    case 15: {
                            if (select_position == 0) {
                                keep = hundred_velue;
                                if (hundred_velue == 0) {
                                    hundred_velue = 2;
                                } else {
                                    hundred_velue--;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    hundreds[2] = hundred_velue;
                                    write_dgus_vp(0x2200, (u8*)&hundred_velue, 1);
                                } else {
                                    hundred_velue = keep;
                                    write_dgus_vp(0x2200, (u8*)&hundred_velue, 1);
                                }
                            } else if (select_position == 1) {
                                keep = ten_velue;
                                if (ten_velue == 0) {
                                    ten_velue = 5;
                                } else {
                                    ten_velue--;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    tens[2] = ten_velue;
                                    write_dgus_vp(0x2210, (u8*)&ten_velue, 1);
                                } else {
                                    ten_velue = keep;
                                    write_dgus_vp(0x2210, (u8*)&ten_velue, 1);
                                }
                            } else if (select_position == 2) {
                                keep = one_velue;
                                if (one_velue == 0) {
                                    one_velue = 9;
                                } else {
                                    one_velue--;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    ones[2] = one_velue;
                                    write_dgus_vp(0x2220, (u8*)&one_velue, 1);
                                } else {
                                    one_velue = keep;
                                    write_dgus_vp(0x2220, (u8*)&one_velue, 1);
                                }
                            
                        }
                    } break;

                    case 16: {
                            if (select_position == 0) {
                                keep = BT1;
                                if (BT1 == 0) {
                                    BT1 = 9;
                                } else {
                                    BT1--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2320, (u8*)&BT1, 1);
                                } else {
                                    BT1 = keep;
                                    write_dgus_vp(0x2320, (u8*)&BT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = BT10;
                                if (BT10 == 0 ) {
                                    BT10 = 9;
                                } else {
                                    BT10--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0 ) {
                                    write_dgus_vp(0x2310, (u8*)&BT10, 1);
                                } else {
                                    BT10 = keep;
                                    write_dgus_vp(0x2310, (u8*)&BT10, 1);
                                }
                            } else if (select_position == 2) {
                                keep = BT100;
                                if (BT100 == 0) {
                                    BT100 = 2;
                                } else {
                                    BT100--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2300, (u8*)&BT100, 1);
                                } else {
                                    BT100 = keep;
                                    write_dgus_vp(0x2300, (u8*)&BT100, 1);
                                }
                        }
                    } break;

                    case 17: {
                            if (select_position == 0) {
                                keep = BC1;
                                if (BC1 == 0) {
                                    BC1 = 9;
                                } else {
                                    BC1--;
                                }
                                result = (u16)(BC10 * 10 + BC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2420, (u8*)&BC1, 1);
                                } else {
                                    BC1 = keep;
                                    write_dgus_vp(0x2420, (u8*)&BC1, 1);
                                }
                           
                            } else if (select_position == 1) {
                         keep = BC10;
                                if (BC10 == 0) {
                                    BC10 = 8;
                                } else {
                                    BC10--;
                                }
                                result = (u16)(BC10 * 10 + BC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2410, (u8*)&BC10, 1);
                                } else {
                                    BC10 = keep;
                                    write_dgus_vp(0x2410, (u8*)&BC10, 1);
                                }
                            }
                    } break;

                    case 18: {
                            if (select_position == 0) {
                                keep = hundred_velue;
                                if (hundred_velue == 2) {
                                    hundred_velue = 0;
                                } else {
                                    hundred_velue--;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    hundreds[5] = hundred_velue;
                                    write_dgus_vp(0x2500, (u8*)&hundred_velue, 1);
                                } else {
                                    hundred_velue = keep;
                                    write_dgus_vp(0x2500, (u8*)&hundred_velue, 1);
                                }
                            } else if (select_position == 1) {
                                keep = ten_velue;
                                if (ten_velue == 0) {
                                    ten_velue = 5;
                                } else {
                                    ten_velue--;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    tens[5] = ten_velue;
                                    write_dgus_vp(0x2510, (u8*)&ten_velue, 1);
                                } else {
                                    ten_velue = keep;
                                    write_dgus_vp(0x2510, (u8*)&ten_velue, 1);
                                }
                            } else if (select_position == 2) {
                                keep = one_velue;
                                if (one_velue == 0) {
                                    one_velue = 9;
                                } else {
                                    one_velue--;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    ones[5] = one_velue;
                                    write_dgus_vp(0x2520, (u8*)&one_velue, 1);
                                } else {
                                    one_velue = keep;
                                    write_dgus_vp(0x2520, (u8*)&one_velue, 1);
                                }
                            
                        }
                    } break;

                    case 19: {
                            if (select_position == 0) {
                                 keep = min;
                                if (min == 0) {
                                     if(second != 0){
                                    min = 59;
                                }else{
                                    min = 60;
                                }
                                } else {
                                    min--;
                                }
                                  result = (u16)(min * 60 + second);
                                 if(result<= 3600 && result > 0){
                                    write_dgus_vp(0x2850, (u8*)&min, 1);
                                }else{
                                    min = keep;
                                    write_dgus_vp(0x2850, (u8*)&min, 1);
                                }
                            } else if (select_position == 1) {
                                keep = second;
                                if (second == 0) {
                                    second = 59;
                                } else {
                                    second--;
                                }
                                result = (u16)(min * 60 + second);
                                 if(result<= 3600 && result > 0){
                                    write_dgus_vp(0x2860, (u8*)&second, 1);
                                }else{
                                    second = keep;
                                    write_dgus_vp(0x2860, (u8*)&second, 1);
                                }
                                // write_dgus_vp(0x2860, (u8*)&second, 1);
                            }
                    } break;

                    case 20: {
                            if (select_position == 0 && press > 1) {
                                press--;
                                write_dgus_vp(0x2920, (u8*)&press, 1);
                            }
                    } break;

                    case 21:{
                        if(counting21 == 1){
                             if (select_position == 0) {
                               
                           keep = TT1;
                                if (TT1 == 0) {
                                    TT1 = 9;
                                } else {
                                    TT1--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2820, (u8*)&TT1, 1);
                                } else {
                                    TT1 = keep;
                                    write_dgus_vp(0x2820, (u8*)&TT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = TT10;
                                if (TT10 == 0) {
                                    TT10 = 9;
                                } else {
                                    TT10--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2810, (u8*)&TT10, 1);
                                } else {
                                    TT10 = keep;
                                    write_dgus_vp(0x2810, (u8*)&TT10, 1);
                                }
                            } else if (select_position == 2) {
                                 keep = TT100;
                                if (TT100 == 0) {
                                    TT100 = 2;
                                } else {
                                    TT100--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2800, (u8*)&TT100, 1);
                                } else {
                                    TT100 = keep;
                                    write_dgus_vp(0x2800, (u8*)&TT100, 1);
                                }
                            }
                        }else if(counting21 == 2){
                            if(press > 1){
                                press --;
                                write_dgus_vp(0x2930, (u8*)&press, 1);
                            }
                        }else if(counting21 == 3){
                           if (select_position == 0) {
                              keep = BT1;
                                if (BT1 == 0) {
                                    BT1 = 9;
                                } else {
                                    BT1--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2890, (u8*)&BT1, 1);
                                } else {
                                    BT1 = keep;
                                    write_dgus_vp(0x2890, (u8*)&BT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = BT10;
                                if (BT10 == 0) {
                                    BT10 = 9;
                                } else {
                                    BT10--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2880, (u8*)&BT10, 1);
                                } else {
                                    BT10 = keep;
                                    write_dgus_vp(0x2880, (u8*)&BT10, 1);
                                }
                            } else if (select_position == 2) {
                                         keep = BT100;
                                if (BT100 == 0) {
                                    BT100 = 2;
                                } else {
                                    BT100--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2870, (u8*)&BT100, 1);
                                } else {
                                    BT100 = keep;
                                    write_dgus_vp(0x2870, (u8*)&BT100, 1);
                                }
                            }
                        }else if(counting21 == 4){
                            if (select_position == 0) {
                                keep = min;
                                 if (min == 0) {
                                     if(second != 0){
                                    min = 59;
                                }else{
                                    min = 60;
                                }
                                } else {
                                    min--;
                                }
                                result = (u16)(min * 60 + second);
                                if(result<= 3600 && result > 0){
                                    write_dgus_vp(0x2850, (u8*)&min, 1);
                                }else{
                                    min = keep;
                                    write_dgus_vp(0x2850, (u8*)&min, 1);
                                }
                                
                            } else if (select_position == 1) {
                                keep = second;
                                if (second == 0) {
                                    second = 59;
                                } else {
                                    second--;
                                }
                                result = (u16)(min * 60 + second);
                                 if(result<= 3600 && result > 0){
                                    write_dgus_vp(0x2860, (u8*)&second, 1);
                                }else{
                                    second = keep;
                                    write_dgus_vp(0x2860, (u8*)&second, 1);
                                }
                                
                            }
                        }
                    }

                }
            } else if (page21flag == 1) { // 세팅 모드가 아닌 텍스트와 페이지 색 변경
                if (settingflag == 0) {
                    Page21InIt();
                    if (counting21 == 1) {  // 21페이지 카운팅 증가
                        counting21 = 4;
                    } else {
                        counting21--;
                    }
                    Page21Functioning(counting21);
                }
            } else {  // 페이지 전환(역방향)
                if ((page_number > 0 && page_number <= 5)) {
                    page_number--;
                    Page_Change_Handler(page_number);
                } else if (page_number == 0) {
                    page_number = 5;
                    Page_Change_Handler(page_number);
                } else if (page6flag == 1) {
                    if (page_number == 6) {
                        page_number = 9;
                        Page_Change_Handler(page_number);
                    }else if(page_number == 9){
                        page_number = 7;
                        Page_Change_Handler(page_number);
                    }else {
                        page_number--;
                        Page_Change_Handler(page_number);
                    }
                } else if (page10flag == 1) {
                    if (page_number == 9) {
                        page_number = 11;
                        Page_Change_Handler(page_number);
                    }else if(page_number == 10){
                        page_number = 9;
                        Page_Change_Handler(page_number);
                    }else {
                        page_number--;
                        Page_Change_Handler(page_number);
                    }
                }
            }
            enc_busy = 1;
            StartTimer(TMR_ENCODER, ENC_DELAY_MS);
        } break;

        case 2: {
            // 데이터 설정 모드
            if (settingflag == 1) {
                switch (page_number) {
                    case 13: {
                            if (select_position == 0) {
                                  keep = TT1;
                                if (TT1 != 9) {
                                    TT1++;
                                } else {
                                    TT1 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2020, (u8*)&TT1, 1);
                                } else {
                                    TT1 = keep;
                                    write_dgus_vp(0x2020, (u8*)&TT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = TT10;
                                if (TT10 != 9) {
                                    TT10++;
                                } else {
                                    TT10 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2010, (u8*)&TT10, 1);
                                } else {
                                    TT10 = keep;
                                    write_dgus_vp(0x2010, (u8*)&TT10, 1);
                                }
                            } else if (select_position == 2) {
                                  keep = TT100;
                                if (TT100 != 2) {
                                    TT100++;
                                } else {
                                    TT100 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2000, (u8*)&TT100, 1);
                                } else {
                                    TT100 = keep;
                                    write_dgus_vp(0x2000, (u8*)&TT100, 1);
                                }
                            
                        }
                    } break;

                    case 14: {
                            if (select_position == 0) {
                           keep = TC1;
                                if (TC1 != 9) {
                                    TC1++;
                                } else {
                                    TC1 = 0;
                                }
                                result = (u16)(TC10 * 10 + TC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2120, (u8*)&TC1, 1);
                                } else {
                                    TC1 = keep;
                                    write_dgus_vp(0x2120, (u8*)&TC1, 1);
                                }
                            } else if (select_position == 1) {
                         
                                     keep = TC10;
                                if (TC10 == 8) {
                                    TC10 = 0;
                                } else {
                                    TC10++;
                                }
                                result = (u16)(TC10 * 10 + TC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2110, (u8*)&TC10, 1);
                                } else {
                                    TC10 = keep;
                                    write_dgus_vp(0x2110, (u8*)&TC10, 1);
                                }
                            }
                    } break;

                    case 15: {
                            if (select_position == 0) {
                                keep = hundred_velue;
                                if (hundred_velue == 9) {
                                    hundred_velue = 0;
                                } else {
                                    hundred_velue++;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    hundreds[2] = hundred_velue;
                                    write_dgus_vp(0x2200, (u8*)&hundred_velue, 1);
                                } else {
                                    hundred_velue = keep;
                                    write_dgus_vp(0x2200, (u8*)&hundred_velue, 1);
                                }
                            } else if (select_position == 1) {
                                keep = ten_velue;
                                if (ten_velue == 5) {
                                    ten_velue = 0;
                                } else {
                                    ten_velue++;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    tens[2] = ten_velue;
                                    write_dgus_vp(0x2210, (u8*)&ten_velue, 1);
                                } else {
                                    ten_velue = keep;
                                    write_dgus_vp(0x2210, (u8*)&ten_velue, 1);
                                }
                            } else if (select_position == 2) {
                                keep = one_velue;
                                if (one_velue == 9) {
                                    one_velue = 0;
                                } else {
                                    one_velue++;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    ones[2] = one_velue;
                                    write_dgus_vp(0x2220, (u8*)&one_velue, 1);
                                } else {
                                    one_velue = keep;
                                    write_dgus_vp(0x2220, (u8*)&one_velue, 1);
                                }
                            
                        }
                    } break;

                    case 16: {
                            if (select_position == 0) {
                          keep = BT1;
                                if (BT1 != 9) {
                                    BT1++;
                                } else {
                                    BT1 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2320, (u8*)&BT1, 1);
                                } else {
                                    BT1 = keep;
                                    write_dgus_vp(0x2320, (u8*)&BT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = BT10;
                                if (BT10 != 9) {
                                    BT10++;
                                } else {
                                    BT10 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2310, (u8*)&BT10, 1);
                                } else {
                                    BT10 = keep;
                                    write_dgus_vp(0x2310, (u8*)&BT10, 1);
                                }
                            } else if (select_position == 2) {
                                       keep = BT100;
                                if (BT100 != 2) {
                                    BT100++;
                                } else {
                                    BT100 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2300, (u8*)&BT100, 1);
                                } else {
                                    BT100 = keep;
                                    write_dgus_vp(0x2300, (u8*)&BT100, 1);
                                }
                               
                            
                        }
                    } break;

                    case 17: {
                            if (select_position == 0) {
                              keep = BC1;
                                if (BC1 == 9) {
                                    BC1 = 0;
                                } else {
                                    BC1++;
                                }
                                result = (u16)(BC10 * 10 + BC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2420, (u8*)&BC1, 1);
                                } else {
                                    BC1 = keep;
                                    write_dgus_vp(0x2420, (u8*)&BC1, 1);
                                }
                            } else if (select_position == 1) {
                                    keep = BC10;
                                if (BC10 == 8) {
                                    BC10 = 0;
                                } else {
                                    BC10++;
                                }
                                result = (u16)(BC10 * 10 + BC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2410, (u8*)&BC10, 1);
                                } else {
                                    BC10 = keep;
                                    write_dgus_vp(0x2410, (u8*)&BC10, 1);
                                }
                                
                             
                            }
                    } break;

                    case 18: {
                            if (select_position == 0) {
                                keep = hundred_velue;
                                if (hundred_velue == 9) {
                                    hundred_velue = 0;
                                } else {
                                    hundred_velue++;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    hundreds[5] = hundred_velue;
                                    write_dgus_vp(0x2500, (u8*)&hundred_velue, 1);
                                } else {
                                    hundred_velue = keep;
                                    write_dgus_vp(0x2500, (u8*)&hundred_velue, 1);
                                }
                            } else if (select_position == 1) {
                                keep = ten_velue;
                                if (ten_velue == 5) {
                                    ten_velue = 0;
                                } else {
                                    ten_velue++;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    tens[5] = ten_velue;
                                    write_dgus_vp(0x2510, (u8*)&ten_velue, 1);
                                } else {
                                    ten_velue = keep;
                                    write_dgus_vp(0x2510, (u8*)&ten_velue, 1);
                                }
                            } else if (select_position == 2) {
                                keep = one_velue;
                                if (one_velue == 9) {
                                    one_velue = 0;
                                } else {
                                    one_velue++;
                                }
                                result = (u16)(hundred_velue * 100 + ten_velue * 10 + one_velue);
                                if (result <= 50 && result > 0) {
                                    ones[5] = one_velue;
                                    write_dgus_vp(0x2520, (u8*)&one_velue, 1);
                                } else {
                                    one_velue = keep;
                                    write_dgus_vp(0x2520, (u8*)&one_velue, 1);
                                }
                            
                        }
                    } break;

                    case 19: {
                            if (select_position == 0) {
                                keep = min;
                                if (min == 59) {
                                    if(second != 0){
                                        min = 0;
                                    }else{
                                        min++;
                                    }
                                }else if(min == 60){
                                    min = 0;
                                }else {
                                    min++;
                                }
                                result = (u16)(min * 60 + second);
                                 if(result<= 3600 && result > 0){
                                    write_dgus_vp(0x2850, (u8*)&min, 1);
                                }else{
                                    min = keep;
                                    write_dgus_vp(0x2850, (u8*)&min, 1);
                                }
                                // write_dgus_vp(0x2850, (u8*)&min, 1);
                            } else if (select_position == 1) {
                                keep = second;
                                if (second == 59) {
                                    second = 0;
                                } else {
                                    second++;
                                }
                                result = (u16)(min * 60 + second);
                                 if(result<= 3600 && result > 0){
                                    write_dgus_vp(0x2860, (u8*)&second, 1);
                                }else{
                                    second = keep;
                                    write_dgus_vp(0x2860, (u8*)&second, 1);
                                }
                                // write_dgus_vp(0x2860, (u8*)&second, 1);
                            
                        }
                    } break;

                    case 20: {
                            if (press < 20) {
                                press++;
                                write_dgus_vp(0x2920, (u8*)&press, 1);
                            }
                    } break;

                    case 21:{
                        if(counting21 == 1){
                             if (select_position == 0) {
                                 keep = TT1;
                                if (TT1 != 9) {
                                    TT1++;
                                } else {
                                    TT1 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2820, (u8*)&TT1, 1);
                                } else {
                                    TT1 = keep;
                                    write_dgus_vp(0x2820, (u8*)&TT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = TT10;
                                if (TT10 != 9) {
                                    TT10++;
                                } else {
                                    TT10 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2810, (u8*)&TT10, 1);
                                } else {
                                    TT10 = keep;
                                    write_dgus_vp(0x2810, (u8*)&TT10, 1);
                                }
                            } else if (select_position == 2) {
                                keep = TT100;
                                if (TT100 != 2) {
                                    TT100++;
                                } else {
                                    TT100 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2800, (u8*)&TT100, 1);
                                } else {
                                    TT100 = keep;
                                    write_dgus_vp(0x2800, (u8*)&TT100, 1);
                                }
                            }
                        }else if(counting21 == 2){
                            if(press != 20){
                                press ++;
                               write_dgus_vp(0x2930, (u8*)&press, 1);
                            }
                        }else if(counting21 == 3){
                           if (select_position == 0) {
                             keep = BT1;
                                if (BT1 != 9) {
                                    BT1++;
                                } else {
                                    BT1 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2890, (u8*)&BT1, 1);
                                } else {
                                    BT1 = keep;
                                    write_dgus_vp(0x2890, (u8*)&BT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = BT10;
                                if (BT10 != 9) {
                                    BT10++;
                                } else {
                                    BT10 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2880, (u8*)&BT10, 1);
                                } else {
                                    BT10 = keep;
                                    write_dgus_vp(0x2880, (u8*)&BT10, 1);
                                }
                            } else if (select_position == 2) {
                                   keep = BT100;
                                if (BT100 != 2) {
                                    BT100++;
                                } else {
                                    BT100 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= 200 && result > 0) {
                                    write_dgus_vp(0x2870, (u8*)&BT100, 1);
                                } else {
                                    BT100 = keep;
                                    write_dgus_vp(0x2870, (u8*)&BT100, 1);
                                }
                            }
                        }else if(counting21 == 4){
                            if (select_position == 0) {
                                keep = min;
                                if (min == 59) {
                                    if(second != 0){
                                        min = 0;
                                    }else{
                                        min++;
                                    }
                                }else if(min == 60){
                                    min = 0;
                                }else {
                                    min++;
                                }
                                result = (u16)(min * 60 + second);
                                 if(result<= 3600 && result > 0){
                                    write_dgus_vp(0x2850, (u8*)&min, 1);
                                }else{
                                    min = keep;
                                    write_dgus_vp(0x2850, (u8*)&min, 1);
                                }
                            } else if (select_position == 1) {
                                keep = second;
                                if (second == 59) {
                                    second = 0;
                                } else {
                                    second++;
                                }
                                result = (u16)(min * 60 + second);
                                if(result<= 3600 && result > 0){
                                    write_dgus_vp(0x2860, (u8*)&second, 1);
                                }else{
                                    second = keep;
                                    write_dgus_vp(0x2860, (u8*)&second, 1);
                                }
                                // write_dgus_vp(0x2860, (u8*)&second, 1);
                            }
                        }
                    }break;
                }
            } else if (page21flag == 1) { // 21페이지 작동 (정방향)
                if (page_number == 21) {
                    Page21InIt();
                    if (counting21 == 4) {  // 21페이지 카운팅 증가
                        counting21 = 1;
                    } else {
                        counting21++;
                    }
                    Page21Functioning(counting21);
                }
            } else { // 페이지 전환(정방향)
                if (page_number < 5) {
                    page_number++;
                    Page_Change_Handler(page_number);
                } else if (page_number == 5) {
                    page_number = 0;
                    Page_Change_Handler(page_number);
                } else if (page6flag == 1) {
                    if (page_number == 9) {
                        page_number = 6;
                        Page_Change_Handler(page_number);
                    }else if(page_number == 7){
                        page_number=9;
                        Page_Change_Handler(page_number);
                    }else {
                        page_number++;
                        Page_Change_Handler(page_number);
                    }
                } else if (page10flag == 1) {
                    if (page_number == 9) {
                        page_number = 10;
                        Page_Change_Handler(page_number);
                    }else if(page_number == 11){
                        page_number = 9;
                        Page_Change_Handler(page_number);
                    } else {
                        page_number++;
                        Page_Change_Handler(page_number);
                    }
                }
            }
            enc_busy = 1;
            StartTimer(TMR_ENCODER, ENC_DELAY_MS);
        } break;

        case 3: {
            if (settingflag == 1) {
                if(page_number == 21){
                    if(counting21 == 1){
                        select_position++;
                        if(select_position ==3){
                            settingflag = 0;
                            select_position = 0;
                            result = (u16)(TT100 * 100 + TT10 * 10 +TT1);
                            check_Start(VP_SET_TT,result);
                            QuickSettingTextSet(counting21, 100);
                        }else{
                            QuickSettingTextSet(counting21, select_position);
                        }
                    }else if(counting21 == 2){
                        settingflag = 0;
                        select_position = 0;
                        check_Start(VP_SET_P,press);
                        SetTextColorYellow(0x9253);
                    }else if(counting21 == 3){
                        select_position++;
                        if(select_position ==3){
                            settingflag = 0;
                            select_position = 0;
                            result = (u16)(BT100 * 100 + BT10 * 10 +BT1);
                            check_Start(VP_SET_TB,result);
                            QuickSettingTextSet(counting21, 100);
                        }else{
                            QuickSettingTextSet(counting21, select_position);
                        }
                    }else if(counting21 == 4){
                        select_position++;
                        if(select_position ==2){
                            settingflag = 0;
                            select_position = 0;
                            read_dgus_vp(0x2850, (u8*)&min,1);
                            read_dgus_vp(0x2860, (u8*)&second,1);
                            result = (u16)(min*60 + second);                            
                            write_dgus_vp(0x2860, (u8*)&second, 1);
                            check_Start(VP_SET_H,result);
                            write_dgus_vp(VP_TEST1, (u8*)&result, 1);
                            write_dgus_vp(VP_TEST2, (u8*)&result, 1);
                            QuickSettingTextSet(counting21, 100);
                        }else{
                            QuickSettingTextSet(counting21, select_position);
                        }
                    }
                }else{
                    select_position++;
                    select_num(page_number, select_position);
                    if(page_number == 13 && select_position == 4){
                        settingflag     = 0;
                        select_position = 0;
                        page_number = 6;
                        result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                        check_Start(VP_SET_TT,result);
                        Page_Change_Handler(page_number);
                    }else if(page_number == 14 && select_position == 3){
                        settingflag     = 0;
                        select_position = 0;
                        page_number = 7;
                        result = (u16)(TC10 * 10 + TC1);
                        check_Start(VP_SET_CHTT,result);
                        Page_Change_Handler(page_number);
                    }else if(page_number == 16 && select_position == 4){
                        settingflag     = 0;
                        select_position = 0;
                        page_number = 10;
                        result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                        check_Start(VP_SET_TB,result);
                        Page_Change_Handler(page_number);
                    }else if(page_number == 17 && select_position == 3){
                        settingflag     = 0;
                        select_position = 0;
                        page_number = 11;
                        result = (u16)(BC10 * 10 + BC1);
                        check_Start(VP_SET_CHTB,result);
                        Page_Change_Handler(page_number);
                    }else if(page_number == 19 && select_position == 3){
                        settingflag     = 0;
                        select_position = 0;
                        page_number     = 3;
                        Page_Change_Handler(page_number);
                        result = (u16)(min * 60 + second);
                        check_Start(VP_SET_H,result);
                         write_dgus_vp(VP_TEST1, (u8*)&result, 1);
                            write_dgus_vp(VP_TEST2, (u8*)&result, 1);
                    }else if(page_number == 20 && select_position == 2){
                        settingflag     = 0;
                        select_position = 0;
                        page_number     = 4;
                        Page_Change_Handler(page_number);
                        check_Start(VP_SET_P,press);
                    }else break;
                }
            } else {
                switch (page_number) {
                    case 0: {
                        read_dgus_vp(VP_SET_TT, (u8*)&result, 1);
                        TT100 = result / 100;
                        TT10   = (result / 10) % 10;
                        TT1     = result % 10;

                        write_dgus_vp(0x2800, (u8*)&TT100, 1);
                        write_dgus_vp(0x2810, (u8*)&TT10,     1);
                        write_dgus_vp(0x2820, (u8*)&TT1,     1);

                        read_dgus_vp(VP_SET_TB, (u8*)&result, 1);
                        BT100 = result / 100;
                        BT10   = (result / 10) % 10;
                        BT1     = result % 10;

                        write_dgus_vp(0x2870, (u8*)&BT100, 1);
                        write_dgus_vp(0x2880, (u8*)&BT10,     1);
                        write_dgus_vp(0x2890, (u8*)&BT1 ,     1);

                        read_dgus_vp(VP_SET_H, (u8*)&result, 2);
                        min = result / 60;
                        second = result % 60;

                        write_dgus_vp(0x2850, (u8*)&min,    2);
                        write_dgus_vp(0x2860, (u8*)&second ,     2);

                        read_dgus_vp(VP_SET_P, (u8*)&press, 2);
                        write_dgus_vp(0x2930, (u8*)&press,    2);


                        result = 0;
                        counting21 = 1;
                        Page21Functioning(counting21);

                        page_number = 21;
                        Page_Change_Handler(page_number);
                        page21flag = 1;
                    } break;

                    case 1: {
                        page6flag   = 1;
                        page_number = 6;
                        Page_Change_Handler(page_number);
                    } break;

                    case 2: {
                        page10flag  = 1;
                        page_number = 10;
                        Page_Change_Handler(page_number);
                    } break;

                    case 3: {
                        settingflag = 1;
                        page_number = 19;
                        read_dgus_vp(VP_SET_H, (u8*)&result,    1);
                        min = result / 60;
                        second = result % 60;
                        write_dgus_vp(0x2850, (u8*)&min,    1);
                        write_dgus_vp(0x2860, (u8*)&second, 1);
                        Page_Change_Handler(page_number);
                        SetTextColorBlue(0x9193);
                        SetTextColorBlack(0x9203);
                        ChangeImage(0x3970, 0);
                    } break;

                    case 4: {
                        settingflag = 1;
                        read_dgus_vp(VP_SET_P, (u8*)&press, 1);
                        write_dgus_vp(0x2920, (u8*)&press, 1);
                        page_number = 20;
                        Page_Change_Handler(page_number);
                        SetTextColorBlue(0x9213);
                        ChangeImage(0x3980, 0);
                    } break;

                    case 5: {
                        if(f_ready==1){
                            page_number = 23;
                            Page_Change_Handler(page_number);
                        }else if(f_start == 1){
                            page_number = 24;
                            Page_Change_Handler(page_number);
                        }else if(f_cooling==1){
                            page_number = 25;
                            Page_Change_Handler(page_number);
                        }else{
                            page_number = 23;
                            Page_Change_Handler(page_number);
                            SetTextColorWhite(0x9273);
                        }
                        if(deleay_flag == 0){
                            read_dgus_vp(VP_SET_H, (u8*)&result, 2);
                            min = result / 60;
                            second = result % 60;
                            write_dgus_vp(0x2850, (u8*)&min,    1);
                            write_dgus_vp(0x2860, (u8*)&second, 1);
                        } 
                    } break;

                    case 6: {
                        page_number  = 13;
                        settingflag  = 1;
                        Page_Change_Handler(page_number);
                        read_dgus_vp(VP_SET_TT, (u8*)&result, 1);
                        TT100 = result / 100;
                        TT10   = (result / 10) % 10;
                        TT1    = result % 10;
                        write_dgus_vp(0x2000, (u8*)&TT100, 1);
                        write_dgus_vp(0x2010, (u8*)&TT10,     1);
                        write_dgus_vp(0x2020, (u8*)&TT1,     1);
                        
                        select_num(page_number, 0);
                        ChangeImage(0x3910, 0);
                    } break;

                    case 7: {
                        page_number  = 14;
                        settingflag  = 1;
                        read_dgus_vp(VP_SET_CHTT, (u8*)&result, 1);
                        TC10   = (result / 10) % 10;
                        TC1     = result % 10;
                        write_dgus_vp(0x2110, (u8*)&TC10,     1);
                        write_dgus_vp(0x2120, (u8*)&TC1,     1);
                        select_num(page_number, 0);
                        Page_Change_Handler(page_number);
                        ChangeImage(0x3920, 0);
                    } break;

                    case 8: {
                        page_number  = 15;
                        settingflag  = 1;
                        Page_Change_Handler(page_number);
                        hundred_velue = hundreds[2];
                        ten_velue     = tens[2];
                        one_velue     = ones[2];
                        write_dgus_vp(0x2200, (u8*)&hundred_velue, 1);
                        write_dgus_vp(0x2210, (u8*)&ten_velue,     1);
                        write_dgus_vp(0x2220, (u8*)&one_velue,     1);
                        select_num(page_number, 0);
                        ChangeImage(0x3930, 0);
                    } break;

                    case 9: {
                        if (page6flag == 1) {
                            page6flag   = 0;
                            page_number = 1;
                            Page_Change_Handler(page_number);
                        } else {
                            page10flag  = 0;
                            page_number = 2;
                            Page_Change_Handler(page_number);
                        }
                    } break;

                    case 10: {
                        page_number  = 16;
                        settingflag  = 1;
                        Page_Change_Handler(page_number);
                        read_dgus_vp(VP_SET_TB, (u8*)&result, 1);
                        BT100 = result / 100;
                        BT10   = (result / 10) % 10;
                        BT1    = result % 10;
                        write_dgus_vp(0x2300, (u8*)&BT100, 1);
                        write_dgus_vp(0x2310, (u8*)&BT10,     1);
                        write_dgus_vp(0x2320, (u8*)&BT1,     1);
                        select_num(page_number, 0);
                        ChangeImage(0x2940, 0);
                    } break;

                    case 11: {
                        page_number  = 17;
                        settingflag  = 1;
                        read_dgus_vp(VP_SET_CHTB, (u8*)&result, 1);
                        BC10    = result / 10;
                        BC1     = result % 10;
                        write_dgus_vp(0x2410, (u8*)&BC10,     1);
                        write_dgus_vp(0x2420, (u8*)&BC1,     1);
                        select_num(page_number, 0);
                        ChangeImage(0x3950, 0);
                        Page_Change_Handler(page_number);
                    } break;

                    case 12: {
                        page_number  = 18;
                        settingflag  = 1;
                        Page_Change_Handler(page_number);
                        hundred_velue = hundreds[5];
                        ten_velue     = tens[5];
                        one_velue     = ones[5];
                        write_dgus_vp(0x2500, (u8*)&hundred_velue, 1);
                        write_dgus_vp(0x2510, (u8*)&ten_velue,     1);
                        write_dgus_vp(0x2520, (u8*)&one_velue,     1);
                        select_num(page_number, 0);
                        ChangeImage(0x3960, 0);
                    } break;

                    case 21 : {
                        if(counting21!=0){
                            settingflag = 1;
                            QuickSettingTextSet(counting21,0);
                            if(counting21 == 1){
                                 read_dgus_vp(VP_SET_TT, (u8*)&result, 1);
                                 TT100 = result / 100;
                                 TT10   = (result / 10) % 10;
                                 TT1     = result % 10;
                            }else if(counting21 ==3){
                                read_dgus_vp(VP_SET_TB, (u8*)&result, 1);
                                 BT100 = result / 100;
                                 BT10   = (result / 10) % 10;
                                 BT1     = result % 10;
                            }
                        }
                    }break;

                    case 22:{
                        page_number = 5;
                        Page_Change_Handler(page_number);
                    }break;

                    case 23:{
                        // page_number = 24;
                        // Page_Change_Handler(page_number);
                        // read_dgus_vp(VP_SET_H, (u8*)&timer, 1);
                        // ShowMMSS2(timer);
                        page_number = 5;
                        Page_Change_Handler(page_number);
                    }break;
                    // case 24:{
                    //     SecCnt_Start();
                    //     //  page_number = 25;
                    //     // Page_Change_Handler(page_number);
                    // }break;
                    // case 25:{
                    //      page_number = 5;
                    //     Page_Change_Handler(page_number);
                    // }break;
                    case 27:{
                        page_number = 0;
                        Page_Change_Handler(page_number);
                    }break;

                }
            }
        } break;

        case 4: {
            if (page_number == 21) {
                if(settingflag == 0){
                page21flag   = 0; 
                 settingflag = 0; 
                 page_number = 0; 
                 counting21 = 0; 
                 Page_Change_Handler(page_number); 
                 Page21InIt();
                }
            }
            // switch(page_number){
            //     case 24:{
            //         SecCnt_Stop();
            //         page_number = 25;
            //             Page_Change_Handler(page_number);
            // }break;
            // }
            
            ui_poll_enable = 0;
        } break;
    }
}
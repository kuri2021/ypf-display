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


#define Q_TOP_H_S 0X1000 
#define Q_TOP_H_V 0X2000 
#define Q_TOP_T_S 0X1010 
#define Q_TOP_T_V 0X2010
// #define Q_TOP_O_S 0X1020 
// #define Q_TOP_O_V 0X2020
#define Q_TOP_O_S 0X1030 
#define Q_TOP_O_V 0X2030

// #define Q_D_S 0X1040 #define Q_D_V 0X2040
#define Q_P_S 0X1050 
#define Q_P_V 0X2050

#define Q_BOT_H_S 0X1060 
#define Q_BOT_H_V 0X2060
#define Q_BOT_T_S 0X1070 
#define Q_BOT_T_V 0X2070
#define Q_BOT_O_S 0X1080 
#define Q_BOT_O_V 0X2080

#define Q_D_TEXT_S 0X1090 
#define Q_D_TEXT_V 0X2090
#define Q_D_M_S 0X1100 
#define Q_D_S_V 0X2100
#define Q_D_DIVIDE_S 0X1110 
#define Q_D_DIVIDE_V 0X2110
#define Q_D_S_S 0X1120 
#define Q_D_S_V 0X2120

// #define Q_TOP_H_S 0X1130 #define Q_TOP_H_V 0X2130
// #define Q_TOP_H_S 0X1140 #define Q_TOP_H_V 0X2140
// #define Q_TOP_H_S 0X1150 #define Q_TOP_H_V 0X2150
// #define Q_TOP_H_S 0X1160 #define Q_TOP_H_V 0X2160
// #define Q_TOP_H_S 0X1170 #define Q_TOP_H_V 0X2170
// #define Q_TOP_H_S 0X1180 #define Q_TOP_H_V 0X2180
// #define Q_TOP_H_S 0X1190 #define Q_TOP_H_V 0X2190
// #define Q_TOP_H_S 0X1200 #define Q_TOP_H_V 0X2200
// #define Q_TOP_H_S 0X1210 #define Q_TOP_H_V 0X2210
// #define Q_TOP_H_S 0X1220 #define Q_TOP_H_V 0X2220
// #define Q_TOP_H_S 0X1230 #define Q_TOP_H_V 0X2230
// #define Q_TOP_H_S 0X1240 #define Q_TOP_H_V 0X2240
// #define Q_TOP_H_S 0X1250 #define Q_TOP_H_V 0X2250
// #define Q_TOP_H_S 0X1260 #define Q_TOP_H_V 0X2260
// #define Q_TOP_H_S 0X1270 #define Q_TOP_H_V 0X2270
// #define Q_TOP_H_S 0X1280 #define Q_TOP_H_V 0X2280
// #define Q_TOP_H_S 0X1290 #define Q_TOP_H_V 0X2290
// #define Q_TOP_H_S 0X1300 #define Q_TOP_H_V 0X2300 
// #define Q_TOP_T_S 0X1310 #define Q_TOP_T_V 0X2310
// #define Q_TOP_H_S 0X1320 #define Q_TOP_H_V 0X2320
// #define Q_TOP_H_S 0X1330 #define Q_TOP_H_V 0X2330
// #define Q_TOP_H_S 0X1340 #define Q_TOP_H_V 0X2340
// #define Q_TOP_H_S 0X1350 #define Q_TOP_H_V 0X2350
// #define Q_TOP_H_S 0X1360 #define Q_TOP_H_V 0X2360
// #define Q_TOP_H_S 0X1370 #define Q_TOP_H_V 0X2370
// #define Q_TOP_H_S 0X1380 #define Q_TOP_H_V 0X2380
// #define Q_TOP_H_S 0X1390 #define Q_TOP_H_V 0X2390
// #define Q_TOP_H_S 0X1400 #define Q_TOP_H_V 0X2400
// #define Q_TOP_H_S 0X1410 #define Q_TOP_H_V 0X2410
// #define Q_TOP_H_S 0X1420 #define Q_TOP_H_V 0X2420
// #define Q_TOP_H_S 0X1430 #define Q_TOP_H_V 0X2430
// #define Q_TOP_H_S 0X1440 #define Q_TOP_H_V 0X2440
// #define Q_TOP_H_S 0X1450 #define Q_TOP_H_V 0X2450
// #define Q_TOP_H_S 0X1460 #define Q_TOP_H_V 0X2460
// #define Q_TOP_H_S 0X1470 #define Q_TOP_H_V 0X2470
// #define Q_TOP_H_S 0X1480 #define Q_TOP_H_V 0X2480
// #define Q_TOP_H_S 0X1490 #define Q_TOP_H_V 0X2490
// #define Q_TOP_H_S 0X1500 #define Q_TOP_H_V 0X2500
// #define Q_TOP_H_S 0X1510 #define Q_TOP_H_V 0X2510
// #define Q_TOP_H_S 0X1520 #define Q_TOP_H_V 0X2520
// #define Q_TOP_H_S 0X1530 #define Q_TOP_H_V 0X2530
// #define Q_TOP_H_S 0X1540 #define Q_TOP_H_V 0X2540
// #define Q_TOP_H_S 0X1550 #define Q_TOP_H_V 0X2550
// #define Q_TOP_H_S 0X1560 #define Q_TOP_H_V 0X2560
// #define Q_TOP_H_S 0X1570 #define Q_TOP_H_V 0X2570
// #define Q_TOP_H_S 0X1580 #define Q_TOP_H_V 0X2580
// #define Q_TOP_H_S 0X1590 #define Q_TOP_H_V 0X2590

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

u8 topTempChinese[] = {
    0xC9,0xCF,
    0xB2,0xBF,
    0xCE,0xC2,
    0xB6,0xC8,
    0x20,0x20,   // 공백
    0x20,0x20
};


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

#define VP_SET_LIMT_TOP_MIN          0x4100  // 상판 온도 최저값
#define VP_SET_LIMT_TOP_MAX          0x4110  // 상판 온도 최대값
#define VP_SET_LIMT_BOT_MIN          0x4130  // 하판 온도 최저값
#define VP_SET_LIMT_BOT_MAX          0x4140  // 하판 온도 최대값
#define VP_SET_LIMT_PRESS_MIN          0x4160  // 압력 최저값
#define VP_SET_LIMT_PRESS_MAX          0x4170  // 압력 최대값
#define VP_SET_LIMT_DELAY_MIN          0x4190  // 지연시간 최저값
#define VP_SET_LIMT_DELAY_MAX          0x4200  // 지연시간 최대값

#define VP_IO_TEST 0x8210  // io테스트 주소


#define VP_ELAPSED_TIME 0x8114 // 유지 경과 시간


#define SP_TXT_TT 0x5000
#define SP_TXT_TB 0x5002

#define TMR_UI_POLL 1       // 새 타이머 채널
#define UI_POLL_MS  200     // 200ms마다 화면 값 갱신

#define VP_NOR_FLASH_RW_CMD 0x0008

static u8  g_cnt_active = 0;
static u16 g_cnt_value  = 0;



u8 page_number;

static u8 read500_enable = 0;
u8 xdata page_set[4] = {0};
u8 xdata txt_off[6] = {0};

u16 Page[39] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,29,30,31,32,33,34,35,36,37,38
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

static u8 loding = 0;
static u8 main1 = 1;
static u8 main2 = 2;
static u8 main3 = 3;
static u8 main4 = 4;
static u8 main5 = 5;
static u8 main6 = 6;
static u8 main7 = 7;
static u8 quickSetting= 8;
static u8 topHeating = 9;
static u8 topCooling = 10;
static u8 topFrame = 11;
static u8 botHeating = 12;
static u8 botCooling = 13;
static u8 botFrame = 14;
static u8 exit = 15;
static u8 topHeatingS = 16;
static u8 topCoolingS = 17;
static u8 topFrameS = 18;
static u8 botHeatingS = 19;
static u8 botCoolingS = 20;
static u8 botFrameS = 21;
static u8 delayS = 22;
static u8 pressureS = 23;
static u8 workPageN = 24;
static u8 workPageH = 25;
static u8 workPageC = 26;
static u8 adminList = 27;
static u8 adminUserSetting = 28;
static u8 adminIOTest = 29;
static u8 adminFactoryResetNotice = 30;
static u8 adminLogError = 31;
static u8 adminMaintenance = 32;
static u8 adminLanguage = 33;
static u8 adminCompany = 34;
static u8 adminEngineermod = 35;
static u8 adminEngineermodS = 36;
static u8 adminActiveLog = 37;
static u8 adminErrorLog = 38;

static u16 toptempmin = 1;
static u16 toptempmax = 200;
static u16 bottempmin = 1;
static u16 bottempmax = 200;
static u16 pressmin = 0;
static u16 pressmax = 20;
static u16 delaymin = 0;
static u16 delaymax = 3600;

// static u16 language = 0;

static u16 pw = 123456;
static u16 inputpw = 0;


char language[] = "eng";

void language_change(void)
{
    write_dgus_vp(0x4270, (u8*)"TOP HEATER", strlen("TOP HEATER"));
    write_dgus_vp(0x4270, (u8*)"TOP HEATER", strlen("TOP HEATER"));
    // if(strcmp(language, "eng") == 0)
    // {
    //     write_dgus_vp(0x4270, (u8*)"TOP HEATER", 1);
    // }
}


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

void QuickSettingTextSet(u16 count, u16 select_position){
    switch(count){
        case 1:{
            switch(select_position){
                case 0:{
                    SetTextColorYellow(0x1003);
                    SetTextColorYellow(0x1013);
                    SetTextColorBlue(0x1033);
                }break;
                 case 1:{
                    SetTextColorYellow(0x1003);
                    SetTextColorBlue(0x1013);
                    SetTextColorYellow(0x1033);
                }break;
                 case 2:{
                    SetTextColorBlue(0x1003);
                    SetTextColorYellow(0x1013);
                    SetTextColorYellow(0x1033);
           
                }break;
                
                default : {
                    SetTextColorYellow(0x1003);
                    SetTextColorYellow(0x1013);
                    SetTextColorYellow(0x1033);
                }break;
            }
        }break;

        case 2:{
            SetTextColorBlue(0x1053);
        }break;

        case 3:{
             switch(select_position){
                case 0:{
                       SetTextColorYellow(0x1063);
                    SetTextColorYellow(0x1073);
                    SetTextColorBlue(0x1083);
                }break;
                 case 1:{
                    SetTextColorYellow(0x1063);
                    SetTextColorBlue(0x1073);
                    SetTextColorYellow(0x1083);
                }break;
                 case 2:{
                      SetTextColorBlue(0x1063);
                    SetTextColorYellow(0x1073);
                    SetTextColorYellow(0x1083);
                }break;
                default : {
                    SetTextColorYellow(0x1063);
                    SetTextColorYellow(0x1073);
                    SetTextColorYellow(0x1083);
                }break;
            }
        }break;
         case 4:{
              switch(select_position){
                case 0:{
                    SetTextColorBlue(0x1113);
                    SetTextColorYellow(0x1133);
                }break;
                 case 1:{
                    SetTextColorYellow(0x1113);
                    SetTextColorBlue(0x1133);
                }break;
                default : {
                    SetTextColorYellow(0x1113);
                    SetTextColorYellow(0x1133);
                }break;
            }
        }break;
    }
}

void select_num(u16 page, u16 count) { // 텍스트 색 설정
    switch (page) {
        case 16: {
            if (count == 0) {
                SetTextColorBlack(0x1153);
                SetTextColorBlack(0x1163);
                SetTextColorBlue(0x1173);
            } else if (count == 1) {
                SetTextColorBlack(0x1153);
                SetTextColorBlue(0x1163);
                SetTextColorBlack(0x1173);
            } else if (count == 2) {
                   SetTextColorBlue(0x1153);
                SetTextColorBlack(0x1163);
                SetTextColorBlack(0x1173);
            } else {
                SetTextColorBlack(0x1153);
                SetTextColorBlack(0x1163);
                SetTextColorBlack(0x1173);
            }
            if (count == 3) {
                ChangeImage(0x2180, 1);
            } else {
                ChangeImage(0x2180, 0);
            }
        } break;

        case 17: {
            if (count == 0) {
                SetTextColorBlack(0x1193);
                SetTextColorBlue(0x1203);
                
            } else if (count == 1) {
                SetTextColorBlue(0x1193);
                SetTextColorBlack(0x1203);
            }else if (count == 2) {
                SetTextColorBlack(0x1193);
                SetTextColorBlack(0x1203);
            }

            if (count == 2) {
                ChangeImage(0x2210, 1);
            } else {
                ChangeImage(0x2210, 0);
            }
        } break;

        case 18: {
            if (count == 0) {
                SetTextColorBlack(0x1223);
                SetTextColorBlack(0x9083);
                SetTextColorBlue(0x9093);
            } else if (count == 1) {
                SetTextColorBlack(0x1223);
                SetTextColorBlue(0x9083);
                SetTextColorBlack(0x9093);
            } else if (count == 2) {
              
                SetTextColorBlue(0x1223);
                SetTextColorBlack(0x9083);
                SetTextColorBlack(0x9093);
            } else {
                SetTextColorBlack(0x1223);
                SetTextColorBlack(0x9083);
                SetTextColorBlack(0x9093);
            }

            if (count == 3) {
                ChangeImage(0x3930, 1);
            } else {
                ChangeImage(0x3930, 0);
            }
        } break;

        case 19: {
            if (count == 0) {
                SetTextColorBlack(0x1263);
                SetTextColorBlack(0x1233);
                SetTextColorBlue(0x1243);
               
            } else if (count == 1) {
                SetTextColorBlack(0x1263);
                SetTextColorBlue(0x1233);
                SetTextColorBlack(0x1243);
            } else if (count == 2) {
                 SetTextColorBlue(0x1263);
                SetTextColorBlack(0x1233);
                SetTextColorBlack(0x1243);
            } else {
                SetTextColorBlack(0x1263);
                SetTextColorBlack(0x1233);
                SetTextColorBlack(0x1243);
            }

            if (count == 3) {
                ChangeImage(0x2250, 1);
            } else {
                ChangeImage(0x2250, 0);
            }
        } break;

        case 20: {
            if (count == 0) {
                     SetTextColorBlack(0x1303);
                SetTextColorBlue(0x1313);
            } else if (count == 1) {
           SetTextColorBlue(0x1303);
                SetTextColorBlack(0x1313);
            } else if (count == 2) {    
                SetTextColorBlack(0x1303);
                SetTextColorBlack(0x1313);
            }
            if (count == 2) {
                ChangeImage(0x2320, 1);
            } else {
                ChangeImage(0x2320, 0);
            }
        } break;

        case 21: {
            if (count == 0) {
                SetTextColorBlack(0x1333);
                SetTextColorBlack(0x1343);
                SetTextColorBlue(0x1353);
            } else if (count == 1) {
                SetTextColorBlack(0x1333);
                SetTextColorBlue(0x1343);
                SetTextColorBlack(0x1353);
            } else if (count == 2) {
                SetTextColorBlue(0x1333);
                SetTextColorBlack(0x1343);
                SetTextColorBlack(0x1353);
            } else {
                SetTextColorBlack(0x1333);
                SetTextColorBlack(0x1343);
                SetTextColorBlack(0x1353);
            }

            if (count == 3) {
                ChangeImage(0x2360, 1);
            } else {
                ChangeImage(0x2360, 0);
            }
        } break;

        case 22: {
            if (count == 0) {
                SetTextColorBlue(0x1373);
                SetTextColorBlack(0x1383);
            } else if (count == 1) {
                SetTextColorBlack(0x1373);
                SetTextColorBlue(0x1383);
            } else {
                SetTextColorBlack(0x1373);
                SetTextColorBlack(0x1383);
            }

            if (count == 2) {
                ChangeImage(0x2390, 1);
            } else {
                ChangeImage(0x2390, 0);
            }
        } break;

        case 23: {
            if (count == 1) {
                SetTextColorBlack(0x1403);
                ChangeImage(0x2410, 1);
            } else {
                SetTextColorBlue(0x1403);
                ChangeImage(0x2410, 0);
            }
        } break;

        // case 22: {
        //     if (count == 0) {
        //         SetTextColorBlue(0x1003);
        //         SetTextColorWhite(0x1013);
        //         SetTextColorWhite(0x1033);
        //     } else if (count == 1) {
        //         SetTextColorWhite(0x1003);
        //         SetTextColorBlue(0x1013);
        //         SetTextColorWhite(0x1033);
        //     } else if (count == 2) {
        //         SetTextColorWhite(0x1003);
        //         SetTextColorWhite(0x1013);
        //         SetTextColorBlue(0x1033);
        //     }
        // } break;

        // case 23: {
        //     if (count == 0) {
        //         SetTextColorBlue(0x9433);
        //     } else if (count == 1) {
        //         SetTextColorWhite(0x9433);
        //     }
        // } break;
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

void Page_Change_UI(u8 i){
    page_number = i;
    Page_Change_Handler(page_number);
}

void quickSettingInIt() {
    //탑 세팅온도
    SetTextColorWhite(0x1003);
    SetTextColorWhite(0x1013);
    SetTextColorWhite(0x1033);
    // 압력
    SetTextColorWhite(0x1043);
    SetTextColorWhite(0x1053);
    //바텀 세팅온도
    SetTextColorWhite(0x1063);
    SetTextColorWhite(0x1073);
    SetTextColorWhite(0x1083);
    // 시간
    SetTextColorWhite(0x1113);
    SetTextColorWhite(0x1133);
    SetTextColorWhite(0x9273);

    ChangeImage(0x3000, 0);
}

void Page21Functioning(u16 count) {
    if (count == 1) {
        SetTextColorYellow(0x1003);
        SetTextColorYellow(0x1013);
        SetTextColorYellow(0x1033);
        ChangeImage(0x3000, 1);
    } else if (count == 2) {
        SetTextColorYellow(0x1043);
        SetTextColorYellow(0x1053);
    } else if (count == 3) {
        SetTextColorYellow(0x1063);
        SetTextColorYellow(0x1073);
        SetTextColorYellow(0x1083);
        ChangeImage(0x3000, 2);
    } else if (count == 4) {
        SetTextColorYellow(0x1113);
        SetTextColorYellow(0x1133);
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
static u16 timer = 0;

static void ShowMMSS(u16 t) { 
    u16 m = t / 60; 
    u16 s = t % 60; 
    write_dgus_vp(0x2110, (u8*)&m, 1); 
    write_dgus_vp(0x2130, (u8*)&s, 1); 
}
static void ShowMMSS2(u16 t) { 
    u16 m = t / 60; 
    u16 s = t % 60; 
    write_dgus_vp(0x3100, (u8*)&m, 1); 
    write_dgus_vp(0x3200, (u8*)&s, 1); 
}

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
static u8 adminSP = 0;
static u8 admin_plag = 0;

static u8 usersettingSP = 0;
static u8 usersettingSelect_plag = 0;
static u8 usersettingEditSP = 0;

static u8 IOtestSP = 0;
static u8 IOtestSelect_plag = 0;

static u8 factoryReset_plag = 0;
static u8 factoryResetSP = 0;

static u8 LogErrorSP = 0;

static u8 maintenance_plag = 0;

static u8 LanguageSP = 0;

static u8 company_plag = 0;

static u8 EngineerSP = 0;
static u16 EngineerPw1 = 0,EngineerPw2 = 0,EngineerPw3 = 0,EngineerPw4 = 0,EngineerPw5 = 0,EngineerPw6 = 0;

void admin_text_change(){
      switch(adminSP){
        case 0:{
            SetTextColorYellow(0x5013);
            SetTextColorWhite(0x5023);
            SetTextColorWhite(0x5033);
            SetTextColorWhite(0x5043);
            SetTextColorWhite(0x5053);
            SetTextColorWhite(0x5063);
            SetTextColorWhite(0x5073);
            SetTextColorWhite(0x5643);
        }break;
        case 1:{
            SetTextColorWhite(0x5013);
            SetTextColorYellow(0x5023);
            SetTextColorWhite(0x5033);
            SetTextColorWhite(0x5043);
            SetTextColorWhite(0x5053);
            SetTextColorWhite(0x5063);
            SetTextColorWhite(0x5073);
            SetTextColorWhite(0x5643);
        }break;
        case 2:{
              SetTextColorWhite(0x5013);
            SetTextColorWhite(0x5023);
            SetTextColorYellow(0x5033);
            SetTextColorWhite(0x5043);
            SetTextColorWhite(0x5053);
            SetTextColorWhite(0x5063);
            SetTextColorWhite(0x5073);
            SetTextColorWhite(0x5643);
        }break;
        case 3:{
            SetTextColorWhite(0x5013);
            SetTextColorWhite(0x5023);
            SetTextColorWhite(0x5033);
            SetTextColorYellow(0x5043);
            SetTextColorWhite(0x5053);
            SetTextColorWhite(0x5063);
            SetTextColorWhite(0x5073);
            SetTextColorWhite(0x5643);
        }break;
        case 4:{
            SetTextColorWhite(0x5013);
            SetTextColorWhite(0x5023);
            SetTextColorWhite(0x5033);
            SetTextColorWhite(0x5043);
            SetTextColorYellow(0x5053);
            SetTextColorWhite(0x5063);
            SetTextColorWhite(0x5073);
            SetTextColorWhite(0x5643);
        }break;
        case 5:{
            SetTextColorWhite(0x5013);
            SetTextColorWhite(0x5023);
            SetTextColorWhite(0x5033);
            SetTextColorWhite(0x5043);
            SetTextColorWhite(0x5053);
            SetTextColorYellow(0x5063);
            SetTextColorWhite(0x5073);
            SetTextColorWhite(0x5643);
        }break;
        case 6:{
            SetTextColorWhite(0x5013);
            SetTextColorWhite(0x5023);
            SetTextColorWhite(0x5033);
            SetTextColorWhite(0x5043);
            SetTextColorWhite(0x5053);
            SetTextColorWhite(0x5063);
            SetTextColorYellow(0x5073);
            SetTextColorWhite(0x5643);
        }break;
        case 7:{
            SetTextColorWhite(0x5013);
            SetTextColorWhite(0x5023);
            SetTextColorWhite(0x5033);
            SetTextColorWhite(0x5043);
            SetTextColorWhite(0x5053);
            SetTextColorWhite(0x5063);
            SetTextColorWhite(0x5073);
            SetTextColorYellow(0x5643);
        }break;
    }
}

void adminUserSettingEdit(){
    switch(usersettingSP){
        case 0:{
            if(usersettingSelect_plag == 0){
                SetTextColorWhite(0x5103);
                SetTextColorWhite(0x5113);
            }else{
                if(usersettingEditSP == 0){
                    SetTextColorYellow(0x5103);
                    SetTextColorWhite(0x5113);
                }else if(usersettingEditSP == 1){
                    SetTextColorWhite(0x5103);
                    SetTextColorYellow(0x5113);
                }
            }
        }break;
        case 1:{
            if(usersettingSelect_plag == 0){
                SetTextColorWhite(0x5133);
                SetTextColorWhite(0x5143);
            }else{
                if(usersettingEditSP == 0){
                    SetTextColorYellow(0x5133);
                    SetTextColorWhite(0x5143);
                }else if(usersettingEditSP == 1){
                    SetTextColorWhite(0x5133);
                    SetTextColorYellow(0x5143);
                }
            }
        }break;
        case 2:{
            if(usersettingSelect_plag == 0){
                SetTextColorWhite(0x5163);
                SetTextColorWhite(0x5173);
            }else{
                if(usersettingEditSP == 0){
                    SetTextColorYellow(0x5163);
                    SetTextColorWhite(0x5173);
                }else if(usersettingEditSP == 1){
                    SetTextColorWhite(0x5163);
                    SetTextColorYellow(0x5173);
                }
            }
        }break;
        case 3:{
            if(usersettingSelect_plag == 0){
                SetTextColorWhite(0x5193);
                SetTextColorWhite(0x5203);
            }else{
                if(usersettingEditSP == 0){
                    SetTextColorYellow(0x5193);
                    SetTextColorWhite(0x5203);
                }else if(usersettingEditSP == 1){
                    SetTextColorWhite(0x5193);
                    SetTextColorYellow(0x5203);
                }
            }
        }break;
    }
}

void adminUserSettingText(u16 state){
    switch(state){
        case 1:{
            if(usersettingSP==0){
                usersettingSP = 3;
            }else{
                usersettingSP--;
            }
        }break;
        case 2:{
            if(usersettingSP==3){
                usersettingSP = 0;
            }else{
                usersettingSP++;
            }
        }break;
    }
    switch(usersettingSP){
        case 0:{
            SetTextColorYellow(0x5093);
            SetTextColorWhite(0x5123);
            SetTextColorWhite(0x5153);
            SetTextColorWhite(0x5183);
        }break;
        case 1:{
            SetTextColorWhite(0x5093);
            SetTextColorYellow(0x5123);
            SetTextColorWhite(0x5153);
            SetTextColorWhite(0x5183);
        }break;
        case 2:{
            SetTextColorWhite(0x5093);
            SetTextColorWhite(0x5123);
            SetTextColorYellow(0x5153);
            SetTextColorWhite(0x5183);
        }break;
        case 3:{
            SetTextColorWhite(0x5093);
            SetTextColorWhite(0x5123);
            SetTextColorWhite(0x5153);
            SetTextColorYellow(0x5183);
        }break;
    }
}

void adminIoTestText(u16 state){
    switch(state){
        case 1:{
            if(IOtestSP ==0){
                IOtestSP = 5;
            }else{
                IOtestSP--;
            }
        }break;
        case 2:{
            if(IOtestSP==5){
                IOtestSP = 0;
            }else{
                IOtestSP++;
            }
        }break;
    }

     switch(IOtestSP){
        case 0:{
            SetTextColorYellow(0x5213);
            SetTextColorWhite(0x5243);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5303);
            SetTextColorWhite(0x5343);
            SetTextColorWhite(0x5373);
        }break;
        case 1:{
            SetTextColorWhite(0x5213);
            SetTextColorYellow(0x5243);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5303);
            SetTextColorWhite(0x5343);
            SetTextColorWhite(0x5373);
        }break;
        case 2:{
            SetTextColorWhite(0x5213);
            SetTextColorWhite(0x5243);
            SetTextColorYellow(0x5273);
            SetTextColorWhite(0x5303);
            SetTextColorWhite(0x5343);
            SetTextColorWhite(0x5373);
        }break;
        case 3:{
            SetTextColorWhite(0x5213);
            SetTextColorWhite(0x5243);
            SetTextColorWhite(0x5273);
            SetTextColorYellow(0x5303);
            SetTextColorWhite(0x5343);
            SetTextColorWhite(0x5373);
        }break;
        case 4:{
            SetTextColorWhite(0x5213);
            SetTextColorWhite(0x5243);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5303);
            SetTextColorYellow(0x5343);
            SetTextColorWhite(0x5373);
        }break;
        case 5:{
            SetTextColorWhite(0x5213);
            SetTextColorWhite(0x5243);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5303);
            SetTextColorWhite(0x5343);
            SetTextColorYellow(0x5373);
        }break;
    }
}

static u8 io; 

void adminIoTestWork(){
    switch(IOtestSP){
        case 0:{
            if(IOtestSelect_plag == 1){
                SetTextColorYellow(0x5223);
                SetTextColorWhite(0x5233);
                io=1;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }else{
                SetTextColorWhite(0x5223);
                SetTextColorYellow(0x5233);
                io = 0;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }
        }break;
        case 1:{
             if(IOtestSelect_plag == 1){
                SetTextColorYellow(0x5253);
                SetTextColorWhite(0x5263);
                io=2;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }else{
                SetTextColorWhite(0x5253);
                SetTextColorYellow(0x5263);
                io = 0;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }
        }break;
        case 2:{
             if(IOtestSelect_plag == 1){
                SetTextColorYellow(0x5283);
                SetTextColorWhite(0x5293);
                io=3;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }else{
                SetTextColorWhite(0x5283);
                SetTextColorYellow(0x5293);
                io = 0;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }
        }break;
        case 3:{
             if(IOtestSelect_plag == 1){
                SetTextColorYellow(0x5313);
                SetTextColorWhite(0x5323);
                io=4;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }else{
                SetTextColorWhite(0x5313);
                SetTextColorYellow(0x5323);
                io = 0;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }
        }break;
        case 4:{
             if(IOtestSelect_plag == 1){
                SetTextColorYellow(0x5353);
                SetTextColorWhite(0x5363);
                io=5;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }else{
                SetTextColorWhite(0x5353);
                SetTextColorYellow(0x5363);
                io = 0;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }
        }break;
        case 5:{
             if(IOtestSelect_plag == 1){
                SetTextColorYellow(0x5383);
                SetTextColorWhite(0x5393);
                io=6;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }else{
                SetTextColorWhite(0x5383);
                SetTextColorYellow(0x5393);
                io = 0;
                write_dgus_vp(VP_IO_TEST, (u8*)&io, 1); 
            }
        }break;
    }
}

void adminfactoryResetText(u16 state){
     switch(state){
        case 1:{
            if(factoryResetSP ==0){
                factoryResetSP = 1;
            }else{
                factoryResetSP--;
            }
        }break;
        case 2:{
            if(factoryResetSP==1){
                factoryResetSP = 0;
            }else{
                factoryResetSP++;
            }
        }break;
    }
    if(factoryResetSP == 0){
        SetTextColorYellow(0x5793);
        SetTextColorWhite(0x5803);
    }else{
        SetTextColorWhite(0x5793);
        SetTextColorYellow(0x5803);
    }
}

void adminLogErrorText(u16 state){
     switch(state){
        case 1:{
            if(LogErrorSP  ==0){
                LogErrorSP  = 1;
            }else{
                LogErrorSP --;
            }
        }break;
        case 2:{
            if(LogErrorSP ==1){
                LogErrorSP  = 0;
            }else{
                LogErrorSP ++;
            }
        }break;
    }
    if(LogErrorSP  == 0){
        SetTextColorYellow(0x5673);
        SetTextColorWhite(0x5683);
    }else{
        SetTextColorWhite(0x5673);
        SetTextColorYellow(0x5683);
    }
}

void adminLanguageText(u16 state){
      switch(state){
        case 1:{
            if(LanguageSP  ==0){
                LanguageSP  = 2;
            }else{
                LanguageSP --;
            }
        }break;
        case 2:{
            if(LanguageSP == 2){
                LanguageSP  = 0;
            }else{
                LanguageSP ++;
            }
        }break;
    }
    if(LanguageSP  == 0){
        SetTextColorYellow(0x5553);
        SetTextColorWhite(0x5563);
        SetTextColorWhite(0x5573);
    }else if(LanguageSP  == 1){
        SetTextColorWhite(0x5553);
        SetTextColorYellow(0x5563);
        SetTextColorWhite(0x5573);
    }else if(LanguageSP  == 2){
        SetTextColorWhite(0x5553);
        SetTextColorWhite(0x5563);
        SetTextColorYellow(0x5573);
    }
}

void EngineermodTextChange(){
    switch (EngineerSP)
    {
    case 0:{
        SetTextColorYellow(0x5703);
        SetTextColorWhite(0x5713);
        SetTextColorWhite(0x5723);
        SetTextColorWhite(0x5733);
        SetTextColorWhite(0x5743);
        SetTextColorWhite(0x5753);
    }break;
    case 1:{
        SetTextColorWhite(0x5703);
        SetTextColorYellow(0x5713);
        SetTextColorWhite(0x5723);
        SetTextColorWhite(0x5733);
        SetTextColorWhite(0x5743);
        SetTextColorWhite(0x5753);
    }break;
    case 2:{
        SetTextColorWhite(0x5703);
        SetTextColorWhite(0x5713);
        SetTextColorYellow(0x5723);
        SetTextColorWhite(0x5733);
        SetTextColorWhite(0x5743);
        SetTextColorWhite(0x5753);
    }break;
    case 3:{
        SetTextColorWhite(0x5703);
        SetTextColorWhite(0x5713);
        SetTextColorWhite(0x5723);
        SetTextColorYellow(0x5733);
        SetTextColorWhite(0x5743);
        SetTextColorWhite(0x5753);
    }break;
    case 4:{
        SetTextColorWhite(0x5703);
        SetTextColorWhite(0x5713);
        SetTextColorWhite(0x5723);
        SetTextColorWhite(0x5733);
        SetTextColorYellow(0x5743);
        SetTextColorWhite(0x5753);
    }break;
    case 5:{
        SetTextColorWhite(0x5703);
        SetTextColorWhite(0x5713);
        SetTextColorWhite(0x5723);
        SetTextColorWhite(0x5733);
        SetTextColorWhite(0x5743);
        SetTextColorYellow(0x5753);
    }break;
    }
}

void admin_page_change(){
    switch(adminSP){
        case 0:{
            SetTextColorYellow(0x5093);
            SetTextColorWhite(0x5123);
            SetTextColorWhite(0x5153);
            SetTextColorWhite(0x5183);
            Page_Change_UI(adminUserSetting);
        }break;
        case 1:{
            SetTextColorYellow(0x5213);
            SetTextColorWhite(0x5243);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5303);
            SetTextColorWhite(0x5343);
            SetTextColorWhite(0x5373);
            Page_Change_UI(adminIOTest);
        }break;
        case 2:{
            SetTextColorYellow(0x5793);
            SetTextColorWhite(0x5803);
            Page_Change_UI(adminFactoryResetNotice);
        }break;
        case 3:{
            SetTextColorYellow(0x5673);
            SetTextColorWhite(0x5683);
            Page_Change_UI(adminLogError);
        }break;
        case 4:{
            Page_Change_UI(adminMaintenance);
        }break;
        case 5:{
            SetTextColorYellow(0x5553);
            SetTextColorWhite(0x5563);
            SetTextColorWhite(0x5573);
            write_dgus_vp(0x4550, topTempChinese, sizeof(topTempChinese));
            Page_Change_UI(adminLanguage);
        }break;
        case 6:{
            Page_Change_UI(adminCompany);
        }break;
        case 7:{
            Page_Change_UI(adminEngineermod);
            EngineermodTextChange();
        }break;
    }

}

void admininit(){
    adminSP = 0;
    usersettingSP = 0;
    usersettingSelect_plag = 0;
    usersettingEditSP = 0;
    IOtestSP = 0;
    factoryReset_plag = 0;
    factoryResetSP = 0;
    LogErrorSP = 0;
    maintenance_plag = 0;
    LanguageSP = 0;
    company_plag = 0;
    EngineerSP = 0;
}

void encoder_page_change(u16 state)
{
    u16 keep = 0;
    u16 result = 0;

   
    u16 f_ready, f_start, f_tempon, f_cooling, f_deleay, f_toptemp, f_bottemp,f_topcool,f_botcool, f_stop,f_run,f_pump,f_outpump,f_sensor_err;   
    static u8 enc_busy = 0;  
    static u8 select_flag    = 0;  // 타입 보강
    static u8 select_position = 0; // 타입 보강
    
    static u8 topSelectflag = 0;
	static u8 botSelectflag = 0;
    static u8 quickSettingflag = 0; 
	static u8 quickSettingS = 0;
    static u16 hundred_velue = 0; 
    static u16 ten_velue = 0;
	static u16 one_velue = 0;


    static settingflag      = 0;

    static u16 flags = 0;

    if (enc_busy && GetTimeOutFlag(TMR_ENCODER)) {
        enc_busy = 0;
    }

    if ((state == 1 || state == 2) && enc_busy) {
        return;
    }
language_change();
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
            Page_Change_UI(workPageN);
             read_dgus_vp(VP_SET_H, (u8*)&result,    1);
             min = result / 60;
             second = result % 60;
             write_dgus_vp(0x2110, (u8*)&min,    1);
             write_dgus_vp(0x2130, (u8*)&second, 1);
            quickSettingflag = 0;
            select_flag=0;
            select_position = 0;
            settingflag = 0;
            quickSettingS = 0;
            start_flag = 0;
            deleay_flag = 0;
            page24_flag = 0;
            SecCnt_Stop();
    }else if(f_tempon==1||f_bottemp==1||f_toptemp==1){
            quickSettingInIt();
            start_flag = 1;
            Page_Change_UI(workPageH);
            quickSettingflag = 0;
            select_flag=0;
            select_position = 0;
            settingflag = 0;
            quickSettingS = 0;
            error_flag = 0;
        }else if(f_deleay==1){
            quickSettingInIt();
            start_flag = 1;
            Page_Change_UI(workPageH);
            quickSettingflag = 0;
            select_flag=0;
            select_position = 0;
            settingflag = 0;
            quickSettingS = 0;
            error_flag = 0;
        }else if(f_botcool==1||f_topcool == 1){
            quickSettingInIt();
            Page_Change_UI(workPageC);
            SecCnt_Stop();
            if(timer == 1){
                timer = 0;
                ShowMMSS2(timer);
            }
            quickSettingflag = 0;
            select_flag=0;
            select_position = 0;
            settingflag = 0;
            quickSettingS = 0;
            start_flag = 1;
    }
    switch (state) {
        case 1: {
            // 데이터 설정 모드
            if (settingflag == 1) {
                if(page_number == topHeatingS){
                         if (select_position == 0) {
                                keep = TT1;
                                if (TT1 == 0) {
                                    TT1 = 9;
                                } else {
                                    TT1--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2170, (u8*)&TT1, 1);
                                } else {
                                    TT1 = keep;
                                    write_dgus_vp(0x2170, (u8*)&TT1, 1);
                                }
                              
                            } else if (select_position == 1) {
                                keep = TT10;
                                if (TT10 == 0) {
                                    TT10 = 9;
                                } else {
                                    TT10--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2160, (u8*)&TT10, 1);
                                } else {
                                    TT10 = keep;
                                    write_dgus_vp(0x2160, (u8*)&TT10, 1);
                                }
                            } else if (select_position == 2) {
                              keep = TT100;
                                if (TT100 == 0) {
                                    TT100 = 2;
                                } else {
                                    TT100--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2150, (u8*)&TT100, 1);
                                } else {
                                    TT100 = keep;
                                    write_dgus_vp(0x2150, (u8*)&TT100, 1);
                                }
                        }
                }else if(page_number == topCoolingS){
                        if (select_position == 0) {
                                keep = TC1;
                                if (TC1 == 0) {
                                    TC1 = 9;
                                } else {
                                    TC1--;
                                }
                                result = (u16)(TC10 * 10 + TC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2100, (u8*)&TC1, 1);
                                } else {
                                    TC1 = keep;
                                    write_dgus_vp(0x2200, (u8*)&TC1, 1);
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
                                    write_dgus_vp(0x2190, (u8*)&TC10, 1);
                                } else {
                                    TC10 = keep;
                                    write_dgus_vp(0x2190, (u8*)&TC10, 1);
                                }
                            }
                }else if(page_number == topFrameS){
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
                }else if(page_number == botHeatingS){
                    if (select_position == 0) {
                                keep = BT1;
                                if (BT1 == 0) {
                                    BT1 = 9;
                                } else {
                                    BT1--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2280, (u8*)&BT1, 1);
                                } else {
                                    BT1 = keep;
                                    write_dgus_vp(0x2280, (u8*)&BT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = BT10;
                                if (BT10 == 0 ) {
                                    BT10 = 9;
                                } else {
                                    BT10--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin ) {
                                    write_dgus_vp(0x2270, (u8*)&BT10, 1);
                                } else {
                                    BT10 = keep;
                                    write_dgus_vp(0x2270, (u8*)&BT10, 1);
                                }
                            } else if (select_position == 2) {
                                keep = BT100;
                                if (BT100 == 0) {
                                    BT100 = 2;
                                } else {
                                    BT100--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2260, (u8*)&BT100, 1);
                                } else {
                                    BT100 = keep;
                                    write_dgus_vp(0x2260, (u8*)&BT100, 1);
                                }
                        }
                }else if(page_number == botCoolingS){
                    if (select_position == 0) {
                                keep = BC1;
                                if (BC1 == 0) {
                                    BC1 = 9;
                                } else {
                                    BC1--;
                                }
                                result = (u16)(BC10 * 10 + BC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2310, (u8*)&BC1, 1);
                                } else {
                                    BC1 = keep;
                                    write_dgus_vp(0x2310, (u8*)&BC1, 1);
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
                                    write_dgus_vp(0x2300, (u8*)&BC10, 1);
                                } else {
                                    BC10 = keep;
                                    write_dgus_vp(0x2300, (u8*)&BC10, 1);
                                }
                            }
                }else if(page_number == botFrameS){
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
                }else if(page_number == delayS){
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
                                 if(result<= delaymax && result > delaymin){
                                    write_dgus_vp(0x2370, (u8*)&min, 1);
                                }else{
                                    min = keep;
                                    write_dgus_vp(0x2370, (u8*)&min, 1);
                                }
                            } else if (select_position == 1) {
                                keep = second;
                                if (second == 0) {
                                    second = 59;
                                } else {
                                    second--;
                                }
                                result = (u16)(min * 60 + second);
                                 if(result<= delaymax && result > delaymin){
                                    write_dgus_vp(0x2280, (u8*)&second, 1);
                                }else{
                                    second = keep;
                                    write_dgus_vp(0x2280, (u8*)&second, 1);
                                }
                            }
                }else if(page_number == pressureS){
                    if (select_position == 0 && press > pressmin) {
                        press--;
                        write_dgus_vp(0x2400, (u8*)&press, 1);
                    }
                }else if(page_number == quickSetting){
                        if(quickSettingS == 1){
                             if (select_position == 0) {
                           keep = TT1;
                                if (TT1 == 0) {
                                    TT1 = 9;
                                } else {
                                    TT1--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2030, (u8*)&TT1, 1);
                                } else {
                                    TT1 = keep;
                                    write_dgus_vp(0x2030, (u8*)&TT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = TT10;
                                if (TT10 == 0) {
                                    TT10 = 9;
                                } else {
                                    TT10--;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
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
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2000, (u8*)&TT100, 1);
                                } else {
                                    TT100 = keep;
                                    write_dgus_vp(0x2000, (u8*)&TT100, 1);
                                }
                            }
                        }else if(quickSettingS == 2){
                            if(press > pressmin){
                                press --;
                                write_dgus_vp(0x2050, (u8*)&press, 1);
                            }
                        }else if(quickSettingS == 3){
                           if (select_position == 0) {
                              keep = BT1;
                                if (BT1 == 0) {
                                    BT1 = 9;
                                } else {
                                    BT1--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2080, (u8*)&BT1, 1);
                                } else {
                                    BT1 = keep;
                                    write_dgus_vp(0x2080, (u8*)&BT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = BT10;
                                if (BT10 == 0) {
                                    BT10 = 9;
                                } else {
                                    BT10--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2070, (u8*)&BT10, 1);
                                } else {
                                    BT10 = keep;
                                    write_dgus_vp(0x2070, (u8*)&BT10, 1);
                                }
                            } else if (select_position == 2) {
                                         keep = BT100;
                                if (BT100 == 0) {
                                    BT100 = 2;
                                } else {
                                    BT100--;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2060, (u8*)&BT100, 1);
                                } else {
                                    BT100 = keep;
                                    write_dgus_vp(0x2060, (u8*)&BT100, 1);
                                }
                            }
                        }else if(quickSettingS == 4){
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
                                if(result<= delaymax && result > delaymin){
                                    write_dgus_vp(0x2110, (u8*)&min, 1);
                                }else{
                                    min = keep;
                                    write_dgus_vp(0x2110, (u8*)&min, 1);
                                }
                                
                            } else if (select_position == 1) {
                                keep = second;
                                if (second == 0) {
                                    second = 59;
                                } else {
                                    second--;
                                }
                                result = (u16)(min * 60 + second);
                                 if(result<= delaymax && result > delaymin){
                                    write_dgus_vp(0x2130, (u8*)&second, 1);
                                }else{
                                    second = keep;
                                    write_dgus_vp(0x2130, (u8*)&second, 1);
                                }
                                
                            }
                        }
                }else if(admin_plag == 1){
                if(page_number == adminUserSetting){
                    if(usersettingSP == 0){
                        if(usersettingEditSP == 0){
                        if(toptempmin>1){
                            toptempmin--;
                            write_dgus_vp(0x4100, (u8*)&toptempmin, 1);
                        }
                        
                    }else if(usersettingEditSP == 1){
                        if(toptempmax>toptempmin){
                            toptempmax--;
                            write_dgus_vp(0x4110, (u8*)&toptempmax, 1);
                        }
                    }
                    }else if(usersettingSP == 1){
                        if(usersettingEditSP == 0){
                        if(bottempmin>1){
                            bottempmin--;
                            write_dgus_vp(0x4130, (u8*)&bottempmin, 1);
                        }
                        
                    }else if(usersettingEditSP == 1){
                        if(bottempmax >bottempmin){
                            bottempmax --;
                            write_dgus_vp(0x4140, (u8*)&bottempmax , 1);
                        }
                    }
                    }else if(usersettingSP == 2){
                        if(usersettingEditSP == 0){
                        if(pressmin>0){
                            pressmin--;
                            write_dgus_vp(0x4160, (u8*)&pressmin, 1);
                        }
                        
                    }else if(usersettingEditSP == 1){
                        if(pressmax >pressmin ){
                            pressmax --;
                            write_dgus_vp(0x4170, (u8*)&pressmax , 1);
                        }
                    }
                    }else if(usersettingSP == 3){
                        if(usersettingEditSP == 0){
                        if(delaymin>0){
                            delaymin--;
                            write_dgus_vp(0x4190, (u8*)&delaymin, 1);
                        }
                        
                    }else if(usersettingEditSP == 1){
                        if(delaymax >delaymin ){
                            delaymax --;
                            write_dgus_vp(0x4200, (u8*)&delaymax , 1);
                        }
                    }
                    }
                    
                }else if(page_number == adminLogError){

                }else if(page_number == adminMaintenance){

                }else if(page_number == adminLanguage){

                }else if(page_number == adminCompany){

                }else if(page_number == adminEngineermod){

                }
                }
            } else if (quickSettingflag == 1) { // 세팅 모드가 아닌 텍스트와 페이지 색 변경
                if (settingflag == 0) {
                    quickSettingInIt();
                    if (quickSettingS == 1) {  // 21페이지 카운팅 증가
                        quickSettingS = 4;
                    } else {
                        quickSettingS--;
                    }
                    Page21Functioning(quickSettingS);
                }
            } else {  // 페이지 전환(역방향)
                if(page_number > 1 && page_number <=7){
                    page_number--;
                    Page_Change_Handler(page_number);
                }else if (page_number == main1) {
                    page_number = main7;
                    Page_Change_Handler(page_number);
                } else if (topSelectflag == 1) {
                    if (page_number == exit) {
                        Page_Change_UI(topFrame);
                    }else if(page_number == topHeating){
                        Page_Change_UI(exit);
                    }else {
                        page_number--;
                        Page_Change_Handler(page_number);
                    }
                } else if (botSelectflag == 1) {
                    if (page_number == botHeating) {
                        Page_Change_UI(exit);
                    }else {
                        page_number--;
                        Page_Change_UI(page_number);
                    }
                } else if (admin_plag ==1){
                if(page_number == adminList){
                    if(adminSP == 0){
                        adminSP = 7;
                    }else{
                        adminSP--;
                    }
                    admin_text_change();
                }else if(page_number == adminUserSetting){
                    adminUserSettingText(state);
                }else if(page_number == adminIOTest){
                    if(IOtestSelect_plag == 0){
                        adminIoTestText(state);
                    }
                }else if(page_number == adminFactoryResetNotice){
                    adminfactoryResetText(state);
                }else if(page_number == adminLogError){
                    adminLogErrorText(state);
                }else if(page_number == adminMaintenance){
                    //저장된 값만 보여줌
                }else if(page_number == adminLanguage){
                    adminLanguageText(state);
                }else if(page_number == adminCompany){
                    // 그냥 회사 소개임
                }else if(page_number == adminEngineermod){
                    switch (EngineerSP){
                    case 0:{
                        if(EngineerPw1==0){
                            EngineerPw1 = 9;
                        }else{
                            EngineerPw1--;
                        }
                        write_dgus_vp(0x4700, (u8*)&EngineerPw1 , 1);
                    }break;
                    case 1:{
                         if(EngineerPw2==0){
                            EngineerPw2 = 9;
                        }else{
                            EngineerPw2--;
                        }
                        write_dgus_vp(0x4710, (u8*)&EngineerPw2 , 1);
                    }break;
                    case 2:{
                         if(EngineerPw3==0){
                            EngineerPw3 = 9;
                        }else{
                            EngineerPw3--;
                        }
                        write_dgus_vp(0x4720, (u8*)&EngineerPw3 , 1);
                    }break;
                    case 3:{
                         if(EngineerPw4==0){
                            EngineerPw4 = 9;
                        }else{
                            EngineerPw4--;
                        }
                        write_dgus_vp(0x4730, (u8*)&EngineerPw4 , 1);
                    }break;
                    case 4:{
                         if(EngineerPw5==0){
                            EngineerPw5 = 9;
                        }else{
                            EngineerPw5--;
                        }
                        write_dgus_vp(0x4740, (u8*)&EngineerPw5 , 1);
                    }break;
                    case 5:{
                         if(EngineerPw6==0){
                            EngineerPw6 = 9;
                        }else{
                            EngineerPw6--;
                        }
                        write_dgus_vp(0x4750, (u8*)&EngineerPw6 , 1);
                    }break;

                    }
                    // 특수 기믹을 넣어야하여 넣지 않음
                }
                    
                }
            }
            enc_busy = 1;
            StartTimer(TMR_ENCODER, ENC_DELAY_MS);
        } break;

        case 2: {
            // 데이터 설정 모드
            if (settingflag == 1) {
                if(page_number == topHeatingS){
                    if (select_position == 0) {
                                  keep = TT1;
                                if (TT1 != 9) {
                                    TT1++;
                                } else {
                                    TT1 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2170, (u8*)&TT1, 1);
                                } else {
                                    TT1 = keep;
                                    write_dgus_vp(0x2170, (u8*)&TT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = TT10;
                                if (TT10 != 9) {
                                    TT10++;
                                } else {
                                    TT10 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2160, (u8*)&TT10, 1);
                                } else {
                                    TT10 = keep;
                                    write_dgus_vp(0x2160, (u8*)&TT10, 1);
                                }
                            } else if (select_position == 2) {
                                  keep = TT100;
                                if (TT100 != 2) {
                                    TT100++;
                                } else {
                                    TT100 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2150, (u8*)&TT100, 1);
                                } else {
                                    TT100 = keep;
                                    write_dgus_vp(0x2150, (u8*)&TT100, 1);
                                }
                            
                        }
                }else if(page_number == topCoolingS){
                      if (select_position == 0) {
                           keep = TC1;
                                if (TC1 != 9) {
                                    TC1++;
                                } else {
                                    TC1 = 0;
                                }
                                result = (u16)(TC10 * 10 + TC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2200, (u8*)&TC1, 1);
                                } else {
                                    TC1 = keep;
                                    write_dgus_vp(0x2200, (u8*)&TC1, 1);
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
                                    write_dgus_vp(0x2190, (u8*)&TC10, 1);
                                } else {
                                    TC10 = keep;
                                    write_dgus_vp(0x2190, (u8*)&TC10, 1);
                                }
                            }
                }else if(page_number == topFrameS){
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
                }else if(page_number == botHeatingS){
                      if (select_position == 0) {
                             keep = BT1;
                                if (BT1 != 9) {
                                    BT1++;
                                } else {
                                    BT1 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2280, (u8*)&BT1, 1);
                                } else {
                                    BT1 = keep;
                                    write_dgus_vp(0x2280, (u8*)&BT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = BT10;
                                if (BT10 != 9) {
                                    BT10++;
                                } else {
                                    BT10 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2270, (u8*)&BT10, 1);
                                } else {
                                    BT10 = keep;
                                    write_dgus_vp(0x2270, (u8*)&BT10, 1);
                                }
                            } else if (select_position == 2) {
                                       keep = BT100;
                                if (BT100 != 2) {
                                    BT100++;
                                } else {
                                    BT100 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2260, (u8*)&BT100, 1);
                                } else {
                                    BT100 = keep;
                                    write_dgus_vp(0x2260, (u8*)&BT100, 1);
                                }
                               
                            
                        }
                }else if(page_number == botCoolingS){
                    if (select_position == 0) {
                              keep = BC1;
                                if (BC1 == 9) {
                                    BC1 = 0;
                                } else {
                                    BC1++;
                                }
                                result = (u16)(BC10 * 10 + BC1);
                                if (result <= 80 && result > 0) {
                                    write_dgus_vp(0x2310, (u8*)&BC1, 1);
                                } else {
                                    BC1 = keep;
                                    write_dgus_vp(0x2310, (u8*)&BC1, 1);
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
                                    write_dgus_vp(0x2300, (u8*)&BC10, 1);
                                } else {
                                    BC10 = keep;
                                    write_dgus_vp(0x2300, (u8*)&BC10, 1);
                                }
                                
                             
                            }
                }else if(page_number == botFrameS){
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
                }else if(page_number == delayS){
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
                                 if(result<= delaymax && result > delaymin){
                                    write_dgus_vp(0x2370, (u8*)&min, 1);
                                }else{
                                    min = keep;
                                    write_dgus_vp(0x2370, (u8*)&min, 1);
                                }
                                // write_dgus_vp(0x2110, (u8*)&min, 1);
                            } else if (select_position == 1) {
                                keep = second;
                                if (second == 59) {
                                    second = 0;
                                } else {
                                    second++;
                                }
                                result = (u16)(min * 60 + second);
                                 if(result<= delaymax && result > delaymin){
                                    write_dgus_vp(0x2380, (u8*)&second, 1);
                                }else{
                                    second = keep;
                                    write_dgus_vp(0x2380, (u8*)&second, 1);
                                }
                        }
                }else if(page_number == pressureS){
                    if (press < pressmax) {
                                press++;
                                write_dgus_vp(0x2400, (u8*)&press, 1);
                            }
                }else if(page_number == quickSetting){
                    if(quickSettingS == 1){
                             if (select_position == 0) {
                                 keep = TT1;
                                if (TT1 != 9) {
                                    TT1++;
                                } else {
                                    TT1 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >=toptempmin) {
                                    write_dgus_vp(0x2030, (u8*)&TT1, 1);
                                } else {
                                    TT1 = keep;
                                    write_dgus_vp(0x2030, (u8*)&TT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = TT10;
                                if (TT10 != 9) {
                                    TT10++;
                                } else {
                                    TT10 = 0;
                                }
                                result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                                if (result <= toptempmax && result >= toptempmin) {
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
                                if (result <= toptempmax && result >= toptempmin) {
                                    write_dgus_vp(0x2000, (u8*)&TT100, 1);
                                } else {
                                    TT100 = keep;
                                    write_dgus_vp(0x2000, (u8*)&TT100, 1);
                                }
                            }
                        }else if(quickSettingS == 2){
                            if(press != pressmax){
                                press ++;
                               write_dgus_vp(0x2050, (u8*)&press, 1);
                            }
                        }else if(quickSettingS == 3){
                           if (select_position == 0) {
                             keep = BT1;
                                if (BT1 != 9) {
                                    BT1++;
                                } else {
                                    BT1 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2080, (u8*)&BT1, 1);
                                } else {
                                    BT1 = keep;
                                    write_dgus_vp(0x2080, (u8*)&BT1, 1);
                                }
                            } else if (select_position == 1) {
                                keep = BT10;
                                if (BT10 != 9) {
                                    BT10++;
                                } else {
                                    BT10 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2070, (u8*)&BT10, 1);
                                } else {
                                    BT10 = keep;
                                    write_dgus_vp(0x2070, (u8*)&BT10, 1);
                                }
                            } else if (select_position == 2) {
                                   keep = BT100;
                                if (BT100 != 2) {
                                    BT100++;
                                } else {
                                    BT100 = 0;
                                }
                                result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                                if (result <= bottempmax && result >= bottempmin) {
                                    write_dgus_vp(0x2060, (u8*)&BT100, 1);
                                } else {
                                    BT100 = keep;
                                    write_dgus_vp(0x2060, (u8*)&BT100, 1);
                                }
                            }
                        }else if(quickSettingS == 4){
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
                                 if(result<= delaymax && result >= delaymin){
                                    write_dgus_vp(0x2110, (u8*)&min, 1);
                                }else{
                                    min = keep;
                                    write_dgus_vp(0x2110, (u8*)&min, 1);
                                }
                            } else if (select_position == 1) {
                                keep = second;
                                if (second == 59) {
                                    second = 0;
                                } else {
                                    second++;
                                }
                                result = (u16)(min * 60 + second);
                                if(result<= delaymax && result >= delaymin){
                                    write_dgus_vp(0x2130, (u8*)&second, 1);
                                }else{
                                    second = keep;
                                    write_dgus_vp(0x2130, (u8*)&second, 1);
                                }
                            }
                        }
                }else if(admin_plag ==1){
                if(page_number == adminUserSetting){
                      if(usersettingSP == 0){
                        if(usersettingEditSP == 0){
                        if(toptempmin<toptempmax){
                            toptempmin++;
                            write_dgus_vp(0x4100, (u8*)&toptempmin, 1);
                        }
                    }else if(usersettingEditSP == 1){
                        if(toptempmax<220){
                            toptempmax++;
                            write_dgus_vp(0x4110, (u8*)&toptempmax, 1);
                        }
                    }
                      }else if(usersettingSP == 1){
                        if(usersettingEditSP == 0){
                        if(bottempmin <bottempmax ){
                            bottempmin ++;
                            write_dgus_vp(0x4130, (u8*)&bottempmin , 1);
                        }
                    }else if(usersettingEditSP == 1){
                        if(bottempmax<220){
                            bottempmax++;
                            write_dgus_vp(0x4140, (u8*)&bottempmax, 1);
                        }
                    }
                      }else if(usersettingSP == 2){
                        if(usersettingEditSP == 0){
                        if(pressmin<pressmax){
                            pressmin++;
                            write_dgus_vp(0x4160, (u8*)&pressmin, 1);
                        }
                    }else if(usersettingEditSP == 1){
                        if(pressmax<20){
                            pressmax++;
                            write_dgus_vp(0x4170, (u8*)&pressmax, 1);
                        }
                    }
                      }else if(usersettingSP == 3){
                        if(usersettingEditSP == 0){
                        if(delaymin<delaymax){
                            delaymin++;
                            write_dgus_vp(0x4190, (u8*)&delaymin, 1);
                        }
                    }else if(usersettingEditSP == 1){
                        if(delaymax<65535){
                            delaymax++;
                            write_dgus_vp(0x4200, (u8*)&delaymax, 1);
                        }
                    }
                      }
                }else if(page_number == adminFactoryResetNotice){
                    adminfactoryResetText(state);
                }else if(page_number == adminLogError){
                    adminLogErrorText(state);
                }else if(page_number == adminMaintenance){
                    //저장된 값만 보여줌
                }else if(page_number == adminLanguage){
                    adminLanguageText(state);
                }else if(page_number == adminEngineermod){
                    // 특수 기믹을 넣어야하여 넣지 않음
                }
            }
            }else if (quickSettingflag == 1) { // 21페이지 작동 (정방향)
                if (page_number == quickSetting) {
                    quickSettingInIt();
                    if (quickSettingS == 4) {  // 21페이지 카운팅 증가
                        quickSettingS = 1;
                    } else {
                        quickSettingS++;
                    }
                    Page21Functioning(quickSettingS);
                }
            } else { // 페이지 전환(정방향)
                if(page_number < main7 && page_number >= main1){
                    page_number++;
                    Page_Change_Handler(page_number);
                }else if (page_number == main7) {
                    page_number = main1;
                    Page_Change_Handler(page_number);
                } else if (topSelectflag == 1) {
                    if (page_number == topFrame) {
                         Page_Change_UI(exit);
                    }else if(page_number == exit){
                        Page_Change_UI(topHeating);
                    }else {
                        page_number++;
                        Page_Change_UI(page_number);
                    }
                } else if (botSelectflag == 1) {
                    if (page_number == botFrame) {
                        Page_Change_UI(exit);
                    }else if(page_number == exit){
                         Page_Change_UI(botHeating);
                    } else {
                        page_number++;
                        Page_Change_UI(page_number);
                    }
                } else if (admin_plag ==1){
                if(page_number == adminList){
                    if(adminSP == 7){
                        adminSP = 0;
                    }else{
                        adminSP++;
                    }
                    admin_text_change();
                }else if(page_number == adminUserSetting){
                    adminUserSettingText(state);
                }else if(page_number == adminIOTest){
                    if(IOtestSelect_plag == 0){
                        adminIoTestText(state);
                    }
                }else if(page_number == adminFactoryResetNotice){
                    adminfactoryResetText(state);
                }else if(page_number == adminLogError){
                    adminLogErrorText(state);
                }else if(page_number == adminMaintenance){
                    //저장된 값만 보여줌
                }else if(page_number == adminLanguage){
                    adminLanguageText(state);
                }else if(page_number == adminCompany){
                    // 그냥 회사 소개임
                }else if(page_number == adminEngineermod){
                    // 특수 기믹을 넣어야하여 넣지 않음
                    switch (EngineerSP){
                    case 0:{
                        if(EngineerPw1==9){
                            EngineerPw1 = 0;
                        }else{
                            EngineerPw1++;
                        }
                        write_dgus_vp(0x4700, (u8*)&EngineerPw1 , 1);
                    }break;
                    case 1:{
                         if(EngineerPw2==9){
                            EngineerPw2 = 0;
                        }else{
                            EngineerPw2++;
                        }
                        write_dgus_vp(0x4710, (u8*)&EngineerPw2 , 1);
                    }break;
                    case 2:{
                         if(EngineerPw3==9){
                            EngineerPw3 = 0;
                        }else{
                            EngineerPw3++;
                        }
                        write_dgus_vp(0x4720, (u8*)&EngineerPw3 , 1);
                    }break;
                    case 3:{
                         if(EngineerPw4==9){
                            EngineerPw4 = 0;
                        }else{
                            EngineerPw4++;
                        }
                        write_dgus_vp(0x4730, (u8*)&EngineerPw4 , 1);
                    }break;
                    case 4:{
                         if(EngineerPw5==9){
                            EngineerPw5 = 0;
                        }else{
                            EngineerPw5++;
                        }
                        write_dgus_vp(0x4740, (u8*)&EngineerPw5 , 1);
                    }break;
                    case 5:{
                        if(EngineerPw6==9){
                            EngineerPw6 = 0;
                        }else{
                            EngineerPw6++;
                        }
                        write_dgus_vp(0x4750, (u8*)&EngineerPw6 , 1);
                    }break;
                    }
                }
            }
            }
            enc_busy = 1;
            StartTimer(TMR_ENCODER, ENC_DELAY_MS);
        } break;

        case 3: {
            if (settingflag == 1) {
                if(page_number == quickSetting){
                    if(quickSettingS == 1){
                        select_position++;
                        if(select_position ==3){
                            settingflag = 0;
                            select_position = 0;
                            result = (u16)(TT100 * 100 + TT10 * 10 +TT1);
                            check_Start(VP_SET_TT,result);
                            QuickSettingTextSet(quickSettingS, 100);
                        }else{
                            QuickSettingTextSet(quickSettingS, select_position);
                        }
                    }else if(quickSettingS == 2){
                        settingflag = 0;
                        select_position = 0;
                        check_Start(VP_SET_P,press);
                        SetTextColorYellow(0x1053);
                    }else if(quickSettingS == 3){
                        select_position++;
                        if(select_position ==3){
                            settingflag = 0;
                            select_position = 0;
                            result = (u16)(BT100 * 100 + BT10 * 10 +BT1);
                            check_Start(VP_SET_TB,result);
                            QuickSettingTextSet(quickSettingS, 100);
                        }else{
                            QuickSettingTextSet(quickSettingS, select_position);
                        }
                    }else if(quickSettingS == 4){
                        select_position++;
                        if(select_position ==2){
                            settingflag = 0;
                            select_position = 0;
                            read_dgus_vp(0x2110, (u8*)&min,1);
                            read_dgus_vp(0x2130, (u8*)&second,1);
                            result = (u16)(min*60 + second);                            
                            write_dgus_vp(0x2130, (u8*)&second, 1);
                            check_Start(VP_SET_H,result);
                            QuickSettingTextSet(quickSettingS, 100);
                        }else{
                            QuickSettingTextSet(quickSettingS, select_position);
                        }
                    }
                }else if(admin_plag == 1){
                    if(page_number == adminList){
                    admin_page_change();
                }else if(page_number == adminUserSetting){
                    if(usersettingEditSP == 0){
                            usersettingEditSP = 1;
                    }else if(usersettingEditSP ==1){
                            usersettingEditSP = 0;
                            usersettingSelect_plag = 0;
                            settingflag = 0;
                    }
                    adminUserSettingEdit();
                }else if(page_number == adminFactoryResetNotice){
                    settingflag = 1;
                }else if(page_number == adminMaintenance){
                    if(LogErrorSP == 0){
                        Page_Change_UI(adminActiveLog);
                    }else if(LogErrorSP == 1){
                        Page_Change_UI(adminErrorLog);
                    }
                }else if(page_number == adminLanguage){
                    settingflag = 1;
                }else if(page_number == adminCompany){

                }else if(page_number == adminEngineermod){
                    settingflag = 1;
                }
                }else{
                    select_position++;
                    select_num(page_number, select_position);
                    if(page_number == topHeatingS && select_position == 4){
                        settingflag     = 0;
                        select_position = 0;
                        result = (u16)(TT100 * 100 + TT10 * 10 + TT1);
                        check_Start(VP_SET_TT,result);
                        Page_Change_UI(topHeating);
                    }else if(page_number == topCoolingS && select_position == 3){
                        settingflag     = 0;
                        select_position = 0;
                        result = (u16)(TC10 * 10 + TC1);
                        check_Start(VP_SET_CHTT,result);
                        Page_Change_UI(topCooling);
                    }else if(page_number == topFrameS && select_position == 4){
                        settingflag     = 0;
                        select_position = 0;
                        result = (u16)(BT100 * 100 + BT10 * 10 + BT1);
                        check_Start(VP_SET_TB,result);
                        Page_Change_UI(topFrame);
                    }else if(page_number == botHeatingS && select_position == 3){
                        settingflag     = 0;
                        select_position = 0;
                        result = (u16)(BC10 * 10 + BC1);
                        check_Start(VP_SET_CHTB,result);
                        Page_Change_UI(botHeating);
                    }else if(page_number == botCoolingS && select_position == 3){
                        settingflag     = 0;
                        select_position = 0;
                        result = (u16)(min * 60 + second);
                        check_Start(VP_SET_H,result);
                        Page_Change_UI(botCooling);
                    }else if(page_number == botFrameS && select_position == 2){
                        settingflag     = 0;
                        select_position = 0;
                        check_Start(VP_SET_P,press);
                        Page_Change_UI(botFrame);
                    }else break;
                }
            }else if(admin_plag == 1){
                if(page_number == adminList){
                    admin_page_change();
                }else if(page_number == adminUserSetting){
                    settingflag = 1;
                    usersettingSelect_plag = 1;
                    adminUserSettingEdit();
                }else if(page_number == adminIOTest){
                    if(IOtestSelect_plag == 0){
                        IOtestSelect_plag = 1;
                    }else{
                        IOtestSelect_plag = 0;
                    }
                    adminIoTestWork();
                }else if(page_number == adminFactoryResetNotice){
                    Page_Change_UI(adminList);
                }else if(page_number == adminLogError){
                    if(LogErrorSP == 0){
                        Page_Change_UI(adminActiveLog);
                    }else if(LogErrorSP == 1){
                        Page_Change_UI(adminErrorLog);
                    }
                }else if(page_number == adminMaintenance){
                    Page_Change_UI(main7);
                }else if(page_number == adminLanguage){
                    Page_Change_UI(main7);
                }else if(page_number == adminCompany){
                    Page_Change_UI(main7);
                }else if(page_number == adminEngineermod){
                    if(EngineerSP>=0&&EngineerSP<5){
                        EngineerSP++;
                        EngineermodTextChange();
                    }else if(EngineerSP==5){
                        EngineerSP = 0;
                        Page_Change_UI(adminEngineermodS);
                    }
                }else if(page_number == adminActiveLog){
                    Page_Change_UI(adminLogError);
                }else if(page_number == adminErrorLog){
                    Page_Change_UI(adminLogError);
                }else if(page_number == adminEngineermodS){
                    settingflag = 1;
                }
            }else{
                 if(page_number == main1){
                        read_dgus_vp(VP_SET_TT, (u8*)&result, 1);
                        TT100 = result / 100;
                        TT10   = (result / 10) % 10;
                        TT1     = result % 10;

                        write_dgus_vp(0X2000, (u8*)&TT100, 1);
                        write_dgus_vp(0x2010, (u8*)&TT10,     1);
                        write_dgus_vp(0x2030, (u8*)&TT1,     1);

                        read_dgus_vp(VP_SET_TB, (u8*)&result, 1);
                        BT100 = result / 100;
                        BT10   = (result / 10) % 10;
                        BT1     = result % 10;

                        write_dgus_vp(0x2060, (u8*)&BT100, 1);
                        write_dgus_vp(0x2070, (u8*)&BT10,     1);
                        write_dgus_vp(0x2080, (u8*)&BT1 ,     1);

                        read_dgus_vp(VP_SET_H, (u8*)&result, 2);
                        min = result / 60;
                        second = result % 60;

                        write_dgus_vp(0x2110, (u8*)&min,    2);
                        write_dgus_vp(0x2120, (u8*)&second ,     2);

                        read_dgus_vp(VP_SET_P, (u8*)&press, 2);
                        write_dgus_vp(0x2050, (u8*)&press,    2);

                        result = 0;
                        quickSettingS = 1;
                        Page21Functioning(quickSettingS);

                        page_number = quickSetting;
                        Page_Change_Handler(page_number);
                        quickSettingflag = 1;
                }else if(page_number == main2){
                    topSelectflag   = 1;
                    page_number = topHeating;
                    Page_Change_Handler(page_number);
                }else if(page_number == main3){
                    botSelectflag = 1;
                    Page_Change_UI(botHeating);
                }else if(page_number == main4){
                    settingflag = 1;
                    read_dgus_vp(VP_SET_H, (u8*)&result,    1);
                    min = result / 60;
                    second = result % 60;
                    write_dgus_vp(0x2110, (u8*)&min,    1);
                    write_dgus_vp(0x2130, (u8*)&second, 1);
                    Page_Change_Handler(page_number);
                    SetTextColorBlue(0x1373);
                    SetTextColorBlack(0x1383);
                    ChangeImage(0x2390, 0);
                    Page_Change_UI(delayS);
                }else if(page_number == main5){
                    settingflag = 1;
                    read_dgus_vp(VP_SET_P, (u8*)&press, 1);
                    write_dgus_vp(0x2920, (u8*)&press, 1);
                    SetTextColorBlue(0x1403);
                    ChangeImage(0x2410, 0);
                    Page_Change_UI(pressureS);
                }else if(page_number == main6){
                     if(f_ready==1){
                        Page_Change_UI(workPageN);
                        }else if(f_start == 1){
                            Page_Change_UI(workPageH);
                        }else if(f_cooling==1){
                            Page_Change_UI(workPageC);
                        }else{
                            Page_Change_UI(workPageN);
                        }
                        if(deleay_flag == 0){
                            read_dgus_vp(VP_SET_H, (u8*)&result, 2);
                            min = result / 60;
                            second = result % 60;
                            write_dgus_vp(0x2110, (u8*)&min,    1);
                            write_dgus_vp(0x2130, (u8*)&second, 1);
                        } 
                }else if(page_number == main7){
                    page_number = adminList;
                    admin_plag = 1;
                    Page_Change_Handler(page_number);
                    admin_text_change();
                }else if(page_number == topHeating){
                    settingflag  = 1;
                    write_dgus_vp(0X2150, (u8*)&TT100, 1);
                    write_dgus_vp(0x2160, (u8*)&TT10,     1);
                    write_dgus_vp(0x2170, (u8*)&TT1,     1);
                    select_num(page_number, 0);
                    ChangeImage(0x2180, 0);
                    Page_Change_UI(topHeatingS);
                }else if(page_number == topCooling){
                    settingflag  = 1;
                    write_dgus_vp(0x2190, (u8*)&TC10, 1);
                    write_dgus_vp(0x2200, (u8*)&TC1,     1);
                    select_num(page_number, 0);
                    ChangeImage(0x2210, 0);
                    Page_Change_UI(topCoolingS);
                }else if(page_number == topFrame){
                    Page_Change_UI(topFrameS);
                }else if(page_number == botHeating){
                    settingflag  = 1;
                    write_dgus_vp(0x2260, (u8*)&BT100, 1);
                    write_dgus_vp(0x2270, (u8*)&BT10,     1);
                    write_dgus_vp(0x2280, (u8*)&BT1,     1);
                    select_num(page_number, 0);
                    ChangeImage(0x2290, 0);
                    Page_Change_UI(botHeatingS);
                }else if(page_number == botCooling){
                    settingflag  = 1;
                    write_dgus_vp(0x2300, (u8*)&BC10,     1);
                    write_dgus_vp(0x2310, (u8*)&BC1,     1);
                    select_num(page_number, 0);
                    ChangeImage(0x2320, 0);
                    Page_Change_UI(botCoolingS);
                }else if(page_number == botFrame){
                    Page_Change_UI(botFrameS);
                }else if(page_number == exit){
                    if (topSelectflag == 1) {
                        topSelectflag = 0;
                        Page_Change_UI(main2);
                    } else {
                        botSelectflag  = 0;
                        Page_Change_UI(main3);
                    }
                }else if(page_number == workPageN){
                    Page_Change_UI(main6);
                }else if(page_number == quickSetting){
                    settingflag = 1;
                    QuickSettingTextSet(quickSettingS, 0);
                }
            }
        } break;

        case 4: {
            if(admin_plag==1){
                admininit();
                settingflag = 0;
                if(page_number == adminList){
                    admin_plag = 0;
                    Page_Change_UI(main7);
                }else{
                    Page_Change_UI(adminList);
                }
            }else if (page_number == quickSetting) {
                if(settingflag == 0){
                quickSettingflag = 0; 
                 settingflag = 0; 
                 quickSettingS = 0; 
                 Page_Change_UI(main1);
                 quickSettingInIt();
                }
            }
            ui_poll_enable = 0;
        } break;
    }
}


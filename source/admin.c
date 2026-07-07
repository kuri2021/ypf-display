#include "sys.h"
#include "language.h"
#include "admin.h"
#include "ui.h"
#include "color.h"


#define VP_ADMIN_LIST_TITLE          0X4010
#define VP_ADMIN_LIST_1          0X4020
#define VP_ADMIN_LIST_2          0X4030
#define VP_ADMIN_LIST_3          0X4040


#define VP_SET_TT         0x8100  // 상 히터 온도 설정
#define VP_SET_TB         0x8102  // 하 히터 온도 설정
#define VP_SET_P          0x8104  // 목표압력 설정
#define VP_SET_H          0x8106  // 유지시간 설정
#define VP_SET_CHTT       0x8108  // 상 냉각온도 설정
#define VP_SET_CHTB       0x810A  // 하 냉각온도 설정
#define VP_SET_NTC_BASE_T 0x810C  // 상 프레임 기준 온도 설정
#define VP_SET_NTC_BASE_B 0x8110  // 하 프레임 기준 온도 설정

#define VP_IO_TEST 0x8210  // io테스트 주소

static u16 language = 0;

u8 adminSP = 0;
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

u8 EngineerSP = 0;
u16 EngineerPw1 = 0,EngineerPw2 = 0,EngineerPw3 = 0,EngineerPw4 = 0,EngineerPw5 = 0,EngineerPw6 = 0;

static u8 io; 


static u16 pw = 123456;

u16 topTempDefault = 160;
u16 botTempDefault = 160;
u16 pressuTempDefault = 160;
u16 topCoolingDefault = 60;
u16 botCoolingDefault = 60;
u16 delayDefault = 90;

u16 toptempmin = 1;
u16 toptempmax = 200;
u16 bottempmin = 1;
u16 bottempmax = 200;
u16 pressmin = 0;
u16 pressmax = 20;
u16 delaymin = 0;
u16 delaymax = 3600;

//어드민 리스트
void admin_List_text_change(){
    u16 len = 0;
     switch(adminSP){
        case 0:{
            if(language == 0){
                write_dgus_vp(0x4020, (u8*)&ENGINEER_MODE_ENG, 13);
                len = 26;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&USER_SETTING_ENG, 12);
                len = 24;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&IO_TEST_ENG, 12);
                len = 14;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4020, (u8*)&ENGINEER_MODE_CHN, 5);
                len = 10;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&USER_SETTING_CHN, 4);
                len = 8;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&IO_TEST_CHN, 4);
                len = 8;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4020, (u8*)&ENGINEER_MODE_KOR, 7);
                len = 14;
                write_dgus_vp(0x5028, (u8*)&len, 1);
                
                write_dgus_vp(0x4030, (u8*)&USER_SETTING_KOR, 5);
                len = 10;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&CHECK_KOR, 6);
                len = 12;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }
        }break;
        case 1:{
            if(language == 0){
                write_dgus_vp(0x4020, (u8*)&USER_SETTING_ENG, 12);
                len = 24;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&IO_TEST_ENG, 7);
                len = 14;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&DATA_INIT_ENG, 9);
                len = 18;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4020, (u8*)&USER_SETTING_CHN, 4);
                len = 8;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&IO_TEST_CHN, 4);
                len = 8;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&FACTORY_RESET_CHN, 5);
                len = 10;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4020, (u8*)&USER_SETTING_KOR, 5);
                len = 10;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&CHECK_KOR, 6);
                len = 12;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&FACTORY_RESET_KOR, 6);
                len = 12;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }
        }break;
        case 2:{
            if(language == 0){
               write_dgus_vp(0x4020, (u8*)&IO_TEST_ENG, 7);
                len = 14;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&DATA_INIT_ENG, 9);
                len = 18;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&LOG_ERROR_ENG, 11);
                len = 22;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4020, (u8*)&IO_TEST_CHN, 4);
                len = 8;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&FACTORY_RESET_CHN, 5);
                len = 10;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&LOG_ERROR_CHN, 7);
                len = 14;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4020, (u8*)&CHECK_KOR, 6);
                len = 12;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&FACTORY_RESET_KOR, 6);
                len = 12;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&LOG_ERROR_KOR, 7);
                len = 14;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }
           
        }break;
        case 3:{
            if(language == 0){
               write_dgus_vp(0x4020, (u8*)&DATA_INIT_ENG, 9);
                len = 18;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&LOG_ERROR_ENG, 11);
                len = 22;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&RUN_TIME_ENG, 8);
                len = 16;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4020, (u8*)&FACTORY_RESET_CHN, 5);
                len = 10;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&LOG_ERROR_CHN, 7);
                len = 14;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&RUN_TIME_CHN, 4);
                len = 8;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4020, (u8*)&FACTORY_RESET_KOR, 6);
                len = 12;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&LOG_ERROR_KOR, 7);
                len = 14;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&WORK_TIME_KOR, 5);
                len = 10;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }
        }break;
        case 4:{
            if(language == 0){
               write_dgus_vp(0x4020, (u8*)&LOG_ERROR_ENG, 11);
                len = 22;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&RUN_TIME_ENG, 8);
                len = 16;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&LANGUAGE_ENG, 8);
                len = 16;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4020, (u8*)&LOG_ERROR_CHN, 7);
                len = 14;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&RUN_TIME_CHN, 4);
                len = 8;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&LANGUAGE_CHN, 2);
                len = 4;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4020, (u8*)&LOG_ERROR_KOR, 7);
                len = 14;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&WORK_TIME_KOR, 5);
                len = 10;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&LANGUAGE_KOR, 2);
                len = 4;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }
        }break;
        case 5:{
          if(language == 0){
               write_dgus_vp(0x4020, (u8*)&RUN_TIME_ENG, 8);
                len = 16;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&LANGUAGE_ENG, 8);
                len = 16;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&COMPANY_INFO_ENG, 12);
                len = 24;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4020, (u8*)&RUN_TIME_CHN, 4);
                len = 8;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&LANGUAGE_CHN, 2);
                len = 4;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&COMPANY_INFO_CHN, 4);
                len = 8;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4020, (u8*)&WORK_TIME_KOR, 5);
                len = 10;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&LANGUAGE_KOR, 2);
                len = 4;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&COMPANY_KOR, 4);
                len = 8;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }
        }break;
        case 6:{
            if(language == 0){
               write_dgus_vp(0x4020, (u8*)&LANGUAGE_ENG, 8);
                len = 16;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&COMPANY_INFO_ENG, 12);
                len = 24;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&ENGINEER_MODE_ENG, 13);
                len = 26;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4020, (u8*)&LANGUAGE_CHN, 2);
                len = 4;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&COMPANY_INFO_CHN, 4);
                len = 8;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&ENGINEER_MODE_CHN, 5);
                len = 10;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4020, (u8*)&LANGUAGE_KOR, 2);
                len = 4;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&COMPANY_KOR, 4);
                len = 8;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&ENGINEER_MODE_KOR, 7);
                len = 14;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }
        }break;
        case 7:{
            if(language == 0){
               write_dgus_vp(0x4020, (u8*)&COMPANY_INFO_ENG, 12);
                len = 24;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&ENGINEER_MODE_ENG, 13);
                len = 26;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&USER_SETTING_ENG, 12);
                len = 24;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4020, (u8*)&COMPANY_INFO_CHN, 4);
                len = 8;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&ENGINEER_MODE_CHN, 5);
                len = 10;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&USER_SETTING_CHN, 4);
                len = 8;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4020, (u8*)&COMPANY_KOR, 4);
                len = 8;
                write_dgus_vp(0x5028, (u8*)&len, 1);

                write_dgus_vp(0x4030, (u8*)&ENGINEER_MODE_KOR, 7);
                len = 14;
                write_dgus_vp(0x5038, (u8*)&len, 1);

                write_dgus_vp(0x4040, (u8*)&USER_SETTING_KOR, 5);
                len = 10;
                write_dgus_vp(0x5048, (u8*)&len, 1);
            }
        }break;
    }
}

void admin_List(u16 i){
    if (i == 1)
    {
        if(adminSP==0){
            adminSP = 7;
        }else{
            adminSP--;
        }
    }else if(i == 2){
        if(adminSP == 7){
            adminSP = 0;
        }else{
            adminSP++;
        }
    }
    admin_List_text_change(); 
}
//어드민 유저 세팅
void admin_User_Setting_text_color(){
    if(usersettingSP == 0){
        SetTextColorYellow(0x5063);
        SetTextColorWhite(0x5093);
        SetTextColorWhite(0x5123);
        SetTextColorWhite(0x5153);
        if(usersettingSelect_plag == 1){
            if(usersettingEditSP == 0){
                SetTextColorYellow(0x5073);
                SetTextColorWhite(0x5083);
            }else if(usersettingEditSP == 1){
                SetTextColorWhite(0x5073);
                SetTextColorYellow(0x5083);
            }
        }else{
            SetTextColorWhite(0x5073);
            SetTextColorWhite(0x5083);
        }
    }else if(usersettingSP == 1){
        SetTextColorWhite(0x5063);
        SetTextColorYellow(0x5093);
        SetTextColorWhite(0x5123);
        SetTextColorWhite(0x5153);
        if(usersettingSelect_plag == 1){
            if(usersettingEditSP == 0){
                SetTextColorYellow(0x5103);
                SetTextColorWhite(0x5113);
            }else if(usersettingEditSP == 1){
                SetTextColorWhite(0x5103);
                SetTextColorYellow(0x5113);
            }
        }else{
            SetTextColorWhite(0x5103);
            SetTextColorWhite(0x5113);
        }
    }else if(usersettingSP == 2){
        SetTextColorWhite(0x5063);
        SetTextColorWhite(0x5093);
        SetTextColorYellow(0x5123);
        SetTextColorWhite(0x5153);
        if(usersettingSelect_plag == 1){
            if(usersettingEditSP == 0){
                SetTextColorYellow(0x5133);
                SetTextColorWhite(0x5143);
            }else if(usersettingEditSP == 1){
                SetTextColorWhite(0x5133);
                SetTextColorYellow(0x5143);
            }
        }else{
            SetTextColorWhite(0x5133);
            SetTextColorWhite(0x5143);
        }
    }else if(usersettingSP == 3){
        SetTextColorWhite(0x5063);
        SetTextColorWhite(0x5093);
        SetTextColorWhite(0x5123);
        SetTextColorYellow(0x5153);
        if(usersettingSelect_plag == 1){
            if(usersettingEditSP == 0){
                SetTextColorYellow(0x5163);
                SetTextColorWhite(0x5173);
            }else if(usersettingEditSP == 1){
                SetTextColorWhite(0x5163);
                SetTextColorYellow(0x5173);
            }
        }else{
            SetTextColorWhite(0x5163);
            SetTextColorWhite(0x5173);
        }
    }
   
}

void admin_User_Setting_Function(u16 i){
    if(i == 0){
        usersettingSP = 0;
        usersettingSelect_plag = 0;
    }else if(i == 1){
        if(usersettingSelect_plag == 0){
            if(usersettingSP == 0){
                usersettingSP = 3;
            }else{
                usersettingSP--;
            }
        }else{
            if(usersettingEditSP==0){
                if(usersettingSP == 0){
                    if(toptempmin>1 && toptempmax > toptempmin){
                        toptempmin--;
                    }
                    write_dgus_vp(0x4070, (u8*)&toptempmin, 1);
                }else if(usersettingSP == 1){
                    if(bottempmin>1 && bottempmax > bottempmin){
                        bottempmin--;
                    }
                    write_dgus_vp(0x4100, (u8*)&bottempmin, 1);
                }else if(usersettingSP == 2){
                    if(pressmin>0 && pressmax > pressmin){
                        pressmin--;
                    }
                    write_dgus_vp(0x4130, (u8*)&pressmin, 1);
                }else if(usersettingSP == 3){
                    if(delaymin>1 && delaymax > delaymin){
                        delaymin--;
                    }
                    write_dgus_vp(0x4160, (u8*)&delaymin, 1);
                }
            }else if(usersettingEditSP == 1){
                if(usersettingSP == 0){
                    if(toptempmax > toptempmin){
                        toptempmax--;
                    }
                    write_dgus_vp(0x4080, (u8*)&toptempmax, 1);
                }else if(usersettingSP == 1){
                    if(bottempmax > bottempmin){
                        bottempmax--;
                    }
                    write_dgus_vp(0x4110, (u8*)&bottempmax, 1);
                }else if(usersettingSP == 2){
                    if(pressmax > pressmin){
                        pressmax--;
                    }
                    write_dgus_vp(0x4140, (u8*)&pressmax, 1);
                }else if(usersettingSP == 3){
                    if(delaymax > delaymin){
                        delaymax--;
                    }
                    write_dgus_vp(0x4170, (u8*)&delaymax, 1);
                }
            }
        }
        admin_User_Setting_text_color();
    }else if(i == 2){
        if(usersettingSelect_plag == 0){
            if(usersettingSP == 3){
                usersettingSP = 0;
            }else{
                usersettingSP++;
            }
        }else {
            if(usersettingEditSP==0){
                if(usersettingSP == 0){
                    if(toptempmax > toptempmin){
                        toptempmin++;
                    }
                    write_dgus_vp(0x4070, (u8*)&toptempmin, 1);
                }else if(usersettingSP == 1){
                    if(bottempmax > bottempmin){
                        bottempmin++;
                    }
                    write_dgus_vp(0x4100, (u8*)&bottempmin, 1);
                }else if(usersettingSP == 2){
                    if(pressmax > pressmin){
                        pressmin++;
                    }
                    write_dgus_vp(0x4130, (u8*)&pressmin, 1);
                }else if(usersettingSP == 3){
                    if(delaymax > delaymin){
                        delaymin++;
                    }
                    write_dgus_vp(0x4160, (u8*)&delaymin, 1);
                }
            }else if(usersettingEditSP == 1){
                if(usersettingSP == 0){
                    if(toptempmax < 220 && toptempmax > toptempmin){
                        toptempmax++;
                    }
                    write_dgus_vp(0x4080, (u8*)&toptempmax, 1);
                }else if(usersettingSP == 1){
                    if(bottempmax < 220 && bottempmax > bottempmin){
                        bottempmax++;
                    }
                    write_dgus_vp(0x4110, (u8*)&bottempmax, 1);
                }else if(usersettingSP == 2){
                    if(pressmax < 20 && pressmax > pressmin){
                        pressmax++;
                    }
                    write_dgus_vp(0x4140, (u8*)&pressmax, 1);
                }else if(usersettingSP == 3){
                    if(delaymax < 180 &&delaymax > delaymin){
                        delaymax++;
                    }
                    write_dgus_vp(0x4170, (u8*)&delaymax, 1);
                }
            }
        }
        admin_User_Setting_text_color();
    }else if(i == 3){
        if(usersettingSelect_plag == 0){
            usersettingSelect_plag = 1;
        }else {
            if(usersettingEditSP == 0){
                usersettingEditSP = 1;
            }else if(usersettingEditSP == 1){
                usersettingSelect_plag = 0;
                usersettingEditSP = 0;
            }
        }
        admin_User_Setting_text_color();
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

//어드민 io테스트
void adminIoTestText(u16 state){
    if(IOtestSelect_plag ==0){
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
    }
    

     switch(IOtestSP){
        case 0:{
            SetTextColorYellow(0x5193);
            SetTextColorWhite(0x5213);
            SetTextColorWhite(0x5233);
            SetTextColorWhite(0x5253);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5293);
        }break;
        case 1:{
            SetTextColorWhite(0x5193);
            SetTextColorYellow(0x5213);
            SetTextColorWhite(0x5233);
            SetTextColorWhite(0x5253);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5293);
        }break;
        case 2:{
            SetTextColorWhite(0x5193);
            SetTextColorWhite(0x5213);
            SetTextColorYellow(0x5233);
            SetTextColorWhite(0x5253);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5293);
        }break;
        case 3:{
            SetTextColorWhite(0x5193);
            SetTextColorWhite(0x5213);
            SetTextColorWhite(0x5233);
            SetTextColorYellow(0x5253);
            SetTextColorWhite(0x5273);
            SetTextColorWhite(0x5293);
        }break;
        case 4:{
            SetTextColorWhite(0x5193);
            SetTextColorWhite(0x5213);
            SetTextColorWhite(0x5233);
            SetTextColorWhite(0x5253);
            SetTextColorYellow(0x5273);
            SetTextColorWhite(0x5293);
        }break;
        case 5:{
            SetTextColorWhite(0x5193);
            SetTextColorWhite(0x5213);
            SetTextColorWhite(0x5233);
            SetTextColorWhite(0x5253);
            SetTextColorWhite(0x5273);
            SetTextColorYellow(0x5293);
        }break;
    }
}

void adminIoTestWork(void){
    u16 default_result = 10;
    u16 len = 0;
    if(IOtestSelect_plag == 0){
        IOtestSelect_plag = 1;
        write_dgus_vp(VP_IO_TEST, (u8*)&IOtestSP, 1);
        switch (IOtestSP){
        case 0:{
            write_dgus_vp(0x4200, (u8*)&ON_TEXT, 2);
            len = 4;
            write_dgus_vp(0x5208, (u8*)&len, 1);

            write_dgus_vp(0x4220, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5228, (u8*)&len, 1);

            write_dgus_vp(0x4240, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5248, (u8*)&len, 1);

            write_dgus_vp(0x4260, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5268, (u8*)&len, 1);

            write_dgus_vp(0x4280, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5288, (u8*)&len, 1);

            write_dgus_vp(0x4300, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5308, (u8*)&len, 1);
        }break;

        case 1:{
            write_dgus_vp(0x4200, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5208, (u8*)&len, 1);

            write_dgus_vp(0x4220, (u8*)&ON_TEXT, 2);
            len = 4;
            write_dgus_vp(0x5228, (u8*)&len, 1);

            write_dgus_vp(0x4240, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5248, (u8*)&len, 1);

            write_dgus_vp(0x4260, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5268, (u8*)&len, 1);

            write_dgus_vp(0x4280, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5288, (u8*)&len, 1);

            write_dgus_vp(0x4300, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5308, (u8*)&len, 1);
        }break;

        case 2:{
            write_dgus_vp(0x4200, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5208, (u8*)&len, 1);

            write_dgus_vp(0x4220, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5228, (u8*)&len, 1);

            write_dgus_vp(0x4240, (u8*)&ON_TEXT, 2);
            len = 4;
            write_dgus_vp(0x5248, (u8*)&len, 1);

            write_dgus_vp(0x4260, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5268, (u8*)&len, 1);

            write_dgus_vp(0x4280, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5288, (u8*)&len, 1);

            write_dgus_vp(0x4300, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5308, (u8*)&len, 1);
        }break;

        case 3:{
            write_dgus_vp(0x4200, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5208, (u8*)&len, 1);

            write_dgus_vp(0x4220, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5228, (u8*)&len, 1);

            write_dgus_vp(0x4240, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5248, (u8*)&len, 1);

            write_dgus_vp(0x4260, (u8*)&ON_TEXT, 2);
            len = 4;
            write_dgus_vp(0x5268, (u8*)&len, 1);

            write_dgus_vp(0x4280, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5288, (u8*)&len, 1);

            write_dgus_vp(0x4300, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5308, (u8*)&len, 1);
        }break;

        case 4:{
            write_dgus_vp(0x4200, (u8*)&ON_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5208, (u8*)&len, 1);

            write_dgus_vp(0x4220, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5228, (u8*)&len, 1);

            write_dgus_vp(0x4240, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5248, (u8*)&len, 1);

            write_dgus_vp(0x4260, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5268, (u8*)&len, 1);

            write_dgus_vp(0x4280, (u8*)&ON_TEXT, 2);
            len = 4;
            write_dgus_vp(0x5288, (u8*)&len, 1);

            write_dgus_vp(0x4300, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5308, (u8*)&len, 1);
        }break;

        case 5:{
            write_dgus_vp(0x4200, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5208, (u8*)&len, 1);

            write_dgus_vp(0x4220, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5228, (u8*)&len, 1);

            write_dgus_vp(0x4240, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5248, (u8*)&len, 1);

            write_dgus_vp(0x4260, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5268, (u8*)&len, 1);

            write_dgus_vp(0x4280, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5288, (u8*)&len, 1);

            write_dgus_vp(0x4300, (u8*)&ON_TEXT, 2);
            len = 4;
            write_dgus_vp(0x5308, (u8*)&len, 1);
        }break;
        
        default:
            break;
        }
    }else{
        IOtestSelect_plag = 0;
        write_dgus_vp(VP_IO_TEST, (u8*)&default_result, 1);
        write_dgus_vp(0x4200, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5208, (u8*)&len, 1);

            write_dgus_vp(0x4220, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5228, (u8*)&len, 1);

            write_dgus_vp(0x4240, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5248, (u8*)&len, 1);

            write_dgus_vp(0x4260, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5268, (u8*)&len, 1);

            write_dgus_vp(0x4280, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5288, (u8*)&len, 1);

            write_dgus_vp(0x4300, (u8*)&OFF_TEXT, 3);
            len = 6;
            write_dgus_vp(0x5308, (u8*)&len, 1);
    }
}

//어드민 초기화
void adminfactoryResetText(u16 state){
    if(state == 1||state == 2){
        if(factoryResetSP == 0){
            factoryResetSP = 1;
        }else{
            factoryResetSP = 0;
        }
        if(factoryResetSP == 0){
            SetTextColorYellow(0x5343);
            SetTextColorWhite(0x5353);
        }else{
            SetTextColorWhite(0x5343);
            SetTextColorYellow(0x5353);
        }
    }else if(state == 3){
        if(factoryResetSP == 0){
            page_number = 27;
            Page_Change_Handler(page_number);
        }else if(factoryResetSP == 1){
            write_dgus_vp(VP_SET_TT, (u8*)&topTempDefault, 1);
            write_dgus_vp(VP_SET_TB, (u8*)&botTempDefault, 1);
            write_dgus_vp(VP_SET_P, (u8*)&pressuTempDefault, 1);
            write_dgus_vp(VP_SET_CHTT, (u8*)&topCoolingDefault, 1);
            write_dgus_vp(VP_SET_CHTB, (u8*)&botCoolingDefault, 1);
            write_dgus_vp(VP_SET_H, (u8*)&delayDefault, 1);
            page_number = 27;
            Page_Change_Handler(page_number);
        }
    }
   
}

//어드민 로그 및 에러
void adminLogErrorText(u16 state){
    if(state == 1 || state == 2){
    if(LogErrorSP  == 0){
        SetTextColorYellow(0x5373);
        SetTextColorWhite(0x5383);
    }else{
        SetTextColorWhite(0x5373);
        SetTextColorYellow(0x5383);
    }
    }else if(state == 3){
        if(LogErrorSP  == 0){
            page_number = 33;
            Page_Change_Handler(page_number);
        }else{
            page_number = 34;
            Page_Change_Handler(page_number);
        }
    }
    
}
//어드민 언어 변경
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
        case 3:{
            if(LanguageSP == 0){
                language = 0;
                admin_language_eng();
            }else if(LanguageSP == 1){
                language = 1;
                admin_language_chn();
            }else if(LanguageSP ==2){
                language = 2;
                admin_language_kor();
            }
        }
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

void admin_language_eng(void){
    u16 len = 0;
    //어드민 메인리스트
    admin_List_text_change();

  
    write_dgus_vp(0x4010, (u8*)&ADMIN, 5);
            len = 10;
            write_dgus_vp(0x5018, (u8*)&len, 1);
    
            write_dgus_vp(0x4050, (u8*)&USER_SETTING_ENG, 12);
            len = 24;
            write_dgus_vp(0x5058, (u8*)&len, 1);
     
            write_dgus_vp(0x4180, (u8*)&IO_TEST_ENG, 7);
            len = 14;
            write_dgus_vp(0x5180, (u8*)&len, 1);
       
            write_dgus_vp(0x4310, (u8*)&DATA_INIT_ENG, 9);
            len = 18;
            write_dgus_vp(0x5318, (u8*)&len, 1);
      
            write_dgus_vp(0x4360, (u8*)&LOG_ERROR_ENG, 11);
            len = 22;
            write_dgus_vp(0x5368, (u8*)&len, 1);
        
            write_dgus_vp(0x4390, (u8*)&LOG_ERROR_ENG, 11);
            len = 22;
            write_dgus_vp(0x5398, (u8*)&len, 1);
        
            write_dgus_vp(0x4400, (u8*)&LOG_ERROR_ENG, 11);
            len = 22;
            write_dgus_vp(0x5408, (u8*)&len, 1);
       
            write_dgus_vp(0x4410, (u8*)&RUN_TIME_ENG, 8);
            len = 16;
            write_dgus_vp(0x5418, (u8*)&len, 1);
        
            write_dgus_vp(0x4540, (u8*)&LANGUAGE_ENG, 8);
            len = 16;
            write_dgus_vp(0x5548, (u8*)&len, 1);
        
            write_dgus_vp(0x4580, (u8*)&ENGINEER_MODE_ENG, 13);
            len = 26;
            write_dgus_vp(0x5588, (u8*)&len, 1);
        
            write_dgus_vp(0x4650, (u8*)&ENGINEER_MODE_ENG, 13);
            len = 26;
            write_dgus_vp(0x5658, (u8*)&len, 1);


    write_dgus_vp(0x4060, (u8*)&TOP_TEMP_ENG, 8);
    len = 16;
    write_dgus_vp(0x5068, (u8*)&len, 1);

    write_dgus_vp(0x4090, (u8*)&BOT_TEMP_ENG, 8);
    len = 16;
    write_dgus_vp(0x5098, (u8*)&len, 1);

    write_dgus_vp(0x4120, (u8*)&PRESSURE_ENG, 8);
    len = 16;
    write_dgus_vp(0x5128, (u8*)&len, 1);

    write_dgus_vp(0x4150, (u8*)&DELAY_TIME_ENG, 5);
    len = 10;
    write_dgus_vp(0x5158, (u8*)&len, 1);

    write_dgus_vp(0x4190, (u8*)&TOP_HEATER_ENG, 10);
    len = 20;
    write_dgus_vp(0x5198, (u8*)&len, 1);

    write_dgus_vp(0x4200, (u8*)&OFF_TEXT, 3);
    len = 6;
    write_dgus_vp(0x5208, (u8*)&len, 1);

    write_dgus_vp(0x4210, (u8*)&BOT_HEATER_ENG, 10);
    len = 20;
    write_dgus_vp(0x5218, (u8*)&len, 1);

    write_dgus_vp(0x4220, (u8*)&OFF_TEXT, 3);
    len = 6;
    write_dgus_vp(0x5228, (u8*)&len, 1);

    write_dgus_vp(0x4230, (u8*)&TOP_FAN_ENG, 7);
    len = 14;
    write_dgus_vp(0x5238, (u8*)&len, 1);

    write_dgus_vp(0x4240, (u8*)&OFF_TEXT, 3);
    len = 6;
    write_dgus_vp(0x5248, (u8*)&len, 1);

    write_dgus_vp(0x4250, (u8*)&BOT_FAN_ENG, 7);
    len = 14;
    write_dgus_vp(0x5258, (u8*)&len, 1);

    write_dgus_vp(0x4260, (u8*)&OFF_TEXT, 3);
    len = 6;
    write_dgus_vp(0x5268, (u8*)&len, 1);

    write_dgus_vp(0x4270, (u8*)&COMPRESSOR_ENG, 10);
    len = 20;
    write_dgus_vp(0x5278, (u8*)&len, 1);

    write_dgus_vp(0x4280, (u8*)&OFF_TEXT, 3);
    len = 6;
    write_dgus_vp(0x5288, (u8*)&len, 1);

    write_dgus_vp(0x4290, (u8*)&SOLENOID_ENG, 8);
    len = 16;
    write_dgus_vp(0x5298, (u8*)&len, 1);

    write_dgus_vp(0x4300, (u8*)&OFF_TEXT, 3);
    len = 6;
    write_dgus_vp(0x5308, (u8*)&len, 1);

    write_dgus_vp(0x4320, (u8*)&DATA_ENG, 4);
    len = 8;
    write_dgus_vp(0x5328, (u8*)&len, 1);

    write_dgus_vp(0x4330, (u8*)&INIT_QUESTION_ENG, 11);
    len = 22;
    write_dgus_vp(0x5338, (u8*)&len, 1);


    write_dgus_vp(0x4340, (u8*)&NO_ENG, 2);
    len = 4;
    write_dgus_vp(0x5348, (u8*)&len, 1);

    write_dgus_vp(0x4350, (u8*)&YES_ENG, 3);
    len = 6;
    write_dgus_vp(0x5358, (u8*)&len, 1);

    write_dgus_vp(0x4370, (u8*)&ACTIVE_LOG_ENG, 10);
    len = 20;
    write_dgus_vp(0x5378, (u8*)&len, 1);

    write_dgus_vp(0x4380, (u8*)&ERROR_LOG_ENG, 9);
    len = 18;
    write_dgus_vp(0x5388, (u8*)&len, 1);

    //작업 시간
    write_dgus_vp(0x4410, (u8*)&RUN_TIME_ENG, 8);
    len = 16;
    write_dgus_vp(0x5418, (u8*)&len, 1);


    write_dgus_vp(0x4420, (u8*)&HEATING_TIME_ENG, 12);
    len = 24;
    write_dgus_vp(0x5428, (u8*)&len, 1);

    write_dgus_vp(0x4460, (u8*)&COMPRESSOR_ENG, 10);
    len = 20;
    write_dgus_vp(0x5468, (u8*)&len, 1);

    write_dgus_vp(0x4500, (u8*)&FAN_ENG, 3);
    len = 6;
    write_dgus_vp(0x5508, (u8*)&len, 1);

    //언어 변경
    // write_dgus_vp(0x4540, (u8*)&LANGUAGE_ENG, 8);
    // len = 16;
    // write_dgus_vp(0x5548, (u8*)&len, 1);

    write_dgus_vp(0x4550, (u8*)&ENGLISH_ENG, 7);
    len = 14;
    write_dgus_vp(0x5558, (u8*)&len, 1);
 
    write_dgus_vp(0x4560, (u8*)&CHINESE_ENG, 7);
    len = 14;
    write_dgus_vp(0x5568, (u8*)&len, 1);

    write_dgus_vp(0x4570, (u8*)&KOREAN_ENG, 6);
    len = 12;
    write_dgus_vp(0x5578, (u8*)&len, 1);

    //회사 소개
    write_dgus_vp(0x4650, (u8*)&YONGLI_KOREA_ENG, 12);
    len = 24;
    write_dgus_vp(0x5658, (u8*)&len, 1);

    write_dgus_vp(0x4660, (u8*)&GYEONGGI_DO_ENG, 11);
    len = 22;
    write_dgus_vp(0x5668, (u8*)&len, 1);

    write_dgus_vp(0x4670, (u8*)&PAJU_JORI_ENG, 17);
    len = 34;
    write_dgus_vp(0x5678, (u8*)&len, 1);

    write_dgus_vp(0x4680, (u8*)&DANGJAEBONG_RO_29_ENG, 17);
    len = 34;
    write_dgus_vp(0x5688, (u8*)&len, 1);

    write_dgus_vp(0x4690, (u8*)&TEL_CHN, 12);
    len = 24;
    write_dgus_vp(0x5698, (u8*)&len, 1);

    write_dgus_vp(0x4700, (u8*)&SENSOR_CAL_ENG, 18);
    len = 36;
    write_dgus_vp(0x5708, (u8*)&len, 1);

    write_dgus_vp(0x4720, (u8*)&PRESSURE_CAL_ENG, 20);
    len = 40;
    write_dgus_vp(0x5728, (u8*)&len, 1);
}

void admin_language_chn(void){
    u16 len = 0;
    admin_List_text_change();

    //유저 세팅
    write_dgus_vp(0x4060, (u8*)&TOP_TEMP_CHN, 4);
    len = 8;
    write_dgus_vp(0x5068, (u8*)&len, 1);

    write_dgus_vp(0x4090, (u8*)&BOTTOM_TEMP_CHN, 4);
    len = 8;
    write_dgus_vp(0x5098, (u8*)&len, 1);

    write_dgus_vp(0x4120, (u8*)&PRESSURE_CHN, 2);
    len = 4;
    write_dgus_vp(0x5128, (u8*)&len, 1);

    write_dgus_vp(0x4150, (u8*)&DELAY_TIME_CHN, 4);
    len = 8;
    write_dgus_vp(0x5158, (u8*)&len, 1);

    //IO 테스트
    // write_dgus_vp(0x4400, (u8*)&IO_TEST_CHN, 7);
    // len = 14;
    // write_dgus_vp(0x5408, (u8*)&len, 1);

    write_dgus_vp(0x4190, (u8*)&TOP_HEATER_CHN, 5);
    len = 10;
    write_dgus_vp(0x5198, (u8*)&len, 1);

    write_dgus_vp(0x4210, (u8*)&BOTTOM_HEATER_CHN, 5);
    len = 10;
    write_dgus_vp(0x5218, (u8*)&len, 1);

    write_dgus_vp(0x4230, (u8*)&TOP_FAN_CHN, 4);
    len = 8;
    write_dgus_vp(0x5238, (u8*)&len, 1);

    write_dgus_vp(0x4250, (u8*)&BOTTOM_FAN_CHN, 4);
    len = 8;
    write_dgus_vp(0x5258, (u8*)&len, 1);

    write_dgus_vp(0x4270, (u8*)&COMPRESSOR_CHN, 3);
    len = 6;
    write_dgus_vp(0x5278, (u8*)&len, 1);

    write_dgus_vp(0x4290, (u8*)&SOLENOID_CHN, 3);
    len = 6;
    write_dgus_vp(0x5298, (u8*)&len, 1);
    
    //공장 초기화
    // write_dgus_vp(0x4760, (u8*)&FACTORY_RESET_CHN, 5);
    // len = 10;
    // write_dgus_vp(0x5768, (u8*)&len, 1);

    write_dgus_vp(0x4320, (u8*)&DATA_CHN, 2);
    len = 4;
    write_dgus_vp(0x5328, (u8*)&len, 1);

    write_dgus_vp(0x4330, (u8*)&INIT_QUESTION_CHN, 5);
    len = 10;
    write_dgus_vp(0x5338, (u8*)&len, 1);

    write_dgus_vp(0x4340, (u8*)&NO_CHN, 2);
    len = 4;
    write_dgus_vp(0x5348, (u8*)&len, 1);

    write_dgus_vp(0x4350, (u8*)&YES_CHN, 2);
    len = 4;
    write_dgus_vp(0x5358, (u8*)&len, 1);

    //로그 & 에러
    // write_dgus_vp(0x4650, (u8*)&LOG_ERROR_CHN, 7);
    // len = 14;
    // write_dgus_vp(0x5658, (u8*)&len, 1);

    write_dgus_vp(0x4370, (u8*)&ACTIVE_LOG_CHN, 4);
    len = 8;
    write_dgus_vp(0x5378, (u8*)&len, 1);

    write_dgus_vp(0x4380, (u8*)&ERROR_LOG_CHN, 4);
    len = 8;
    write_dgus_vp(0x5388, (u8*)&len, 1);

    //작업 시간
    write_dgus_vp(0x4410, (u8*)&RUN_TIME_CHN, 4);
    len = 8;
    write_dgus_vp(0x5418, (u8*)&len, 1);

    write_dgus_vp(0x4420, (u8*)&ERROR_LOG_CHN, 2);
    len = 4;
    write_dgus_vp(0x5428, (u8*)&len, 1);

    write_dgus_vp(0x4460, (u8*)&ERROR_LOG_ENG, 9);
    len = 18;
    write_dgus_vp(0x5468, (u8*)&len, 1);

    write_dgus_vp(0x4500, (u8*)&ERROR_LOG_ENG, 9);
    len = 18;
    write_dgus_vp(0x5508, (u8*)&len, 1);

    // 언어
    write_dgus_vp(0x4550, (u8*)&ENGLISH_CHN, 2);
    len = 4;
    write_dgus_vp(0x5558, (u8*)&len, 1);

    write_dgus_vp(0x4560, (u8*)&CHINESE_CHN, 2);
    len = 4;
    write_dgus_vp(0x5568, (u8*)&len, 1);

    write_dgus_vp(0x4570, (u8*)&KOREAN_CHN, 2);
    len = 4;
    write_dgus_vp(0x5578, (u8*)&len, 1);

  //회사 소개
    write_dgus_vp(0x4650, (u8*)&YONGLI_KOREA_ENG, 12);
    len = 24;
    write_dgus_vp(0x5658, (u8*)&len, 1);

    write_dgus_vp(0x4660, (u8*)&GYEONGGI_DO_CHN, 3);
    len = 6;
    write_dgus_vp(0x5668, (u8*)&len, 1);

    write_dgus_vp(0x4670, (u8*)&PAJU_JORI_CHN, 7);
    len = 14;
    write_dgus_vp(0x5678, (u8*)&len, 1);

    write_dgus_vp(0x4680, (u8*)&ADDRESS_CHN, 7);
    len = 14;
    write_dgus_vp(0x5688, (u8*)&len, 1);

    write_dgus_vp(0x4690, (u8*)&TEL_CHN, 12);
    len = 24;
    write_dgus_vp(0x5698, (u8*)&len, 1);

    write_dgus_vp(0x4700, (u8*)&SENSOR_CAL_ENG, 5);
    len = 10;
    write_dgus_vp(0x5708, (u8*)&len, 1);

    write_dgus_vp(0x4720, (u8*)&PRESSURE_CAL_ENG, 4);
    len = 8;
    write_dgus_vp(0x5728, (u8*)&len, 1);
}

void admin_language_kor(void){
    u16 len = 0;
    admin_List_text_change();

    //유저 세팅
    write_dgus_vp(0x4060, (u8*)&TOP_TEMP_KOR, 5);
    len = 10;
    write_dgus_vp(0x5068, (u8*)&len, 1);

    write_dgus_vp(0x4090, (u8*)&BOTTOM_TEMP_KOR, 5);
    len = 10;
    write_dgus_vp(0x5098, (u8*)&len, 1);

    write_dgus_vp(0x4120, (u8*)&PRESSURE_KOR, 2);
    len = 4;
    write_dgus_vp(0x5128, (u8*)&len, 1);

    write_dgus_vp(0x4150, (u8*)&DELAY_TIME_KOR, 4);
    len = 8;
    write_dgus_vp(0x5158, (u8*)&len, 1);

    //IO 테스트
    write_dgus_vp(0x4190, (u8*)&TOP_HEATER_KOR, 5);
    len = 10;
    write_dgus_vp(0x5198, (u8*)&len, 1);

    write_dgus_vp(0x4210, (u8*)&BOTTOM_HEATER_KOR, 5);
    len = 10;
    write_dgus_vp(0x5218, (u8*)&len, 1);

    write_dgus_vp(0x4230, (u8*)&TOP_FAN_KOR, 4);
    len = 8;
    write_dgus_vp(0x5238, (u8*)&len, 1);

    write_dgus_vp(0x4250, (u8*)&BOTTOM_FAN_KOR, 4);
    len = 8;
    write_dgus_vp(0x5258, (u8*)&len, 1);

    write_dgus_vp(0x4270, (u8*)&COMPRESSOR_KOR, 4);
    len = 8;
    write_dgus_vp(0x5278, (u8*)&len, 1);

    write_dgus_vp(0x4290, (u8*)&SOLENOID_KOR, 5);
    len = 10;
    write_dgus_vp(0x5298, (u8*)&len, 1);
    
    //공장 초기화
    write_dgus_vp(0x4320, (u8*)&INIT_QUESTION1, 6);
    len = 12;
    write_dgus_vp(0x5328, (u8*)&len, 1);

    write_dgus_vp(0x4330, (u8*)&INIT_QUESTION2, 9);
    len = 18;
    write_dgus_vp(0x5338, (u8*)&len, 1);

    write_dgus_vp(0x4340, (u8*)&CANCEL_KOR, 2);
    len = 4;
    write_dgus_vp(0x5348, (u8*)&len, 1);

    write_dgus_vp(0x4350, (u8*)&OK_KOR, 2);
    len = 4;
    write_dgus_vp(0x5358, (u8*)&len, 1);

    //로그 & 에러
    // write_dgus_vp(0x4650, (u8*)&LOG_ERROR_KOR, 7);
    // len = 14;
    // write_dgus_vp(0x5658, (u8*)&len, 1);

    write_dgus_vp(0x4370, (u8*)&ACTIVE_LOG_KOR, 5);
    len = 10;
    write_dgus_vp(0x5378, (u8*)&len, 1);

    write_dgus_vp(0x4380, (u8*)&ERROR_LOG_KOR, 5);
    len = 10;
    write_dgus_vp(0x5388, (u8*)&len, 1);

    //작업 시간
    write_dgus_vp(0x4410, (u8*)&WORK_TIME_KOR, 5);
    len = 10;
    write_dgus_vp(0x5418, (u8*)&len, 1);

    write_dgus_vp(0x4420, (u8*)&HEATING_TIME_KOR, 4);
    len = 8;
    write_dgus_vp(0x5428, (u8*)&len, 1);

    write_dgus_vp(0x4460, (u8*)&COMPRESSOR_KOR, 1);
    len = 2;
    write_dgus_vp(0x5468, (u8*)&len, 1);

    write_dgus_vp(0x4500, (u8*)&FAN_KOR, 9);
    len = 18;
    write_dgus_vp(0x5508, (u8*)&len, 1);
    
    // 언어 변경
    write_dgus_vp(0x4550, (u8*)&ENGLISH_KOR, 2);
    len = 4;
    write_dgus_vp(0x5558, (u8*)&len, 1);
    write_dgus_vp(0x4560, (u8*)&CHINESE_KOR, 3);
    len = 6;
    write_dgus_vp(0x5568, (u8*)&len, 1);
    write_dgus_vp(0x4570, (u8*)&KOREAN_KOR, 3);
    write_dgus_vp(0x5578, (u8*)&len, 1);

     //회사 소개
    write_dgus_vp(0x4650, (u8*)&YONGLI_KOREA_KOR, 5);
    len = 10;
    write_dgus_vp(0x5658, (u8*)&len, 1);

    write_dgus_vp(0x4660, (u8*)&GYEONGGI_DO_KOR, 3);
    len = 6;
    write_dgus_vp(0x5668, (u8*)&len, 1);

    write_dgus_vp(0x4670, (u8*)&PAJU_JORI_EUP_KOR, 7);
    len = 14;
    write_dgus_vp(0x5678, (u8*)&len, 1);

    write_dgus_vp(0x4680, (u8*)&DANGJAEBONG_RO_29_KOR, 7);
    len = 14;
    write_dgus_vp(0x5688, (u8*)&len, 1);

    write_dgus_vp(0x4690, (u8*)&TEL_CHN, 12);
    len = 24;
    write_dgus_vp(0x5698, (u8*)&len, 1);

    write_dgus_vp(0x4700, (u8*)&SENSOR_CAL_ENG, 4);
    len = 8;
    write_dgus_vp(0x5708, (u8*)&len, 1);

    write_dgus_vp(0x4720, (u8*)&PRESSURE_CAL_ENG, 4);
    len = 8;
    write_dgus_vp(0x5728, (u8*)&len, 1);

    // //엔지니어 모드
    // write_dgus_vp(0x4660, (u8*)&ENGINEER_MODE_KOR, 7);
    // len = 14;
    // write_dgus_vp(0x5698, (u8*)&len, 1);

    // write_dgus_vp(0x4550, (u8*)&ENGLISH_CHN, 2);
    // write_dgus_vp(0x5558, (u8*)&len2, 1);
    // write_dgus_vp(0x4560, (u8*)&CHINESE_CHN, 2);
    // write_dgus_vp(0x5568, (u8*)&len2, 1);
    // write_dgus_vp(0x4570, (u8*)&KOREAN_CHN, 2);
    // write_dgus_vp(0x5578, (u8*)&len2, 1);

    // write_dgus_vp(0x5089, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5099, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5129, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5159, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5189, (u8*)&KOR_FONT, 1);

    // write_dgus_vp(0x5409, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5219, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5249, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5279, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5309, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5349, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5379, (u8*)&KOR_FONT, 1);

    // write_dgus_vp(0x5769, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5779, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5789, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5799, (u8*)&KOR_FONT, 1);
    // write_dgus_vp(0x5809, (u8*)&KOR_FONT, 1);
}

//어드민 엔지니어 모드
void EngineermodTextColor(){
    if(EngineerSP == 0){
        SetTextColorYellow(0x5593);
        SetTextColorWhite(0x5603);
        SetTextColorWhite(0x5613);
        SetTextColorWhite(0x5623);
        SetTextColorWhite(0x5633);
        SetTextColorWhite(0x5643);
    }else if(EngineerSP == 1){
        SetTextColorWhite(0x5593);
        SetTextColorYellow(0x5603);
        SetTextColorWhite(0x5613);
        SetTextColorWhite(0x5623);
        SetTextColorWhite(0x5633);
        SetTextColorWhite(0x5643);
    }else if(EngineerSP == 2){
        SetTextColorWhite(0x5593);
        SetTextColorWhite(0x5603);
        SetTextColorYellow(0x5613);
        SetTextColorWhite(0x5623);
        SetTextColorWhite(0x5633);
        SetTextColorWhite(0x5643);
    }else if(EngineerSP == 3){
        SetTextColorWhite(0x5593);
        SetTextColorWhite(0x5603);
        SetTextColorWhite(0x5613);
        SetTextColorYellow(0x5623);
        SetTextColorWhite(0x5633);
        SetTextColorWhite(0x5643);
    }else if(EngineerSP == 4){
        SetTextColorWhite(0x5593);
        SetTextColorWhite(0x5603);
        SetTextColorWhite(0x5613);
        SetTextColorWhite(0x5623);
        SetTextColorYellow(0x5633);
        SetTextColorWhite(0x5643);
    }else if(EngineerSP == 5){
        SetTextColorWhite(0x5593);
        SetTextColorWhite(0x5603);
        SetTextColorWhite(0x5613);
        SetTextColorWhite(0x5623);
        SetTextColorWhite(0x5633);
        SetTextColorYellow(0x5643);
    }
}

void EngineermodWork(u16 state){
    switch (state)
    {
    case 1:{
        switch (EngineerSP){
                    case 0:{
                        if(EngineerPw1 == 0){
                            EngineerPw1 = 9;
                        }else{
                            EngineerPw1--;
                        }
                        write_dgus_vp(0x4590, (u8*)&EngineerPw1 , 1);
                    }break;
                    case 1:{
                         if(EngineerPw2 == 0){
                            EngineerPw2 = 9;
                        }else{
                            EngineerPw2++;
                        }
                        write_dgus_vp(0x4600, (u8*)&EngineerPw2 , 1);
                    }break;
                    case 2:{
                         if(EngineerPw3 == 0){
                            EngineerPw3 = 9;
                        }else{
                            EngineerPw3++;
                        }
                        write_dgus_vp(0x4610, (u8*)&EngineerPw3 , 1);
                    }break;
                    case 3:{
                         if(EngineerPw4 == 0){
                            EngineerPw4 = 9;
                        }else{
                            EngineerPw4++;
                        }
                        write_dgus_vp(0x4620, (u8*)&EngineerPw4 , 1);
                    }break;
                    case 4:{
                         if(EngineerPw5 == 0){
                            EngineerPw5 = 9;
                        }else{
                            EngineerPw5++;
                        }
                        write_dgus_vp(0x4630, (u8*)&EngineerPw5 , 1);
                    }break;
                    case 5:{
                        if(EngineerPw6 == 0){
                            EngineerPw6 = 9;
                        }else{
                            EngineerPw6++;
                        }
                        write_dgus_vp(0x4640, (u8*)&EngineerPw6 , 1);
                    }break;
                    }
                    EngineermodTextColor();
    }break;

    case 2:{
        switch (EngineerSP){
                    case 0:{
                        if(EngineerPw1==9){
                            EngineerPw1 = 0;
                        }else{
                            EngineerPw1++;
                        }
                        write_dgus_vp(0x4590, (u8*)&EngineerPw1 , 1);
                    }break;
                    case 1:{
                         if(EngineerPw2==9){
                            EngineerPw2 = 0;
                        }else{
                            EngineerPw2++;
                        }
                        write_dgus_vp(0x4600, (u8*)&EngineerPw2 , 1);
                    }break;
                    case 2:{
                         if(EngineerPw3==9){
                            EngineerPw3 = 0;
                        }else{
                            EngineerPw3++;
                        }
                        write_dgus_vp(0x4610, (u8*)&EngineerPw3 , 1);
                    }break;
                    case 3:{
                         if(EngineerPw4==9){
                            EngineerPw4 = 0;
                        }else{
                            EngineerPw4++;
                        }
                        write_dgus_vp(0x4620, (u8*)&EngineerPw4 , 1);
                    }break;
                    case 4:{
                         if(EngineerPw5==9){
                            EngineerPw5 = 0;
                        }else{
                            EngineerPw5++;
                        }
                        write_dgus_vp(0x4630, (u8*)&EngineerPw5 , 1);
                    }break;
                    case 5:{
                        if(EngineerPw6==9){
                            EngineerPw6 = 0;
                        }else{
                            EngineerPw6++;
                        }
                        write_dgus_vp(0x4640, (u8*)&EngineerPw6 , 1);
                    }break;
                    }
                    EngineermodTextColor();
    }break;

    case 3:{
        if(EngineerSP == 0||
           EngineerSP == 1||
           EngineerSP == 2||
           EngineerSP == 3||
           EngineerSP == 4){
            EngineerSP ++;
            EngineermodTextColor();
        }else if(EngineerSP == 5){
            u16 inputpw = 0;
            EngineerPw1 = EngineerPw1*100000;
            EngineerPw2 = EngineerPw2*10000;
            EngineerPw3 = EngineerPw3*1000;
            EngineerPw4 = EngineerPw4*100;
            EngineerPw5 = EngineerPw5*10;
            EngineerPw6 = EngineerPw6*1;
            inputpw = EngineerPw1+EngineerPw2+EngineerPw3+EngineerPw4+EngineerPw5+EngineerPw6;
            if(pw == inputpw){
                EngineerPw1 = 0;
                EngineerPw2 = 0;
                EngineerPw3 = 0;
                EngineerPw4 = 0;
                EngineerPw5 = 0;
                EngineerPw6 = 0;
                write_dgus_vp(0x4590, (u8*)&EngineerPw1 , 1);
                write_dgus_vp(0x4600, (u8*)&EngineerPw2 , 1);
                write_dgus_vp(0x4610, (u8*)&EngineerPw3 , 1);
                write_dgus_vp(0x4620, (u8*)&EngineerPw4 , 1);
                write_dgus_vp(0x4630, (u8*)&EngineerPw5 , 1);
                write_dgus_vp(0x4640, (u8*)&EngineerPw6 , 1);
                page_number = 38;
                Page_Change_Handler(page_number);
            }else{
                EngineerPw1 = 0;
                EngineerPw2 = 0;
                EngineerPw3 = 0;
                EngineerPw4 = 0;
                EngineerPw5 = 0;
                EngineerPw6 = 0;
                write_dgus_vp(0x4590, (u8*)&EngineerPw1 , 1);
                write_dgus_vp(0x4600, (u8*)&EngineerPw2 , 1);
                write_dgus_vp(0x4610, (u8*)&EngineerPw3 , 1);
                write_dgus_vp(0x4620, (u8*)&EngineerPw4 , 1);
                write_dgus_vp(0x4630, (u8*)&EngineerPw5 , 1);
                write_dgus_vp(0x4640, (u8*)&EngineerPw6 , 1);
                EngineerSP = 0;
                SetTextColorRed(0x5593);
                SetTextColorRed(0x5603);
                SetTextColorRed(0x5613);
                SetTextColorRed(0x5623);
                SetTextColorRed(0x5633);
                SetTextColorRed(0x5643);
            }
        }
    }break;
    
    default:
        break;
    }
}

static u16 EngineermodS_flag = 0;
static u16 EngineermodS_Select_flag = 0;
static u16 sensor_result = 100;
static u16 press_result = 100;

void EngineermodSText(){
    if(EngineermodS_flag == 0){
        if(EngineermodS_Select_flag == 0){
            SetTextColorYellow(0x5633);
            SetTextColorWhite(0x5713);
            SetTextColorWhite(0x5723);
            SetTextColorWhite(0x5733);
        }else{
            SetTextColorYellow(0x5633);
            SetTextColorYellow(0x5713);
            SetTextColorWhite(0x5723);
            SetTextColorWhite(0x5733);
        }
    }else{
        if(EngineermodS_Select_flag == 0){
            SetTextColorWhite(0x5633);
            SetTextColorWhite(0x5713);
            SetTextColorYellow(0x5723);
            SetTextColorWhite(0x5733);
        }else{
            SetTextColorWhite(0x5633);
            SetTextColorWhite(0x5713);
            SetTextColorYellow(0x5723);
            SetTextColorYellow(0x5733);
        }
    }

}


void EngineermodSWork(u16 state){
    if(state == 1 || state == 2){
        if(EngineermodS_Select_flag == 0){
            if(EngineermodS_flag == 0){
                EngineermodS_flag = 1;
            }else{
                EngineermodS_flag = 0;
            }
        }else{
            if(EngineermodS_flag == 0){
                if(state == 1 && sensor_result<120 && sensor_result >80){
                    sensor_result--;
                }else if(state == 2 && sensor_result<120 && sensor_result >80){
                    sensor_result++;
                }
            }else{
                if(state == 1 && press_result<120 && press_result >80){
                    press_result--;
                }else if(state == 2 && press_result<120 && press_result >80){
                    press_result++;
                }
            }
        }
        write_dgus_vp(0x4700, (u8*)& sensor_result, 1);
        write_dgus_vp(0x4730, (u8*)& press_result, 1);
    }else if(state == 3){
        if(EngineermodS_Select_flag == 0){
            EngineermodS_Select_flag = 1;
        }else{
            EngineermodS_Select_flag = 0;
        }
    }
    EngineermodSText();
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
#ifndef __ADMIN_H__
#define __ADMIN_H__

// typedef unsigned short u16;

void admin_User_Setting_Function(u16 i);
void admin_List(u16 i);
void adminUserSettingEdit(void);
void adminUserSettingText(u16 state);
void adminIoTestText(u16 state);
void adminIoTestWork(void);
void adminfactoryResetText(u16 state);
void adminLogErrorText(u16 state);
void adminLanguageText(u16 state);
void admin_language_eng(void);
void admin_language_chn(void);
void admin_language_kor(void);
void EngineermodTextChange(void);
void admin_page_change(void);
void EngineermodWork(u16 state);
void admininit(void);
void Page_Change_UI(u8 i);
void admin_List_text_change(void);
void EngineermodSWork(u16 state);

extern u8 LogErrorSP;
extern u8 adminSP;
extern u8 EngineerSP;
extern u16 EngineerPw1;
extern u16 EngineerPw2;
extern u16 EngineerPw3;
extern u16 EngineerPw4;
extern u16 EngineerPw5;
extern u16 EngineerPw6;
extern u16 toptempmin;
extern u16 toptempmax;
extern u16 bottempmin;
extern u16 bottempmax;
extern u16 pressmin;
extern u16 pressmax;
extern u16 delaymin;
extern u16 delaymax;
extern u16 sensor_result;
extern u16 press_result;
extern u16 temp_H;
extern u16 temp_M;
extern u16 temp_S;

extern u16 pump_H;
extern u16 pump_M;
extern u16 pump_S;

extern u16 fan_H;
extern u16 fan_M;
extern u16 fan_S;
extern u8 usersettingSP;
extern u8 usersettingSelect_plag;
extern u8 usersettingEditSP;

extern u8 factoryReset_plag;
extern u8 factoryResetSP;

extern u8 LanguageSP;

extern u8 IOtestSP;
extern u8 IOtestSelect_plag;

extern u16 EngineermodS_flag;
extern u16 EngineermodS_Select_flag;
#endif
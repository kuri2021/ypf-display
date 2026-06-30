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
void admininit(void);
void Page_Change_UI(u8 i);

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

#endif
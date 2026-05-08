#include "hmi.h"

HmiStatus_t g_hmiStat;
HmiSet_t    g_hmiSet;

#ifndef HMI_USE_EXTERNAL_AD
s16 ad_top_tep(void)    { return g_hmiStat.t_top_x10; }
s16 ad_bottom_tep(void) { return g_hmiStat.t_bot_x10; }
#endif

/* 200ms 주기 서비스: 0x5000~ 읽고 → 0x2102/0x2103 갱신 */
void HMI_Service_200ms(void)
{
    HMI_ReadStatus();          // TT/TB 등 최신화 (12바이트)
    UI_UpdateTempsFromArduino();
}

void HMI_ReadStatus(void)
{
    u16 buf[7]; /* TT,TB,P,NTT,NTB,ST,FLAGS */
    /* 0x5000 ~ 0x500A : 6워드 */
    read_dgus_vp(VP_TT, (u8*)buf, 12);
    g_hmiStat.t_top_x10   = (s16)buf[0];
    g_hmiStat.t_bot_x10   = (s16)buf[1];
    g_hmiStat.p_x100      = (u16)buf[2];
    g_hmiStat.ntc_top_x10 = (s16)buf[3];
    g_hmiStat.ntc_bot_x10 = (s16)buf[4];
    g_hmiStat.stage       = (u16)buf[5];

    /* 0x500C : FLAGS */
    read_dgus_vp(VP_FLAGS, (u8*)&buf[6], 2);
    g_hmiStat.flags = (u16)buf[6];
}

void HMI_ReadSettings(void)
{
    u16 buf[3]; /* SET_T, SET_P, SET_H */
    read_dgus_vp(VP_SET_T, (u8*)buf, 6);
    g_hmiSet.set_t_x10  = buf[0];
    g_hmiSet.set_p_x100 = buf[1];
    g_hmiSet.hold_sec   = buf[2];
}

void HMI_WriteSettings(u16 setT_x10, u16 setP_x100, u16 hold_s)
{
    u16 buf[3];
    buf[0] = setT_x10;
    buf[1] = setP_x100;
    buf[2] = hold_s;
    write_dgus_vp(VP_SET_T, (u8*)buf, 6);
}

/* 상/하판 온도를 0x2102/0x2103에 써서 UI에 표시 */
void UI_UpdateTempsFromArduino(void)
{
    static s16 last_top = 0x7FFF;
    static s16 last_bot = 0x7FFF;

    s16 top_x10 = ad_top_tep();      /* ℃×10 */
    s16 bot_x10 = ad_bottom_tep();   /* ℃×10 */

    if (top_x10 != last_top) {
        write_dgus_vp(0x2102, (u8*)&top_x10, 2);  // ← len=2 (u16 1개)
        last_top = top_x10;
    }
    if (bot_x10 != last_bot) {
        write_dgus_vp(0x2103, (u8*)&bot_x10, 2);  // ← len=2 (u16 1개)
        last_bot = bot_x10;
    }
}
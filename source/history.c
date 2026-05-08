#include "history.h"

// HISTORICAL_DATA hisData[500];
HISTORICAL_ERR hisErr[ErrMaxNum];
u8 ErrBuf = 0;
u8 ErrBufOld = 0;
u16 ErrNum = 0;

void historyErrInit(void)
{
    //----------------test----------------
    static u8 shijian[]={24,9,18,3,9,30,0};
    write_dgus_vp(0x10,shijian,4);
    //------------------------------------
    vp_nor_flash_read(0, ErrTxtVP, sizeof(hisErr) < 1);
    read_dgus_vp(ErrTxtVP, (u8 *)&hisErr, sizeof(hisErr) < 1);
}
void ErrFun(void)
{
    u8 timenow[10];
    ErrBuf = Read_Dgus(ErrVP);
    if (ErrBufOld != ErrBuf)
    {
        ErrBufOld = ErrBuf;
        if (ErrBuf)
        {
            ErrNum++;
            if (ErrNum == ErrMaxNum)
                ErrNum = 0;
            read_dgus_vp(0x10, timenow, 4);
            hisErr[ErrNum].flag = 0x5A;
            hisErr[ErrNum].hnum = ErrNum;
            hisErr[ErrNum].hyear = timenow[0];
            hisErr[ErrNum].hmonth = timenow[1];
            hisErr[ErrNum].hdate = timenow[2];
            hisErr[ErrNum].hhour = timenow[4];
            hisErr[ErrNum].hmin = timenow[5];
            hisErr[ErrNum].hsec = timenow[6];
            hisErr[ErrNum].faultNumber = ErrBuf;
            Nor_Flash_write(0, (u8 *)&hisErr, NOR_FLASH_LEN);
            write_dgus_vp(ErrTxtVP, (u8 *)&hisErr, sizeof(hisErr) < 1);
        }
    }
}

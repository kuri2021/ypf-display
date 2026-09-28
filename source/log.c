#include "ui.h"
#include "sys.h"
#include <string.h>
#include "hmi.h"
#include "admin.h"

#define LOG_MAX 3

#define VP_LOG 0x8500

#define LOG_TOP_TEMP       1
#define LOG_TOP_COOL       2
#define LOG_BOTTOM_TEMP    3
#define LOG_BOTTOM_COOL    4
#define LOG_DELAY          5
#define LOG_PRESSURE       6

LogData logs[LOG_MAX];

u8 logCount = 0;

const u16 SP_ENG_FONTID = 0x000F;
const u16 SP_ENG_FONTSIZE = 0x1A19;
const u16 SP_KOR_FONTID = 0x0010;
const u16 SP_KOR_FONTSIZE = 0x1F1F;


struct LogData
{
    u16 type;
    u16 value;
};

void addLog(u16 type, u16 value)
{
    if(logCount < LOG_MAX)
    {
        logs[logCount].type = type;
        logs[logCount].value = value;

        logCount++;
    }
    else
    {
        for(int i = 0; i < LOG_MAX - 1; i++)
        {
            logs[i] = logs[i + 1];
        }

        // 마지막 자리에 새 로그 저장
        logs[LOG_MAX - 1].type = type;
        logs[LOG_MAX - 1].value = value;
    }

    updateLogsToDGUS();
}

void updateLogsToDGUS()
{
    u16 len = 0;
    for(u8 i = 0; i < LOG_MAX; i++){
        u16 vpType  = VP_LOG + (i * 4);
        u16 vpValue = VP_LOG + (i * 4) + 2;

        if(i < logCount)
        {
            write_dgus_vp(vpType,(u8*)&logs[i].type,1);
            write_dgus_vp(vpValue,(u8*)&logs[i].value,1);
            if(admin_language == 0){
                switch(logs[i].type){
                case 1:{
                    if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&TOP_TEMP_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4770, (u8*)&TOP_TEMP_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5778, (u8*)&len, 1);
                        write_dgus_vp(0x5779, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x577A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4780, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4800, (u8*)&TOP_TEMP_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5808, (u8*)&len, 1);
                        write_dgus_vp(0x5809, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x580A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4810, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 2:{
                    if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&TOP_COOL_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4770, (u8*)&TOP_COOL_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5778, (u8*)&len, 1);
                        write_dgus_vp(0x5779, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x577A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4780, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4800, (u8*)&TOP_COOL_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5808, (u8*)&len, 1);
                        write_dgus_vp(0x5809, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x580A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4810, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 3:{
                     if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&BOT_TEMP_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4770, (u8*)&BOT_TEMP_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5778, (u8*)&len, 1);
                        write_dgus_vp(0x5779, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x577A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4780, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4800, (u8*)&BOT_TEMP_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5808, (u8*)&len, 1);
                        write_dgus_vp(0x5809, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x580A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4810, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 4:{
                    if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&BOT_COOL_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4770, (u8*)&BOT_COOL_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5778, (u8*)&len, 1);
                        write_dgus_vp(0x5779, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x577A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4780, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4800, (u8*)&BOT_COOL_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5808, (u8*)&len, 1);
                        write_dgus_vp(0x5809, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x580A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4810, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 5:{
                    if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&DELAY_ENG, 9);
                        len = 18;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4770, (u8*)&DELAY_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5778, (u8*)&len, 1);
                        write_dgus_vp(0x5779, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x577A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4780, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4800, (u8*)&DELAY_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5808, (u8*)&len, 1);
                        write_dgus_vp(0x5809, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x580A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4810, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 6{
                     if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&PRESS_ENG, 9);
                        len = 18;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4770, (u8*)&PRESS_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5778, (u8*)&len, 1);
                        write_dgus_vp(0x5779, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x577A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4780, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4800, (u8*)&PRESS_ENG, 10);
                        len = 20;
                        write_dgus_vp(0x5808, (u8*)&len, 1);
                        write_dgus_vp(0x5809, (u8*)&SP_ENG_FONTID, 1);
                        write_dgus_vp(0x580A, (u8*)&SP_ENG_FONTSIZE, 1);

                        write_dgus_vp(0x4810, (u8*)&logs[i].value, 1);
                    }
                }break;
            }
            }else{
                switch(logs[i].type){
                case 1:{
                      if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&TOP_TEMP_KOR, 8);
                        len = 16;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4740, (u8*)&TOP_TEMP_KOR, 8);
                        len = 16;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4740, (u8*)&TOP_TEMP_KOR, 8);
                        len = 16;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 2:{
                     if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&TOP_COOL_KOR, 7);
                        len = 14;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4740, (u8*)&TOP_COOL_KOR, 7);
                        len = 14;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4740, (u8*)&TOP_COOL_KOR, 7);
                        len = 14;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 3:{
                     if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&BOT_TEMP_KOR, 8);
                        len = 16;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4740, (u8*)&BOT_TEMP_KOR, 8);
                        len = 16;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4740, (u8*)&BOT_TEMP_KOR, 8);
                        len = 16;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 4:{
                     if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&BOT_COOL_KOR, 7);
                        len = 14;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4740, (u8*)&BOT_COOL_KOR, );
                        len = 14;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4740, (u8*)&BOT_COOL_KOR, 7);
                        len = 14;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 5:{
                     if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&DELAY_KOR, 5);
                        len = 10;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4740, (u8*)&DELAY_KOR, 5);
                        len = 10;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4740, (u8*)&DELAY_KOR, 5);
                        len = 10;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }
                }break;
                case 6{
                     if(i == 0){
                        write_dgus_vp(0x4740, (u8*)&PRESS_KOR, 5);
                        len = 10;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 1){
                        write_dgus_vp(0x4740, (u8*)&PRESS_KOR, 5);
                        len = 10;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }else if(i == 2){
                        write_dgus_vp(0x4740, (u8*)&PRESS_KOR, 5);
                        len = 10;
                        write_dgus_vp(0x5748, (u8*)&len, 1);
                        write_dgus_vp(0x5749, (u8*)&SP_KOR_FONTID, 1);
                        write_dgus_vp(0x574A, (u8*)&SP_KOR_FONTSIZE, 1);

                        write_dgus_vp(0x4750, (u8*)&logs[i].value, 1);
                    }
                }break;
            }
            }
            
        }else{
            u16 zero = 0;
            write_dgus_vp(vpType,(u8*)&zero,1);
            write_dgus_vp(vpValue,(u8*)&zero,1);
        }
    }

}

void clearLogs()
{
    logCount = 0;

    for(u8 i = 0; i < LOG_MAX; i++)
    {
        logs[i].type = 0;
        logs[i].value = 0;
    }

    updateLogsToDGUS();
}

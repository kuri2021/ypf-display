#include "dwin8283.h"
#include "uart.h"
// #include "canbus.h"

#define NUM 2
#define DWIN_UART UART2

#define MAX_BYTE_LEN 512
#define MIN_CHECK_LEN 6
#define FRAME_HEAD 0x5AA5
#define WRITE_VP 0x82
#define READ_VP  0x83
// #define ENABLE_CRC
// #define AUTO_UPLOAD

QUENE DwinFrame[NUM];
u8 SendTmp[MAX_BYTE_LEN];

// === 업그레이드 트리거 플래그(추가) ===
volatile u8 g_need_upgrade = 0;

// CANBUS8283 DataBuf;

void Pro8283Init(void)
{
    // u8 CanReg[4]={0x0F,0x40,0x72,0x00};

    UartInit(DWIN_UART, 19200); // use 8283 protocol on UART2
    UartInit(UART4,     19200); // use 8283 protocol on UART4
    // CanBusInit(CanReg);

    DwinFrame[0].pQuene     = Uart_Struct[DWIN_UART].rx_buf;
    DwinFrame[0].QueneHead  = &Uart_Struct[DWIN_UART].rx_head;
    DwinFrame[0].QueneTail  = &Uart_Struct[DWIN_UART].rx_tail;
    DwinFrame[0].QueneSize  = SERIAL_SIZE;

    DwinFrame[1].pQuene     = Uart_Struct[UART4].rx_buf;
    DwinFrame[1].QueneHead  = &Uart_Struct[UART4].rx_head;
    DwinFrame[1].QueneTail  = &Uart_Struct[UART4].rx_tail;
    DwinFrame[1].QueneSize  = SERIAL_SIZE;

    /* DwinFrame[1].pQuene    = DataBuf.Busbuf;
    DwinFrame[1].QueneHead   = &DataBuf.Can8283RxHead;
    DwinFrame[1].QueneTail   = &DataBuf.Can8283RxTail;
    DwinFrame[1].QueneSize   = 256; */
}

#ifdef ENABLE_CRC
u16 Calculate_CRC16_8283(u8 *updata, u16 len)
{
    u16 Reg_CRC = 0xffff;
    u16 i;
    u8  j;
    u16 crctmp;

    for (i = 0; i < len; i++)
    {
        Reg_CRC ^= *updata++;
        for (j = 0; j < 8; j++)
        {
            if (Reg_CRC & 0x0001)
            {
                Reg_CRC = (Reg_CRC >> 1) ^ 0XA001;
            }
            else
            {
                Reg_CRC >>= 1;
            }
        }
    }
    crctmp  = Reg_CRC << 8;
    crctmp |= (Reg_CRC >> 8);
    return crctmp;
}
#endif

#ifdef AUTO_UPLOAD
u8 AutoUploadValue(void)
{
    u16 address;
    u8  len;

    read_dgus_vp(0x0f00, SendTmp, 2);
    if (SendTmp[0] == 0x5a)
    {
        SendTmp[0] = 0;
        write_dgus_vp(0x0f00, SendTmp, 2);

        len = SendTmp[3];
        if (len >= 200) len = 100;
        address = *(u16 *)&SendTmp[1];

        SendTmp[0] = 0x5a;
        SendTmp[1] = 0xa5;
        SendTmp[2] = (len << 1) + 4;
        SendTmp[3] = 0x83;
        SendTmp[4] = (u8)(address >> 8);
        SendTmp[5] = (u8)(address);
        SendTmp[6] = len;

        read_dgus_vp(address, &SendTmp[7], len);

#ifdef ENABLE_CRC
        *(u16 *)&SendTmp[SendTmp[2] + 3] = Calculate_CRC16_8283(&SendTmp[3], SendTmp[2]);
        SendTmp[2] += 2;
#endif
        return DO_SEND_DATA;
    }
    return NOT_SEND_DATA;
}
#endif

u8 Pro8283RxTreat(QUENE *pBuf)
{
    u16 len, len1, i;
    u16 tmp16, headtmp, tailtmp;

    EA = 0;
    headtmp = *pBuf->QueneHead;
    EA = 1;

    if (*pBuf->QueneTail != headtmp)
    {
        if (headtmp < *pBuf->QueneTail)
        {
            len = (headtmp + pBuf->QueneSize) - *pBuf->QueneTail;
        }
        else
        {
            len = headtmp - *pBuf->QueneTail;
        }

        if (len < MIN_CHECK_LEN)
            return NOT_SEND_DATA;

        tmp16 = pBuf->pQuene[*pBuf->QueneTail];
        tmp16 <<= 8;
        tailtmp = *pBuf->QueneTail + 1;
        if (tailtmp >= pBuf->QueneSize)
        {
            tailtmp = 0;
        }
        tmp16 |= pBuf->pQuene[tailtmp];

        if (FRAME_HEAD == tmp16)
        {
            tailtmp = *pBuf->QueneTail + 3;
            if (tailtmp >= pBuf->QueneSize)
            {
                tailtmp -= pBuf->QueneSize;
            }

            /* ===================== WRITE_VP (0x82) ===================== */
            if (pBuf->pQuene[tailtmp] == WRITE_VP)
            {
                tailtmp = *pBuf->QueneTail + 2;
                if (tailtmp >= pBuf->QueneSize)
                {
                    tailtmp -= pBuf->QueneSize;
                }
                len1 = pBuf->pQuene[tailtmp] + 3;

                if (len >= len1)
                {
                    for (i = 0; i < len1; i++)
                    {
                        SendTmp[i] = pBuf->pQuene[*pBuf->QueneTail];
                        *pBuf->QueneTail += 1;
                        if (*pBuf->QueneTail >= pBuf->QueneSize)
                        {
                            *pBuf->QueneTail = 0;
                        }
                    }

#ifdef ENABLE_CRC
                    if (Calculate_CRC16_8283(&SendTmp[3], SendTmp[2]) == 0)
                    {
                        SendTmp[2] -= 2;
#endif
                        /* --- 여기서 업그레이드 트리거 처리 (문서의 deal_82_cmd 기반) --- */
                        {
                            u16 vp  = *(u16 *)&SendTmp[4]; // VP high/low
                            // 길이: SendTmp[2], 첫 데이터 바이트: SendTmp[6], 둘째: SendTmp[7] ...

                            if ( (SendTmp[4] == 0x00) && (SendTmp[5] == 0x06) )
                            {
                                // arr[4]==0 && arr[5]==6  -> upgrade c code
                                // 문서 예제: arr[7] = 0xA5;  // 업그레이드 플래그 값
                                // 첫 word(2바이트) 중 낮은 바이트를 0xA5로 강제
                                SendTmp[7] = 0xA5;

                                // 펌웨어 쪽에서도 업그레이드 모드 진입 준비
                                g_need_upgrade = 1;
                            }

                            write_dgus_vp(vp, &SendTmp[6], (SendTmp[2] - 3) >> 1);
                        }

                        SendTmp[2] = 3;
                        *(u16 *)&SendTmp[4] = 0x4f4b;   // 'OK'

#ifdef ENABLE_CRC
                        *(u16 *)&SendTmp[6] = Calculate_CRC16_8283(&SendTmp[3], SendTmp[2]);
                        SendTmp[2] += 2;
#endif
                        return DO_SEND_DATA;
#ifdef ENABLE_CRC
                    }
#endif
                }
            }
            /* ===================== READ_VP (0x83) ===================== */
            else if (pBuf->pQuene[tailtmp] == READ_VP)
            {
                tailtmp = *pBuf->QueneTail + 2;
                if (tailtmp >= pBuf->QueneSize)
                {
                    tailtmp -= pBuf->QueneSize;
                }
                len1 = pBuf->pQuene[tailtmp] + 3;
                if (len >= len1)
                {
                    for (i = 0; i < len1; i++)
                    {
                        SendTmp[i] = pBuf->pQuene[*pBuf->QueneTail];
                        *pBuf->QueneTail += 1;
                        if (*pBuf->QueneTail >= pBuf->QueneSize)
                        {
                            *pBuf->QueneTail = 0;
                        }
                    }
#ifdef ENABLE_CRC
                    if (Calculate_CRC16_8283(&SendTmp[3], SendTmp[2]) == 0)
                    {
#endif
                        read_dgus_vp(*(u16 *)&SendTmp[4], &SendTmp[7], SendTmp[6]);
                        SendTmp[2] = (SendTmp[6] << 1) + 4;
#ifdef ENABLE_CRC
                        *(u16 *)&SendTmp[SendTmp[2] + 3] = Calculate_CRC16_8283(&SendTmp[3], SendTmp[2]);
                        SendTmp[2] += 2;
#endif
                        return DO_SEND_DATA;
#ifdef ENABLE_CRC
                    }
#endif
                }
            }
            else
            {
                *pBuf->QueneTail += 1;
                if (*pBuf->QueneTail >= pBuf->QueneSize)
                {
                    *pBuf->QueneTail = 0;
                }
            }
        }
        else
        {
            *pBuf->QueneTail += 1;
            if (*pBuf->QueneTail >= pBuf->QueneSize)
            {
                *pBuf->QueneTail = 0;
            }
        }
    }

    return NOT_SEND_DATA;
}

#define FRAME_ID 0X23

void Pro8283Deal(void)
{
#ifdef AUTO_UPLOAD
    if (DO_SEND_DATA == AutoUploadValue())
    {
        Uart_Send_Data(DWIN_UART, SendTmp[2] + 3, SendTmp);
        /* CanTx(0x23,0,SendTmp[2]+3,SendTmp); */
    }
#endif

    if (DO_SEND_DATA == Pro8283RxTreat(&DwinFrame[0]))
    {
        Uart_Send_Data(DWIN_UART, SendTmp[2] + 3, SendTmp);
    }

    if (DO_SEND_DATA == Pro8283RxTreat(&DwinFrame[1]))
    {
        Uart_Send_Data(UART4, SendTmp[2] + 3, SendTmp);
    }
}

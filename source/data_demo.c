#include "data_demo.h"
#include "sys.h"   // write_dgus_vp, read_dgus_vp 선언 포함

// 보드 → 디스플레이 : 현재 상태 전달 예제
void Demo_SendBoardToDisplay(void)
{
    u16 val16;

    // 현재 상 히터 온도 = 185℃
    val16 = 185;
    write_dgus_vp(0x8000, (u8*)&val16, 2);

    // 현재 하 히터 온도 = 172℃
    val16 = 172;
    write_dgus_vp(0x8002, (u8*)&val16, 2);

    // 현재 압력 = 2.5 bar (엑셀 기준 0.1bar 단위 → 25)
    val16 = 25;
    write_dgus_vp(0x8004, (u8*)&val16, 2);

    // 상태 코드 = 2 (예: HEATUP)
    val16 = 2;
    write_dgus_vp(0x800A, (u8*)&val16, 2);

    // 플래그 예시 (비트 조합 값)
    val16 = 0x0003; // bit0=RUN, bit1=HEATER
    write_dgus_vp(0x800C, (u8*)&val16, 2);

    // 에러 코드/인자
    val16 = 101; // 예: 온도센서 오류
    write_dgus_vp(0x8010, (u8*)&val16, 2);

    val16 = 34; // 예: 채널 번호
    write_dgus_vp(0x8012, (u8*)&val16, 2);
}


// 디스플레이 → 보드 : 설정값 읽기 예제
void Demo_ReadDisplayToBoard(void)
{
    u16 buf[8];

    // 목표 압력 읽기 (0x8100, 단위 0.1bar)
    read_dgus_vp(0x8100, (u8*)buf, 2);
    u16 targetP = buf[0];

    // 유지 시간 읽기 (0x8102, 단위 초)
    read_dgus_vp(0x8102, (u8*)buf, 2);
    u16 holdTimeSec = buf[0];

    // 상 히터 목표온도 (0x8110)
    read_dgus_vp(0x8110, (u8*)buf, 2);
    u16 setTopTemp = buf[0];

    // 하 히터 목표온도 (0x8112)
    read_dgus_vp(0x8112, (u8*)buf, 2);
    u16 setBotTemp = buf[0];

    // 디버그 출력 (필요 시 UART로 확인)
    // printf("TargetP=%d, Hold=%d, Ttop=%d, Tbot=%d\n", targetP, holdTimeSec, setTopTemp, setBotTemp);
}
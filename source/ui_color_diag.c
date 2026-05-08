#include "ui.h"
#include "timer.h"
#include <string.h>
#include "hmi.h"

/* ──────────────────────────────────────────────────────────────
   RGB565 컬러 상수
   ────────────────────────────────────────────────────────────── */
#define COL_BLACK 0x0000
#define COL_WHITE 0xFFFF
#define COL_RED   0xF800
#define COL_GREEN 0x07E0
#define COL_BLUE  0x001F

/* ──────────────────────────────────────────────────────────────
   ※ 외부 제공 함수 가정
   - write_dgus_vp(u16 vp, u8 *data, u16 words)
   - len 인자는 "워드" 개수(2바이트 단위)라고 가정
   ────────────────────────────────────────────────────────────── */

/* ──────────────────────────────────────────────────────────────
   안전한 워드 쓰기 헬퍼 (LE: u16 그대로)
   ────────────────────────────────────────────────────────────── */
static void DGUS_WriteWord(u16 vp, u16 val) {
    write_dgus_vp(vp, (u8*)&val, 1);  // 1워드(2바이트)
}

/* 상위→하위 바이트 순서로 강제 전송(BE) */
static void DGUS_WriteWord_BE(u16 vp, u16 val) {
    u8 b[2]; 
    b[0] = (u8)((val >> 8) & 0xFF);    // High
    b[1] = (u8)( val        & 0xFF);   // Low
    write_dgus_vp(vp, b, 1);
}

/* ──────────────────────────────────────────────────────────────
   컬러 스윕(진단용): 0x880x 경로 + SP 오프셋 직접 접근 둘 다 시도
   - 어느 케이스에서든 색이 한번이라도 바뀌면 그 조합이 정답
   ────────────────────────────────────────────────────────────── */
static void DGUS_Color_Sweep_All(u16 sp_addr) {
    static const u16 colors[] = { COL_WHITE, COL_BLACK, COL_RED, COL_GREEN, COL_BLUE };
    int i;

    /* 1) 0x8800에 SP 선택 */
    DGUS_WriteWord(0x8800, sp_addr);

    /* 2) 0x8801~0x8806까지 BE로 쭉 시도 */
    for (i = 0; i < 5; i++) {
        DGUS_WriteWord_BE(0x8801, colors[i]);
        DGUS_WriteWord_BE(0x8802, colors[i]);
        DGUS_WriteWord_BE(0x8803, colors[i]);   /* 매뉴얼상 보통 여기(Color) */
        DGUS_WriteWord_BE(0x8804, colors[i]);
        DGUS_WriteWord_BE(0x8805, colors[i]);
        DGUS_WriteWord_BE(0x8806, colors[i]);
    }

    /* 3) 같은 오프셋들을 LE(u16 그대로)로도 시도 */
    for (i = 0; i < 5; i++) {
        DGUS_WriteWord(0x8801, colors[i]);
        DGUS_WriteWord(0x8802, colors[i]);
        DGUS_WriteWord(0x8803, colors[i]);
        DGUS_WriteWord(0x8804, colors[i]);
        DGUS_WriteWord(0x8805, colors[i]);
        DGUS_WriteWord(0x8806, colors[i]);
    }

    /* 4) SP 오프셋 직접 접근 (일부 펌웨어에서 허용)
          sp+3이 보통 Color */
    for (i = 0; i < 5; i++) {
        DGUS_WriteWord(sp_addr + 1, colors[i]);
        DGUS_WriteWord(sp_addr + 2, colors[i]);
        DGUS_WriteWord(sp_addr + 3, colors[i]); /* 보통 컬러 */
        DGUS_WriteWord(sp_addr + 4, colors[i]);
        DGUS_WriteWord(sp_addr + 5, colors[i]);
        DGUS_WriteWord(sp_addr + 6, colors[i]);
    }
}

/* ──────────────────────────────────────────────────────────────
   정답 적용(가장 흔한 조합): 0x8800 선택 + 0x8803 BE(상/하위)
   ────────────────────────────────────────────────────────────── */
static void DGUS_ApplyColor_SP_8803_BE(u16 sp_addr, u16 rgb565) {
    DGUS_WriteWord(0x8800, sp_addr);           /* SP 선택 */
    DGUS_WriteWord_BE(0x8803, rgb565);         /* 색상(빅엔디안) */
}

/* ──────────────────────────────────────────────────────────────
   대안: Attr VP 방식(툴에서 Show colour Attr VP를 따로 잡은 경우)
   ────────────────────────────────────────────────────────────── */
static void DGUS_ApplyColor_ATTR_VP(u16 attr_vp, u16 rgb565) {
    DGUS_WriteWord(attr_vp, rgb565);
}

/* ──────────────────────────────────────────────────────────────
   페이지 진입 직후 한 번만 호출
   - sp_addr   : 컨트롤의 SP (당신 화면에선 0x5000)
   - attr_vp   : Show colour Attr VP를 툴에서 잡았다면 그 주소(없으면 0)
   - do_sweep  : 1=진단 스윕 수행, 0=바로 적용만
   ────────────────────────────────────────────────────────────── */
void DGUS_Color_Diagnose_And_Apply(u16 sp_addr, u16 attr_vp, bit do_sweep) {
    /* 1) 진단을 먼저 돌려서 어떤 조합이 먹히는지 확인(1회) */
    if (do_sweep) {
        DGUS_Color_Sweep_All(sp_addr);
        /* 스윕만 돌리고 끝내려면 여기서 return; */
    }

    /* 2) 가장 표준 조합으로 적용(대부분 이게 정답) */
    DGUS_ApplyColor_SP_8803_BE(sp_addr, COL_RED);

    /* 3) 만약 위가 안 먹힌다면, Attr VP 방식도 병행(있을 때만) */
    if (attr_vp != 0) {
        DGUS_ApplyColor_ATTR_VP(attr_vp, COL_RED);
    }
}

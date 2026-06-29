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

static u16 language = 0;
// 0 - 영어, 1 - 중국어, 2 - 한국어

//한국어

//어드민 리스트
const u16 USER_SETTING_KOR[] = {0xC720, 0xC800, 0x0020, 0xC138, 0xD305};//유저 세팅
const u16 CHECK_KOR[] ={0x0049, 0x004F, 0x0020, 0xD14C, 0xC2A4, 0xD2B8};// io 테스트
const u16 FACTORY_RESET_KOR[] = {0xACF5, 0xC7A5, 0x0020, 0xCD08, 0xAE30, 0xD654};// 공장 초기화
const u16 LOG_ERROR_KOR[] ={0xB85C, 0xADF8, 0x0020, 0x0026, 0x0020, 0xC5D0, 0xB7EC};// 로그 및 에러
const u16 WORK_TIME_KOR[] ={0xC791, 0xB3D9, 0x0020, 0xC2DC, 0xAC04};// 작업 시간
const u16 LANGUAGE_KOR[] = {0xC5B8, 0XC5B4};//언어
const u16 COMPANY_KOR[] ={0xD68C, 0xC0AC, 0xC18C, 0xAC1C};// 회사 소개
const u16 ENGINEER_MODE_KOR[] ={0xC5D4, 0xC9C0, 0xB2C8, 0xC5B4, 0x0020, 0xBAA8, 0xB4DC};//엔지니어 모드

// 유저 세팅
const u16 TOP_TEMP[]      = {0xC0C1,0xD310,0x0020,0xC628,0xB3C4};
const u16 BOTTOM_TEMP[]   = {0xD558,0xD310,0x0020,0xC628,0xB3C4};
const u16 PRESSURE[]      = {0xC555,0xB825};
const u16 DELAY_TIME[]    = {0xC9C0,0xC5F0,0xC2DC,0xAC04};

//IO테스트
const u16 TOP_HEATER_KOR[]      = {0xC0C1,0xD310,0x0020,0xD788,0xD130};
const u16 BOTTOM_HEATER_KOR[]   = {0xD558,0xD310,0x0020,0xD788,0xD130};
const u16 TOP_FAN_KOR[]         = {0xC0C1,0xD310,0x0020,0xD32C};
const u16 BOTTOM_FAN_KOR[]      = {0xD558,0xD310,0x0020,0xD32C};
const u16 COMPRESSOR_KOR[]      = {0xCF64,0xD504,0xB808,0xC0E4};
const u16 SOLENOID_KOR[]        = {0xC194,0xB808,0xB178,0xC774,0xB4DC};

//공장 초기화
const u16 INIT_QUESTION1[] ={0xB370, 0xC774, 0xD130, 0xC774, 0xD130, 0xB97C};
const u16 INIT_QUESTION2[] ={0xCD08, 0xAE30, 0xD654,0xD558, 0xC2DC, 0xACA0,0xC2B5, 0xB2C8, 0xAE4C};
const u16 OK_KOR[] ={0xD655,0xC778};
const u16 CANCEL_KOR[] ={0xCDE8,0xC18C};

//로그 & 에러
const u16 ACTIVE_LOG[] ={0xD65C, 0xC131, 0x0020, 0xB85C, 0xADF8};
const u16 ERROR_LOG[] ={0xC5D0, 0xB7EC, 0x0020, 0xB85C, 0xADF8};

//
const u16 HEATING_TIME_KOR[] = {0xAC00,0xC5F4,0xC2DC,0xAC04}; // 가열시간
const u16 FAN_KOR[] = {0xD32C}; // 팬

const u16 ENGLISH_KOR[] = {0xC601,0xC5B4}; // 영어
const u16 CHINESE_KOR[] = {0xC911,0xAD6D,0xC5B4}; // 중국어
const u16 KOREAN_KOR[] = {0xD55C,0xAD6D,0xC5B4}; // 한국어

const u16 YONGLI_KOREA_KOR[] = {0xC6A9,0xB9AC,0xCF54,0xB9AC,0xC544}; // 용리코리아
const u16 GYEONGGI_DO_KOR[] = {0xACBD,0xAE30,0xB3C4}; // 경기도
const u16 PAJU_JORI_EUP_KOR[] = {0xD30C,0xC8FC,0xC2DC,0x0020,0xC870,0xB9AC,0xC74D}; // 파주시 조리읍
const u16 DANGJAEBONG_RO_29_KOR[] = {0xB2F9,0xC7AC,0xBD09,0xB85C,0x0020,0x0032,0x0039}; // 당재봉로 29
const u16 TEL_KOR[] = {0x0030,0x0033,0x0031,0x002D,0x0039,0x0035,0x0033,0x002D,0x0034,0x0030,0x0036,0x0033}; // 031-953-4063

//영어

const u16 ADMIN[] = {0x0041,0x0044,0x004D,0x0049,0x004E,0x0000};
//어드민 리스트
const u16 USER_SETTING_ENG[] = {0x0055,0x0053,0x0045,0x0052,0x0020,0x0053,0x0045,0x0054,0x0054,0x0049,0x004E,0x0047};// User Setting
const u16 IO_TEST_ENG[] = {0x0049,0x004F,0x0020,0x0054,0x0045,0x0053,0x0054};// IO Test
const u16 FACTORY_RESET_ENG[] = {0x0046,0x0041,0x0043,0x0054,0x004F,0x0052,0x0059,
0x000D,0x000A,
0x0052,0x0045,0x0053,0x0045,0x0054};// Factory Reset
const u16 LOG_ERROR_ENG[] = {0x004C,0x004F,0x0047,0x0020,0x0026,0x0020,0x0045,0x0052,0x0052,0x004F,0x0052};// Log & Error
const u16 RUN_TIME_ENG[] = {0x0052,0x0055,0x004E,0x0020,0x0054,0x0049,0x004D,0x0045};// Run Time
const u16 LANGUAGE_ENG[] = {0x004C, 0x0041, 0x004E, 0x0047, 0x0055, 0x0041, 0x0047, 0x0045};//Language
const u16 COMPANY_INFO_ENG[] = {0x0043,0x004F,0x004D,0x0050,0x0041,0x004E,0x0059,0x0020,0x0049,0x004E,0x0046,0x004F};// Company Info
const u16 ENGINEER_MODE_ENG[] = {0x0045,0x004E,0x0047,0x0049,0x004E,0x0045,0x0045,0x0052,0x0020,0x004D,0x004F,0x0044,0x0045};// Engineer Mode

//유저세팅
const u16 TOP_TEMP_ENG[] = {0x0054,0x004F,0x0050,0x0020,0x0054,0x0045,0x004D,0x0050};// Top Temp
const u16 BOTTOM_TEMP_ENG[] = {0x0042,0x004F,0x0054,0x0054,0x004F,0x004D,0x0020,0x0054,0x0045,0x004D,0x0050};// Bottom Temp
const u16 PRESSURE_ENG[] = {0x0050,0x0052,0x0045,0x0053,0x0053,0x0055,0x0052,0x0045};// Pressure
const u16 DELAY_TIME_ENG[] = {0x0044,0x0045,0x004C,0x0041,0x0059,0x0020,0x0054,0x0049,0x004D,0x0045};// Delay Time

//IO 테스트
const u16 TOP_HEATER_ENG[] = {0x0054,0x004F,0x0050,0x0020,0x0048,0x0045,0x0041,0x0054,0x0045,0x0052};// Top Heater
const u16 BOTTOM_HEATER_ENG[] = {0x0042,0x004F,0x0054,0x0054,0x004F,0x004D,0x0020,0x0048,0x0045,0x0041,0x0054,0x0045,0x0052};// Bottom Heater
const u16 TOP_FAN_ENG[] = {0x0054,0x004F,0x0050,0x0020,0x0046,0x0041,0x004E};// Top Fan
const u16 BOTTOM_FAN_ENG[] = {0x0042,0x004F,0x0054,0x0054,0x004F,0x004D,0x0020,0x0046,0x0041,0x004E};// Bottom Fan
const u16 COMPRESSOR_ENG[] = {0x0043,0x004F,0x004D,0x0050,0x0052,0x0045,0x0053,0x0053,0x004F,0x0052};// Compressor
const u16 SOLENOID_ENG[] = {0x0053,0x004F,0x004C,0x0045,0x004E,0x004F,0x0049,0x0044};// Solenoid

//공장 초기화
const u16 DATA_ENG[] = {0x0044,0x0041,0x0054,0x0041};// Data
const u16 INIT_QUESTION_ENG[] = {0x0049,0x004E,0x0049,0x0054,0x0049,0x0041,0x004C,0x0049,0x005A,0x0045,0x003F};// Initialize?
const u16 YES_ENG[] = {0x0059,0x0045,0x0053}; // YES
const u16 NO_ENG[] = {0x004E,0x004F}; // NO

//로그 & 에러
const u16 ACTIVE_LOG_ENG[] = {0x0041,0x0043,0x0054,0x0049,0x0056,0x0045,0x0020,0x004C,0x004F,0x0047};// Active Log
const u16 ERROR_LOG_ENG[] = {0x0045,0x0052,0x0052,0x004F,0x0052,0x0020,0x004C,0x004F,0x0047};// Error Log

// English / Chinese / Korean
const u16 ENGLISH_ENG[] = {0x0045,0x004E,0x0047,0x004C,0x0049,0x0053,0x0048};
const u16 CHINESE_ENG[] = {0x0043,0x0048,0x0049,0x004E,0x0045,0x0053,0x0045};
const u16 KOREAN_ENG[] = {0x004B,0x004F,0x0052,0x0045,0x0041,0x004E};

//회사 소개
const u16 YONGLI_KOREA_ENG[] = {0x0059,0x004F,0x004E,0x0047,0x004C,0x0049,0x0020,0x004B,0x004F,0x0052,0x0045,0x0041}; // YONGLI KOREA
const u16 GYEONGGI_DO_ENG[] = {0x0047,0x0059,0x0045,0x004F,0x004E,0x0047,0x0047,0x0049,0x002D,0x0044,0x004F}; // GYEONGGI-DO
const u16 PAJU_JORI_ENG[] = {0x0050,0x0041,0x004A,0x0055,0x002D,0x0053,0x0049,0x002C,0x0020,0x004A,0x004F,0x0052,0x0049,0x002D,0x0045,0x0055,0x0050}; // PAJU-SI, JORI-EUP
const u16 DANGJAEBONG_RO_29_ENG[] = {0x0032,0x0039,0x0020,0x0044,0x0061,0x006E,0x0067,0x006A,0x0061,0x0065,0x0062,0x006F,0x006E,0x0067,0x002D,0x0072,0x006F}; //Dangjaebong-ro29 
const u16 ADDRESS_ENG[] = {0x0044,0x0041,0x004E,0x0047,0x004A,0x0041,0x0045,0x0042,0x004F,0x004E,0x0047,0x002D,0x0052,0x004F,0x0020,0x0032,0x0039}; // DANGJAEBONG-RO 29

//중국어
//어드민 리스트
const u16 USER_SETTING_CHN[]    = {0x7528,0x6237,0x8BBE,0x7F6E};                         // 用户设置
const u16 IO_TEST_CHN[]         = {0x0049,0x004F,0x6D4B,0x8BD5};                         // IO测试
const u16 FACTORY_RESET_CHN[]   = {0x5DE5,0x5382,0x521D,0x59CB,0x5316};                 // 工厂初始化
const u16 LOG_ERROR_CHN[]       = {0x65E5,0x5FD7,0x0020,0x0026,0x0020,0x9519,0x8BEF};   // 日志 & 错误
const u16 RUN_TIME_CHN[]        = {0x8FD0,0x884C,0x65F6,0x95F4};                         // 运行时间
const u16 LANGUAGE_CHN[]        = {0x8BED, 0x8A00};                                       // 语言
const u16 COMPANY_INFO_CHN[]    = {0x516C,0x53F8,0x7B80,0x4ECB};                         // 公司简介
const u16 ENGINEER_MODE_CHN[]   = {0x5DE5,0x7A0B,0x5E08,0x6A21,0x5F0F};                 // 工程师模式

//IO테스트
const u16 TOP_HEATER_CHN[]      = {0x9876,0x677F,0x52A0,0x70ED,0x5668};                 // 顶板加热器
const u16 BOTTOM_HEATER_CHN[]   = {0x5E95,0x677F,0x52A0,0x70ED,0x5668};                 // 底板加热器
const u16 TOP_FAN_CHN[]         = {0x9876,0x677F,0x98CE,0x6247};                         // 顶板风扇
const u16 BOTTOM_FAN_CHN[]      = {0x5E95,0x677F,0x98CE,0x6247};                         // 底板风扇
const u16 COMPRESSOR_CHN[]      = {0x538B,0x7F29,0x673A};                               // 压缩机
const u16 SOLENOID_CHN[]        = {0x7535,0x78C1,0x9600};                               // 电磁阀
const u16 TOP_TEMP_CHN[]        = {0x9876,0x677F,0x6E29,0x5EA6};                         // 顶板温度
const u16 BOTTOM_TEMP_CHN[]     = {0x5E95,0x677F,0x6E29,0x5EA6};                         // 底板温度
const u16 PRESSURE_CHN[]        = {0x538B,0x529B};                                       // 压力
const u16 DELAY_TIME_CHN[]      = {0x5EF6,0x8FDF,0x65F6,0x95F4};                         // 延迟时间
//공장 초기화
const u16 DATA_CHN[]            = {0x6570,0x636E};                                       // 数据
const u16 INIT_QUESTION_CHN[]   = {0x521D,0x59CB,0x5316,0x5417,0x003F};                 // 初始化吗?
const u16 YES_CHN[] = {0x786E,0x8BA4};   // 确认
const u16 NO_CHN[]  = {0x53D6,0x6D88};   // 取消
//로그 & 에러
const u16 ACTIVE_LOG_CHN[]      = {0x8FD0,0x884C,0x65E5,0x5FD7};                         // 运行日志
const u16 ERROR_LOG_CHN[]       = {0x9519,0x8BEF,0x65E5,0x5FD7};                         // 错误日志
//언어 변경
const u16 ENGLISH_CHN[]         = {0x82F1,0x8BED};                                       // 英语
const u16 CHINESE_CHN[]         = {0x4E2D,0x6587};                                       // 中文
const u16 KOREAN_CHN[]          = {0x97E9,0x8BED};                                       // 韩语
//회사 소개
const u16 GYEONGGI_DO_CHN[]     = {0x4EAC,0x757F,0x9053};                               // 京畿道
const u16 PAJU_JORI_CHN[]       = {0x5761,0x5DDE,0x5E02,0x0020,0x6761,0x91CC,0x9091};   // 坡州市 条里邑
const u16 ADDRESS_CHN[]         = {0x5802,0x624D,0x5CF0,0x8DEF,0x0020,0x0032,0x0039};   // 堂才峰路 29
const u16 TEL_CHN[]             = {0x0030,0x0033,0x0031,0x002D,0x0039,0x0035,0x0033,0x002D,0x0034,0x0030,0x0036,0x0033}; // 031-953-4063


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
    SetTextColorYellow(0x1103);
    SetTextColorYellow(0x1113);
    SetTextColorYellow(0x1123);
    SetTextColorYellow(0x1133);
    read_dgus_vp(VP_SET_H, (u8*)&timer, 1); 
    StartTimer(TMR_1S, ONE_SEC); 
}

void SecCnt_Stop(void) { 
    sec_active = 0;
    SetTextColorWhite(0x1103);
    SetTextColorWhite(0x1113);
    SetTextColorWhite(0x1123);
    SetTextColorWhite(0x1133);
}


void SecCnt_TickTask(void) {
     if (!sec_active){
        SetTextColorWhite(0x1103);
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

//어드민 메인 리스트
void admin_text_change(){
    u16 len = 0;
      switch(adminSP){
        case 0:{
            len = 0;
            write_dgus_vp(0x5028, (u8*)&len, 1);
            if(language == 0){
                write_dgus_vp(0x4030, (u8*)&USER_SETTING_ENG, 12);
                len = 24;
                write_dgus_vp(0x5038, (u8*)&len, 1);
                write_dgus_vp(0x4400, (u8*)&IO_TEST_ENG, 7);
                len = 14;
                write_dgus_vp(0x5408, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4030, (u8*)&USER_SETTING_CHN, 4);
                len = 8;
                write_dgus_vp(0x5038, (u8*)&len, 1);
                write_dgus_vp(0x4400, (u8*)&IO_TEST_CHN, 4);
                len = 8;
                write_dgus_vp(0x5408, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4010, (u8*)&USER_SETTING_KOR, 5);
                len = 10;
                write_dgus_vp(0x5018, (u8*)&len, 1);

                write_dgus_vp(0x4020, (u8*)&CHECK_KOR, 6);
                len = 12;
                write_dgus_vp(0x5028, (u8*)&len, 1);
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
                write_dgus_vp(0x4400, (u8*)&IO_TEST_ENG, 7);
                len = 14;
                write_dgus_vp(0x5408, (u8*)&len, 1);
            }else if(language == 1){
                write_dgus_vp(0x4030, (u8*)&USER_SETTING_CHN, 4);
                len = 8;
                write_dgus_vp(0x5038, (u8*)&len, 1);
                write_dgus_vp(0x4400, (u8*)&IO_TEST_CHN, 4);
                len = 8;
                write_dgus_vp(0x5408, (u8*)&len, 1);
            }else if(language == 2){
                write_dgus_vp(0x4010, (u8*)&USER_SETTING_KOR, 5);
                len = 10;
                write_dgus_vp(0x5018, (u8*)&len, 1);

                write_dgus_vp(0x4020, (u8*)&CHECK_KOR, 6);
                len = 12;
                write_dgus_vp(0x5028, (u8*)&len, 1);
            }
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
//어드민 유저 세팅
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
//uint16_t buf[1];
static u8 test2 = 0;

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
//어드민 초기화
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
//어드민 로그 및 에러
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
    // LANGUAGE_TEXT_VIEW(0x4010, 0x5018, USER_SETTING_KOR);
    write_dgus_vp(0x4010, (u8*)&ADMIN, 6);
    len = 12;
    write_dgus_vp(0x5018, (u8*)&len, 1);

    write_dgus_vp(0x4020, (u8*)&USER_SETTING_ENG, 12);
    len = 24;
    write_dgus_vp(0x5028, (u8*)&len, 1);

    write_dgus_vp(0x4030, (u8*)&FACTORY_RESET_ENG, 13);
    len = 26;
    write_dgus_vp(0x5038, (u8*)&len, 1);

    write_dgus_vp(0x4040, (u8*)&LOG_ERROR_ENG, 11);
    len = 22;
    write_dgus_vp(0x5048, (u8*)&len, 1);

    write_dgus_vp(0x4050, (u8*)&RUN_TIME_ENG, 8);
    len = 16;
    write_dgus_vp(0x5058, (u8*)&len, 1);

    write_dgus_vp(0x4060, (u8*)&LANGUAGE_ENG, 8);
    len = 16;
    write_dgus_vp(0x5068, (u8*)&len, 1);

    write_dgus_vp(0x4070, (u8*)&COMPANY_INFO_ENG, 12);
    len = 24;
    write_dgus_vp(0x5078, (u8*)&len, 1);

    write_dgus_vp(0x4330, (u8*)&ENGINEER_MODE_ENG, 13);
    len = 26;
    write_dgus_vp(0x5338, (u8*)&len, 1);

    //유저 세팅
    write_dgus_vp(0x4080, (u8*)&USER_SETTING_ENG, 12);
    len = 24;
    write_dgus_vp(0x5088, (u8*)&len, 1);

    write_dgus_vp(0x4090, (u8*)&TOP_TEMP_ENG, 8);
    len = 16;
    write_dgus_vp(0x5098, (u8*)&len, 1);

    write_dgus_vp(0x4120, (u8*)&BOTTOM_TEMP_ENG, 11);
    len = 22;
    write_dgus_vp(0x5128, (u8*)&len, 1);

    write_dgus_vp(0x4150, (u8*)&PRESSURE_ENG, 8);
    len = 16;
    write_dgus_vp(0x5158, (u8*)&len, 1);

    write_dgus_vp(0x4180, (u8*)&DELAY_TIME_ENG, 10);
    len = 20;
    write_dgus_vp(0x5188, (u8*)&len, 1);

    //IO 테스트
    write_dgus_vp(0x4400, (u8*)&IO_TEST_ENG, 7);
    len = 14;
    write_dgus_vp(0x5408, (u8*)&len, 1);

    write_dgus_vp(0x4210, (u8*)&TOP_HEATER_ENG, 11);
    len = 22;
    write_dgus_vp(0x5218, (u8*)&len, 1);

    write_dgus_vp(0x4240, (u8*)&BOTTOM_HEATER_ENG, 13);
    len = 26;
    write_dgus_vp(0x5248, (u8*)&len, 1);

    write_dgus_vp(0x4270, (u8*)&TOP_FAN_ENG, 7);
    len = 14;
    write_dgus_vp(0x5278, (u8*)&len, 1);

    write_dgus_vp(0x4300, (u8*)&BOTTOM_FAN_ENG, 10);
    len = 20;
    write_dgus_vp(0x5308, (u8*)&len, 1);

    write_dgus_vp(0x4340, (u8*)&COMPANY_INFO_ENG, 12);
    len = 24;
    write_dgus_vp(0x5348, (u8*)&len, 1);

    write_dgus_vp(0x4370, (u8*)&SOLENOID_ENG, 8);
    len = 16;
    write_dgus_vp(0x5378, (u8*)&len, 1);
    
    //공장 초기화
    write_dgus_vp(0x4760, (u8*)&FACTORY_RESET_ENG, 10);
    len = 20;
    write_dgus_vp(0x5768, (u8*)&len, 1);

    write_dgus_vp(0x4770, (u8*)&DATA_ENG, 4);
    len = 8;
    write_dgus_vp(0x5778, (u8*)&len, 1);

    write_dgus_vp(0x4780, (u8*)&INIT_QUESTION_ENG, 11);
    len = 22;
    write_dgus_vp(0x5788, (u8*)&len, 1);

    write_dgus_vp(0x4790, (u8*)&DELAY_TIME_ENG, 10);
    len = 20;
    write_dgus_vp(0x5798, (u8*)&len, 1);

    write_dgus_vp(0x4800, (u8*)&DELAY_TIME_ENG, 10);
    len = 20;
    write_dgus_vp(0x5808, (u8*)&len, 1);

    //로그 & 에러
    write_dgus_vp(0x4650, (u8*)&LOG_ERROR_ENG, 11);
    len = 22;
    write_dgus_vp(0x5658, (u8*)&len, 1);

    write_dgus_vp(0x4670, (u8*)&ACTIVE_LOG_ENG, 10);
    len = 20;
    write_dgus_vp(0x5678, (u8*)&len, 1);

    write_dgus_vp(0x4680, (u8*)&ERROR_LOG_ENG, 9);
    len = 18;
    write_dgus_vp(0x5688, (u8*)&len, 1);

    //작업 시간
    write_dgus_vp(0x4410, (u8*)&RUN_TIME_ENG, 8);
    len = 16;
    write_dgus_vp(0x5418, (u8*)&len, 1);

    write_dgus_vp(0x4420, (u8*)&ERROR_LOG_ENG, 9);
    len = 18;
    write_dgus_vp(0x5428, (u8*)&len, 1);

    write_dgus_vp(0x4460, (u8*)&ERROR_LOG_ENG, 9);
    len = 18;
    write_dgus_vp(0x5468, (u8*)&len, 1);

    write_dgus_vp(0x4500, (u8*)&ERROR_LOG_ENG, 9);
    len = 18;
    write_dgus_vp(0x5508, (u8*)&len, 1);

    //언어 변경
    write_dgus_vp(0x4540, (u8*)&LANGUAGE_ENG, 8);
    len = 16;
    write_dgus_vp(0x5548, (u8*)&len, 1);

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
    write_dgus_vp(0x4580, (u8*)&COMPANY_INFO_ENG, 12);
    len = 24;
    write_dgus_vp(0x5588, (u8*)&len, 1);

    write_dgus_vp(0x4590, (u8*)&YONGLI_KOREA_ENG, 12);
    len = 24;
    write_dgus_vp(0x5598, (u8*)&len, 1);

    write_dgus_vp(0x4600, (u8*)&GYEONGGI_DO_ENG, 11);
    len = 22;
    write_dgus_vp(0x5608, (u8*)&len, 1);

    write_dgus_vp(0x4610, (u8*)&PAJU_JORI_ENG, 17);
    len = 34;
    write_dgus_vp(0x5618, (u8*)&len, 1);

    write_dgus_vp(0x4620, (u8*)&DANGJAEBONG_RO_29_ENG, 17);
    len = 34;
    write_dgus_vp(0x5628, (u8*)&len, 1);

    write_dgus_vp(0x4630, (u8*)&TEL_CHN, 12);
    len = 24;
    write_dgus_vp(0x5638, (u8*)&len, 1);

    //엔지니어 모드
    write_dgus_vp(0x4660, (u8*)&ENGINEER_MODE_CHN, 5);
    len = 10;
    write_dgus_vp(0x5698, (u8*)&len, 1);
}

void admin_language_chn(void){
    u16 len = 0;
    //어드민 메인리스트
    write_dgus_vp(0x4010, (u8*)&USER_SETTING_CHN, 4);
    len = 8;
    write_dgus_vp(0x5018, (u8*)&len, 1);

    write_dgus_vp(0x4020, (u8*)&IO_TEST_CHN, 4);
    len = 8;
    write_dgus_vp(0x5028, (u8*)&len, 1);

    write_dgus_vp(0x4030, (u8*)&FACTORY_RESET_CHN, 5);
    len = 10;
    write_dgus_vp(0x5038, (u8*)&len, 1);

    write_dgus_vp(0x4040, (u8*)&LOG_ERROR_CHN, 7);
    len = 14;
    write_dgus_vp(0x5048, (u8*)&len, 1);

    write_dgus_vp(0x4050, (u8*)&RUN_TIME_CHN, 4);
    len = 8;
    write_dgus_vp(0x5058, (u8*)&len, 1);

    write_dgus_vp(0x4060, (u8*)&LANGUAGE_CHN, 2);
    len = 4;
    write_dgus_vp(0x5068, (u8*)&len, 1);

    write_dgus_vp(0x4070, (u8*)&COMPANY_INFO_CHN, 4);
    len = 8;
    write_dgus_vp(0x5078, (u8*)&len, 1);

    write_dgus_vp(0x4330, (u8*)&ENGINEER_MODE_ENG, 5);
    len = 10;
    write_dgus_vp(0x5338, (u8*)&len, 1);

    //유저 세팅
    write_dgus_vp(0x4080, (u8*)&USER_SETTING_CHN, 4);
    len = 8;
    write_dgus_vp(0x5088, (u8*)&len, 1);

    write_dgus_vp(0x4090, (u8*)&TOP_TEMP_CHN, 4);
    len = 8;
    write_dgus_vp(0x5098, (u8*)&len, 1);

    write_dgus_vp(0x4120, (u8*)&BOTTOM_TEMP_CHN, 4);
    len = 8;
    write_dgus_vp(0x5128, (u8*)&len, 1);

    write_dgus_vp(0x4150, (u8*)&PRESSURE_CHN, 2);
    len = 4;
    write_dgus_vp(0x5158, (u8*)&len, 1);

    write_dgus_vp(0x4180, (u8*)&DELAY_TIME_CHN, 4);
    len = 8;
    write_dgus_vp(0x5188, (u8*)&len, 1);

    //IO 테스트
    write_dgus_vp(0x4400, (u8*)&IO_TEST_CHN, 7);
    len = 14;
    write_dgus_vp(0x5408, (u8*)&len, 1);

    write_dgus_vp(0x4210, (u8*)&TOP_HEATER_CHN, 5);
    len = 10;
    write_dgus_vp(0x5218, (u8*)&len, 1);

    write_dgus_vp(0x4240, (u8*)&BOTTOM_HEATER_CHN, 5);
    len = 10;
    write_dgus_vp(0x5248, (u8*)&len, 1);

    write_dgus_vp(0x4270, (u8*)&TOP_FAN_CHN, 4);
    len = 8;
    write_dgus_vp(0x5278, (u8*)&len, 1);

    write_dgus_vp(0x4300, (u8*)&BOTTOM_FAN_CHN, 4);
    len = 8;
    write_dgus_vp(0x5308, (u8*)&len, 1);

    write_dgus_vp(0x4340, (u8*)&COMPRESSOR_CHN, 3);
    len = 6;
    write_dgus_vp(0x5348, (u8*)&len, 1);

    write_dgus_vp(0x4370, (u8*)&SOLENOID_CHN, 3);
    len = 6;
    write_dgus_vp(0x5378, (u8*)&len, 1);
    
    //공장 초기화
    write_dgus_vp(0x4760, (u8*)&FACTORY_RESET_CHN, 5);
    len = 10;
    write_dgus_vp(0x5768, (u8*)&len, 1);

    write_dgus_vp(0x4770, (u8*)&DATA_CHN, 2);
    len = 4;
    write_dgus_vp(0x5778, (u8*)&len, 1);

    write_dgus_vp(0x4780, (u8*)&INIT_QUESTION_CHN, 5);
    len = 10;
    write_dgus_vp(0x5788, (u8*)&len, 1);

    write_dgus_vp(0x4790, (u8*)&YES_CHN, 2);
    len = 4;
    write_dgus_vp(0x5798, (u8*)&len, 1);

    write_dgus_vp(0x4800, (u8*)&NO_CHN, 2);
    len = 4;
    write_dgus_vp(0x5808, (u8*)&len, 1);

    //로그 & 에러
    write_dgus_vp(0x4650, (u8*)&LOG_ERROR_CHN, 7);
    len = 14;
    write_dgus_vp(0x5658, (u8*)&len, 1);

    write_dgus_vp(0x4670, (u8*)&ACTIVE_LOG_CHN, 4);
    len = 8;
    write_dgus_vp(0x5678, (u8*)&len, 1);

    write_dgus_vp(0x4680, (u8*)&ERROR_LOG_CHN, 4);
    len = 8;
    write_dgus_vp(0x5688, (u8*)&len, 1);

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
    write_dgus_vp(0x4580, (u8*)&COMPANY_INFO_CHN, 4);
    len = 8;
    write_dgus_vp(0x5588, (u8*)&len, 1);

    write_dgus_vp(0x4590, (u8*)&YONGLI_KOREA_ENG, 12);
    len = 24;
    write_dgus_vp(0x5598, (u8*)&len, 1);

    write_dgus_vp(0x4600, (u8*)&GYEONGGI_DO_CHN, 3);
    len = 6;
    write_dgus_vp(0x5608, (u8*)&len, 1);

    write_dgus_vp(0x4610, (u8*)&PAJU_JORI_CHN, 7);
    len = 14;
    write_dgus_vp(0x5618, (u8*)&len, 1);

    write_dgus_vp(0x4620, (u8*)&ADDRESS_CHN, 7);
    len = 14;
    write_dgus_vp(0x5628, (u8*)&len, 1);

    write_dgus_vp(0x4630, (u8*)&TEL_CHN, 12);
    len = 24;
    write_dgus_vp(0x5638, (u8*)&len, 1);

    //엔지니어 모드
    write_dgus_vp(0x4660, (u8*)&ENGINEER_MODE_ENG, 13);
    len = 26;
    write_dgus_vp(0x5698, (u8*)&len, 1);

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

void admin_language_kor(void){
    u16 len = 0;
    //어드민 메인리스트
    write_dgus_vp(0x4010, (u8*)&USER_SETTING_KOR, 5);
    len = 10;
    write_dgus_vp(0x5018, (u8*)&len, 1);

    write_dgus_vp(0x4020, (u8*)&CHECK_KOR, 6);
    len = 12;
    write_dgus_vp(0x5028, (u8*)&len, 1);

    write_dgus_vp(0x4030, (u8*)&FACTORY_RESET_KOR, 6);
    len = 12;
    write_dgus_vp(0x5038, (u8*)&len, 1);

    write_dgus_vp(0x4040, (u8*)&LOG_ERROR_KOR, 7);
    len = 14;
    write_dgus_vp(0x5048, (u8*)&len, 1);

    write_dgus_vp(0x4050, (u8*)&WORK_TIME_KOR, 5);
    len = 10;
    write_dgus_vp(0x5058, (u8*)&len, 1);

    write_dgus_vp(0x4060, (u8*)&LANGUAGE_KOR, 2);
    len = 4;
    write_dgus_vp(0x5068, (u8*)&len, 1);

    write_dgus_vp(0x4070, (u8*)&COMPANY_KOR, 4);
    len = 8;
    write_dgus_vp(0x5078, (u8*)&len, 1);

    write_dgus_vp(0x4330, (u8*)&ENGINEER_MODE_KOR, 7);
    len = 14;
    write_dgus_vp(0x5338, (u8*)&len, 1);

        //유저 세팅
    write_dgus_vp(0x4080, (u8*)&USER_SETTING_KOR, 5);
    len = 10;
    write_dgus_vp(0x5088, (u8*)&len, 1);

    write_dgus_vp(0x4090, (u8*)&TOP_TEMP, 5);
    len = 10;
    write_dgus_vp(0x5098, (u8*)&len, 1);

    write_dgus_vp(0x4120, (u8*)&BOTTOM_TEMP, 5);
    len = 10;
    write_dgus_vp(0x5128, (u8*)&len, 1);

    write_dgus_vp(0x4150, (u8*)&PRESSURE, 2);
    len = 4;
    write_dgus_vp(0x5158, (u8*)&len, 1);

    write_dgus_vp(0x4180, (u8*)&DELAY_TIME, 4);
    len = 8;
    write_dgus_vp(0x5188, (u8*)&len, 1);

    //IO 테스트
    write_dgus_vp(0x4400, (u8*)&CHECK_KOR, 6);
    len = 14;
    write_dgus_vp(0x5408, (u8*)&len, 1);

    write_dgus_vp(0x4210, (u8*)&TOP_HEATER_KOR, 5);
    len = 10;
    write_dgus_vp(0x5218, (u8*)&len, 1);

    write_dgus_vp(0x4240, (u8*)&BOTTOM_HEATER_KOR, 5);
    len = 10;
    write_dgus_vp(0x5248, (u8*)&len, 1);

    write_dgus_vp(0x4270, (u8*)&TOP_FAN_KOR, 4);
    len = 8;
    write_dgus_vp(0x5278, (u8*)&len, 1);

    write_dgus_vp(0x4300, (u8*)&BOTTOM_FAN_KOR, 4);
    len = 8;
    write_dgus_vp(0x5308, (u8*)&len, 1);

    write_dgus_vp(0x4340, (u8*)&COMPRESSOR_KOR, 4);
    len = 8;
    write_dgus_vp(0x5348, (u8*)&len, 1);

    write_dgus_vp(0x4370, (u8*)&SOLENOID_KOR, 5);
    len = 10;
    write_dgus_vp(0x5378, (u8*)&len, 1);
    
    //공장 초기화
    write_dgus_vp(0x4760, (u8*)&FACTORY_RESET_KOR, 6);
    len = 12;
    write_dgus_vp(0x5768, (u8*)&len, 1);

    write_dgus_vp(0x4770, (u8*)&INIT_QUESTION1, 6);
    len = 12;
    write_dgus_vp(0x5778, (u8*)&len, 1);

    write_dgus_vp(0x4780, (u8*)&INIT_QUESTION2, 9);
    len = 18;
    write_dgus_vp(0x5788, (u8*)&len, 1);

    write_dgus_vp(0x4790, (u8*)&OK_KOR, 2);
    len = 4;
    write_dgus_vp(0x5798, (u8*)&len, 1);

    write_dgus_vp(0x4800, (u8*)&CANCEL_KOR, 2);
    len = 4;
    write_dgus_vp(0x5808, (u8*)&len, 1);

    //로그 & 에러
    write_dgus_vp(0x4650, (u8*)&LOG_ERROR_KOR, 7);
    len = 14;
    write_dgus_vp(0x5658, (u8*)&len, 1);

    write_dgus_vp(0x4670, (u8*)&ACTIVE_LOG_CHN, 5);
    len = 10;
    write_dgus_vp(0x5678, (u8*)&len, 1);

    write_dgus_vp(0x4680, (u8*)&ERROR_LOG_CHN, 5);
    len = 10;
    write_dgus_vp(0x5688, (u8*)&len, 1);

    //작업 시간
    write_dgus_vp(0x4410, (u8*)&WORK_TIME_KOR, 5);
    len = 10;
    write_dgus_vp(0x5418, (u8*)&len, 1);

    write_dgus_vp(0x4420, (u8*)&HEATING_TIME_KOR, 4);
    len = 8;
    write_dgus_vp(0x5428, (u8*)&len, 1);

    write_dgus_vp(0x4460, (u8*)&FAN_KOR, 1);
    len = 2;
    write_dgus_vp(0x5468, (u8*)&len, 1);

    write_dgus_vp(0x4500, (u8*)&COMPRESSOR_KOR, 9);
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
    write_dgus_vp(0x4580, (u8*)&COMPANY_KOR, 4);
    len = 8;
    write_dgus_vp(0x5588, (u8*)&len, 1);

    write_dgus_vp(0x4590, (u8*)&YONGLI_KOREA_KOR, 5);
    len = 24;
    write_dgus_vp(0x5598, (u8*)&len, 1);

    write_dgus_vp(0x4600, (u8*)&GYEONGGI_DO_KOR, 3);
    len = 6;
    write_dgus_vp(0x5608, (u8*)&len, 1);

    write_dgus_vp(0x4610, (u8*)&PAJU_JORI_EUP_KOR, 7);
    len = 14;
    write_dgus_vp(0x5618, (u8*)&len, 1);

    write_dgus_vp(0x4620, (u8*)&DANGJAEBONG_RO_29_KOR, 7);
    len = 14;
    write_dgus_vp(0x5628, (u8*)&len, 1);

    write_dgus_vp(0x4630, (u8*)&TEL_KOR, 12);
    len = 24;
    write_dgus_vp(0x5638, (u8*)&len, 1);

    //엔지니어 모드
    write_dgus_vp(0x4660, (u8*)&ENGINEER_MODE_KOR, 7);
    len = 14;
    write_dgus_vp(0x5698, (u8*)&len, 1);

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
        write_dgus_vp(0x1423, (u8*)&color, 1);  
    }else if(f_topcool == 1){
        u16 color = 0x051C;
        write_dgus_vp(0x1423, (u8*)&color, 1);  
    }else{
         u16 color = 0xCE59;
        write_dgus_vp(0x1423, (u8*)&color, 1);  
    }

    if(f_bottemp == 1){
         u16 color = 0xC044;
         write_dgus_vp(0x1433, (u8*)&color, 1);  
    }else if(f_botcool == 1){
         u16 color = 0x051C;
         write_dgus_vp(0x1433, (u8*)&color, 1);  
    }else{
         u16 color = 0xCE59;
        write_dgus_vp(0x1433, (u8*)&color, 1);  
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
                admin_language_eng();
                // if(page_number == adminList){
                //     admin_page_change();
                // }else if(page_number == adminUserSetting){
                //     settingflag = 1;
                //     usersettingSelect_plag = 1;
                //     adminUserSettingEdit();
                // }else if(page_number == adminIOTest){
                //     if(IOtestSelect_plag == 0){
                //         IOtestSelect_plag = 1;
                //     }else{
                //         IOtestSelect_plag = 0;
                //     }
                //     adminIoTestWork();
                // }else if(page_number == adminFactoryResetNotice){
                //     Page_Change_UI(adminList);
                // }else if(page_number == adminLogError){
                //     if(LogErrorSP == 0){
                //         Page_Change_UI(adminActiveLog);
                //     }else if(LogErrorSP == 1){
                //         Page_Change_UI(adminErrorLog);
                //     }
                // }else if(page_number == adminMaintenance){
                //     Page_Change_UI(main7);
                // }else if(page_number == adminLanguage){
                //     if(LanguageSP == 0){
                //         language_eng();
                //     }else if(LanguageSP == 1){
                //         language_chn();
                //     }else if(LanguageSP == 2){
                //         language_kor();
                //     }
                    
                //     // Page_Change_UI(main7);
                // }else if(page_number == adminCompany){
                //     Page_Change_UI(main7);
                // }else if(page_number == adminEngineermod){
                //     if(EngineerSP>=0&&EngineerSP<5){
                //         EngineerSP++;
                //         EngineermodTextChange();
                //     }else if(EngineerSP==5){
                //         EngineerSP = 0;
                //         Page_Change_UI(adminEngineermodS);
                //     }
                // }else if(page_number == adminActiveLog){
                //     Page_Change_UI(adminLogError);
                // }else if(page_number == adminErrorLog){
                //     Page_Change_UI(adminLogError);
                // }else if(page_number == adminEngineermodS){
                //     settingflag = 1;
                // }
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
                // admininit();
                // settingflag = 0;
                // if(page_number == adminList){
                //     admin_plag = 0;
                //     Page_Change_UI(main7);
                // }else{
                //     Page_Change_UI(adminList);
                // }

                Page_Change_UI(adminList);
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


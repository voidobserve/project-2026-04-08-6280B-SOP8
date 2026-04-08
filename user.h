/******************************************************************************
;  *   	@型号				  : MC30P6280
;  *   	@创建日期             : 2021.08.04
;  *   	@公司/作者			  : SINOMCU-FAE
;  *   	@晟矽微技术支持       : 2048615934
;  *   	@晟矽微官网           : http://www.sinomcu.com/
;  *   	@版权                 : 2021 SINOMCU公司版权所有.
;  *---------------------- 建议 ---------------------------------
;  *   				变量定义时使用全局变量
******************************************************************************/
#ifndef USER
#define USER

#include "mc30-common.h"
#include "mc30P6280.h"

#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long int
#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long int

#define DEF_SET_BIT0 0x01
#define DEF_SET_BIT1 0x02
#define DEF_SET_BIT2 0x04
#define DEF_SET_BIT3 0x08
#define DEF_SET_BIT4 0x10
#define DEF_SET_BIT5 0x20
#define DEF_SET_BIT6 0x40
#define DEF_SET_BIT7 0x80

#define DEF_CLR_BIT0 0xFE
#define DEF_CLR_BIT1 0xFD
#define DEF_CLR_BIT2 0xFB
#define DEF_CLR_BIT3 0xF7
#define DEF_CLR_BIT4 0xEF
#define DEF_CLR_BIT5 0xDF
#define DEF_CLR_BIT6 0xBF
#define DEF_CLR_BIT7 0x7F

#define ENABLE 1
#define DISABLE 0

#define FAIL 1
#define PASS 0

// 按键检测引脚
#define KEY_SCAN_PIN P10D
// // 笔头供电控制引脚（目前不用推挽输出来驱动，还需要结合实际的电路板测试一下）
// #define PEN_POWER_PIN P14D



// 如果只消抖2次，会过滤不掉抖动
#define KEY_FILTER_TIMES (3) // 按键消抖次数 (消抖时间 == 消抖次数 * 按键扫描时间)
// 定义按键键值
enum
{
	KEY_ID_NONE = 0x00,
	KEY_ID_VAILD,
};
// 定义按键事件
enum
{
	KEY_EVENT_NONE = 0x00,
	KEY_EVENT_CLICK, // 单击
	KEY_EVENT_HOLD, // 
	KEY_EVENT_LOOSE, // 长按后松开
};



//===============Field Protection Variables===============
u8 abuf;
u8 statusbuf;

//===============IO Define===============

//===============Global Variable===============

//===============Global Function===============
void CLR_RAM(void);
void IO_Init(void);
void LVD_Init(void);
void Sys_Init(void);

//===============Define  Flag===============
typedef union
{
	unsigned char byte;
	struct
	{
		u8 bit0 : 1;
		u8 bit1 : 1;
		u8 bit2 : 1;
		u8 bit3 : 1;
		u8 bit4 : 1;
		u8 bit5 : 1;
		u8 bit6 : 1;
		u8 bit7 : 1;
	} bits;
} bit_flag_t;
volatile bit_flag_t flag1;

// #define  	FLAG_TIMER0_500ms  	flag1.bits.bit0	   	 // 标志位

#endif

/**************************** end of file *********************************************/
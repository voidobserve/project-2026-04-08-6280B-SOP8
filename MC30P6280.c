/******************************************************************************
;  *   	@型号				  : MC30P6280
;  *   	@创建日期             : 2021.08.04
;  *   	@公司/作者			  : SINOMCU-FAE
;  *   	@晟矽微技术支持       : 2048615934
;  *   	@晟矽微官网           : http://www.sinomcu.com/
;  *   	@版权                 : 2021 SINOMCU公司版权所有.
;  *----------------------摘要描述---------------------------------
;  *					LVD  低电压检测
					VDD低于设定4V   P12输出高，反之输出低
******************************************************************************/

#include "user.h"

/************************************************
;  *    @Function Name       : C_RAM
;  *    @Description         : 初始化RAM
;  *    @IN_Parameter      	 :
;  *    @Return parameter    :
;  ***********************************************/
void C_RAM(void)
{
	__asm;
	movai 0x3F;
	movra FSR;
	movai 47;
	movra 0x3F;
	decr FSR;
	clrr INDF;
	djzr 0x3F;
	goto $ - 3;
	clrr 0x3F;
	__endasm;
}

/************************************************
;  *    @Function Name       : IO_Init
;  *    @Description         :
;  *    @IN_Parameter      	 :
;  *    @Return parameter    :
;  ***********************************************/
void IO_Init(void)
{
	P1 = 0x00;	  // 1:input 0:output
	DDR1 = 0x00;  // 1:input 0:output
	PUCON = 0xff; // 0:Effective 1:invalid
	PDCON = 0xff; // 0:Effective 1:invalid
	ODCON = 0x00; // 0:推挽输出  1:开漏输出
}
/************************************************
;  *    @Function Name       : LVD_Init
;  *    @Description         : LVD set
;  *    @IN_Parameter      	 :
;  *    @Return parameter    :
;  ***********************************************/
void LVD_Init(void)
{
	MCR &= ~(DEF_SET_BIT1 | DEF_SET_BIT2 | DEF_SET_BIT3 | DEF_SET_BIT4);
	MCR |= (DEF_SET_BIT2 | DEF_SET_BIT3 | DEF_SET_BIT4); // 4V
	LVDEN = 1;											 // 使能LVD
}
/************************************************
;  *    @Function Name       : Sys_Init
;  *    @Description         : 系统初始化
;  *    @IN_Parameter      	 :
;  *    @Return parameter    :
;  ***********************************************/
void Sys_Init(void)
{
	GIE = 0;
	C_RAM();
	IO_Init();
	LVD_Init();
	GIE = 1;
}

// 毫秒级延时
// 前提条件：FCPU = FHIRC / 1
void delay_ms(u32 xms)
{
	while (xms)
	{
		u16 i = 531;
		while (i--)
		{
			Nop();
		}
		xms--;
	}
}

//=================================================================================

void user_init(void)
{
	// 按键检测引脚配置 P10
	PUCON &= ~(0x01 << 0); // 上拉电阻 20K
	DDR1 |= 0x01 << 0;	   // 输入模式

	// 控制笔头的电源引脚配置 P14
	DDR1 |= 0x01 << 4; // P14 输入模式

	// 滚珠检测引脚配置 P15
	DDR1 |= 0x01 << 5; // 输入模式

	delay_ms(1); // 等待系统稳定
}

void pen_pwr_on(void)
{
	PDCON &= ~(0x01 << 4); // 下拉电阻 20K

	// DDR1 |= 0x01 << 4;	   // P14 输入模式
}

void pen_pwr_off(void)
{
	PUCON &= ~(0x01 << 4); // 上拉电阻 20K

	// DDR1 |= 0x01 << 4;	   // P14 输入模式
}

// 关闭所有led
void led_all_off(void)
{
	// 关闭所有LED驱动引脚的上下拉：
	PUCON |= (0x01 << 0); // 关闭上拉

	// 所有led驱动引脚配置为 输入模式
	DDR1 |= (0x01 << 0) | // P10
			(0x01 << 1) | // P11
			(0x01 << 2);  // P12
}

void led1_on(void)
{
	// 不使用的引脚配置为输入模式，如果每次都关所有灯，再点亮指定灯，这一步可以省

	// LED公共端配置为输出，输出低电平
	// P10 LED 公共端
	P1 &= ~(0x01 << 0);
	DDR1 &= ~(0x01 << 0); // 输出模式

	// LED B端 配置为输出，输出高电平
	// P11 LED B端
	P1 |= (0x01 << 1);	  // P11 输出高电平
	DDR1 &= ~(0x01 << 1); // P11 输出模式
}

void led2_on(void)
{
	// 不使用的引脚配置为输入模式，如果每次都关所有灯，再点亮指定灯，这一步可以省

	// B端 输出低电平， 公共端 输出高电平
	// P11 LED B端
	P1 &= ~(0x01 << 1);	  // P11 输出低电平
	DDR1 &= ~(0x01 << 1); // P11 输出模式

	// P10 LED 公共端
	P1 |= (0x01 << 0);	  // P10 输出高电平
	DDR1 &= ~(0x01 << 0); // P10 输出模式
}

void led3_on(void)
{
	// 不使用的引脚配置为输入模式，如果每次都关所有灯，再点亮指定灯，这一步可以省
	// A端 输出低电平，公共端 输出高电平
	// P12 LED A端
	P1 &= ~(0x01 << 2);	  // P12 输出低电平
	DDR1 &= ~(0x01 << 2); // P12 输出模式
	// P10 LED 公共端
	P1 |= (0x01 << 0);	  // P10 输出高电平
	DDR1 &= ~(0x01 << 0); // P10 输出模式
}

void led4_on(void)
{
	// 不使用的引脚配置为输入模式，如果每次都关所有灯，再点亮指定灯，这一步可以省
	// 公共端 输出低电平，A端 输出高电平
	// P10 LED 公共端
	P1 &= ~(0x01 << 0);	  // P10 输出低电平
	DDR1 &= ~(0x01 << 0); // P10 输出模式
	// P12 LED A端
	P1 |= (0x01 << 2);	  // P12 输出高电平
	DDR1 &= ~(0x01 << 2); // P12 输出模式
}

// 10ms 定时中断
void timer0_init(void)
{
}

// 100us 定时中断
void timer1_init(void)
{
}

// 按键检测，放在10ms定时器中断处理
void key_scan(void)
{
	// 按键检测引脚配置 P10
	PUCON &= ~(0x01 << 0); // 上拉电阻 20K
	DDR1 |= 0x01 << 0;	   // 输入模式

	if (KEY_SCAN_PIN == 0)
	{
		// 按键按下
	}

	static volatile u8 last_key_id = KEY_ID_NONE;
	static volatile u8 press_cnt = 0;				// 按键按下的时间计数
	static volatile u8 filter_cnt = 0;				// 按键消抖，使用的变量
	static volatile u8 filter_key_id = KEY_ID_NONE; // 按键消抖时使用的变量
	// volatile u8 cur_key_id = get_key_id();          // 获取当前按键的键值(id)
	volatile u8 cur_key_id = KEY_ID_NONE;

	static volatile u8 flag_is_key_mode_hold = 0;

	if (1 == KEY_SCAN_PIN)
	{
		cur_key_id = KEY_ID_VAILD;
	}
	else
	{
		cur_key_id = KEY_ID_NONE;
	}

	if (cur_key_id != filter_key_id)
	{
		// 如果有按键按下/松开
		filter_cnt = 0;
		filter_key_id = cur_key_id;
		return;
	}

	if (filter_cnt < KEY_FILTER_TIMES)
	{
		// 如果检测到相同的按键按下/松开
		// 防止计数溢出
		filter_cnt++;
		return;
	}

	// 滤波/消抖完成后，执行到这里
#if 1
	if (last_key_id != cur_key_id)
	{
		// USER_TO_DO 还需要添加按键双击检测操作
		if (last_key_id == KEY_ID_NONE)
		{
			// 如果有按键按下，清除按键按下的时间计数
			press_cnt = 0;
		}
		else if (cur_key_id == KEY_ID_NONE)
		{
			// 如果按键松开
			if (press_cnt < 75)
			{
				// 按下时间小于 750ms ，是短按
				// if (KEY_ID_MODE == last_key_id)
				// {
				//     // 开关/模式按键短按
				//     key_event = KEY_EVENT_MODE_PRESS;
				// }
			}
			else
			{
				// 长按、长按持续之后松手
				// key_event = KEY_EVENT_NONE;

				// if (KEY_ID_MODE == last_key_id)
				// {
				//     flag_is_key_mode_hold = 0;
				// }
			}
		}
	}
	else if (cur_key_id != KEY_ID_NONE)
	{
		// 如果按键按住不放
		if (press_cnt < 255)
			press_cnt++;

		// 长按对应的操作：
		// if (KEY_ID_MODE == cur_key_id)
		// {
		//     if (FLAG_IS_DEVICE_OPEN)
		//     {
		//         if (press_cnt >= 200) // 2000ms
		//         {
		//             if (flag_is_key_mode_hold)
		//             {
		//             }
		//             else
		//             {
		//                 key_event = KEY_EVENT_MODE_HOLD;
		//                 flag_is_key_mode_hold = 1;
		//             }
		//         }
		//     }
		//     else
		//     {
		//         // 如果当前设备是关闭的
		//         if (press_cnt >= 100) // 1000ms加上看门狗唤醒的1024ms
		//         {
		//             if (flag_is_key_mode_hold)
		//             {
		//             }
		//             else
		//             {
		//                 key_event = KEY_EVENT_MODE_HOLD;
		//                 flag_is_key_mode_hold = 1;
		//             }
		//         }
		//     }
		// }
	}

	last_key_id = cur_key_id;
#endif
}

void main(void)
{
	Sys_Init();
	user_init();

	pen_pwr_on();
	led_all_off();

	while (1)
	{
		// if (KEY_SCAN_PIN == 0)
		// {
		// 	P11D = 0;
		// }
		// else
		// {
		// 	P11D = 1;
		// }

		led_all_off();
		led1_on();
		delay_ms(500);
		// delay(1145);
		// delay_ms(2);

		led_all_off();
		led2_on();
		delay_ms(500);
		// delay(1145);
		// delay_ms(2);

		led_all_off();
		led3_on();
		delay_ms(500);
		// delay(1145);
		// delay_ms(2);

		led_all_off();
		led4_on();
		delay_ms(500);
		// delay(1145);
		// delay_ms(2);
	}
}

/************************************************
;  *    @Function Name       : Interrupt
;  *    @Description         : The interrupt function
;  *    @IN_Parameter          	 :
;  *    @Return parameter      	:
;  ***********************************************/
void int_isr(void) __interrupt
{
	__asm;
	movra _abuf;
	swapar _STATUS;
	movra _statusbuf;
	__endasm;
	__asm;
	swapar _statusbuf;
	movra _STATUS;
	swapr _abuf;
	swapar _abuf;
	__endasm;
}

/**************************** end of file *********************************************/
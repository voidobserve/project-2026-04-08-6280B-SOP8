/******************************************************************************
;  *   	@型号				  : MC30P6280
;  *   	@创建日期             : 2021.08.04
;  *   	@公司/作者			  : SINOMCU-FAE
;  *   	@晟矽微技术支持       : 2048615934
;  *   	@晟矽微官网           : http://www.sinomcu.com/
;  *   	@版权                 : 2021 SINOMCU公司版权所有.
;  *----------------------摘要描述---------------------------------
******************************************************************************/
#include "user.h"

u8 abuf;
u8 statusbuf;

volatile bit_flag_t flag1;
// volatile bit_flag_t flag2;

// volatile dev_sta_t dev_sta;
volatile bat_lev_t tmp_bat_lev = BAT_LEV_4V0; // 临时的电池电压挡位
volatile bat_lev_t bat_lev = BAT_LEV_4V0;     // 稳定之后的、电池电压挡位
volatile u32 pwr_off_cnt = 0;                 // 5min 自动关机的倒计时
/*
    设备没有开机、并且没有在充电，累计计数，
    满足一定时间后进入低功耗
*/
volatile u8 into_low_power_cnt = 0;

volatile u8 charge_anim_phase = 0; // 控制充电动画

volatile u8 i; // 循环计数值

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
    P1 = 0x00;    // 1:input 0:output
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

// void LVD_Init(void)
// {
//     MCR &= ~(DEF_SET_BIT1 | DEF_SET_BIT2 | DEF_SET_BIT3 | DEF_SET_BIT4);
//     MCR |= (DEF_SET_BIT2 | DEF_SET_BIT3 | DEF_SET_BIT4); // 4V

//     // 清空 LVD 电压检测阀值配置
//     MCR &= ~(0x01 << 1 | 0x01 << 2 | 0x01 << 3 | 0x01 << 4);
//     MCR |= LVD_CFG_3V0;

//     LVDEN = 1; // 使能LVD
// }

// 至少隔1ms才调用一次:
void lvd_scan(void)
{
    static u8 cur_lvd_lev = LVD_LEV_NONE;

    switch (cur_lvd_lev) {
    case LVD_LEV_NONE: // LVD_LEV_NONE、LVD_LEV_2V9都用同一个语句块
    case LVD_LEV_2V9:  // LVD_LEV_NONE、LVD_LEV_2V9都用同一个语句块
        /*
            刚上电、刚从低功耗恢复、或者是在 LVD_LEV_2V9 状态
            切换到 4V0 档位
        */
        // 清空 LVD 电压检测阀值配置
        MCR &= ~MCR_LVD_CFG_ALL;
        MCR |= MCR_LVD_CFG_4V0;
        cur_lvd_lev = LVD_LEV_4V0_SWITCHING;
        break;
    case LVD_LEV_4V0_SWITCHING:
        cur_lvd_lev = LVD_LEV_4V0;
        if (0 == LVDF) {
            // 如果VDD不低于该挡位
            tmp_bat_lev = BAT_LEV_4V0;
        }
        break;
    case LVD_LEV_4V0:
        // 4V0 档位切换到 3V6 档位

        // 清空 LVD 电压检测阀值配置
        MCR &= ~MCR_LVD_CFG_ALL;
        MCR |= MCR_LVD_CFG_3V6;
        cur_lvd_lev = LVD_LEV_3V6_SWITCHING;
        break;
    case LVD_LEV_3V6_SWITCHING:
        cur_lvd_lev = LVD_LEV_3V6;
        if (0 == LVDF) {
            // 如果VDD不低于该挡位
            tmp_bat_lev = BAT_LEV_3V6;
        }
        break;
    case LVD_LEV_3V6:
        // 3V6 档位切换到 3V3 档位

        // 清空 LVD 电压检测阀值配置
        MCR &= ~MCR_LVD_CFG_ALL;
        MCR |= MCR_LVD_CFG_3V3;
        cur_lvd_lev = LVD_LEV_3V3_SWITCHING;
        break;
    case LVD_LEV_3V3_SWITCHING:
        cur_lvd_lev = LVD_LEV_3V3;
        if (0 == LVDF) {
            // 如果VDD不低于该挡位
            tmp_bat_lev = BAT_LEV_3V3;
        }
        break;
    case LVD_LEV_3V3:
        // 3V3 档位切换到 3V2 档位

        // 清空 LVD 电压检测阀值配置
        MCR &= ~MCR_LVD_CFG_ALL;
        MCR |= MCR_LVD_CFG_3V2;
        cur_lvd_lev = LVD_LEV_3V2_SWITCHING;
        break;
    case LVD_LEV_3V2_SWITCHING:
        cur_lvd_lev = LVD_LEV_3V2;
        if (0 == LVDF) {
            // 如果VDD不低于该挡位
            tmp_bat_lev = BAT_LEV_3V2;
        }
        break;
    case LVD_LEV_3V2:
        // 3V2 档位切换到 3V0 档位

        // 清空 LVD 电压检测阀值配置
        MCR &= ~MCR_LVD_CFG_ALL;
        MCR |= MCR_LVD_CFG_3V0;
        cur_lvd_lev = LVD_LEV_3V0_SWITCHING;
        break;
    case LVD_LEV_3V0_SWITCHING:
        cur_lvd_lev = LVD_LEV_3V0;
        if (0 == LVDF) {
            // 如果VDD不低于该挡位
            tmp_bat_lev = BAT_LEV_3V0;
        }
        break;
    case LVD_LEV_3V0:
        // 3V0 档位切换到 2V9 档位

        // 清空 LVD 电压检测阀值配置
        MCR &= ~MCR_LVD_CFG_ALL;
        MCR |= MCR_LVD_CFG_2V9;
        cur_lvd_lev = LVD_LEV_2V9_SWITCHING;
        break;
    case LVD_LEV_2V9_SWITCHING:
        cur_lvd_lev = LVD_LEV_2V9;
        if (0 == LVDF) {
            // 如果VDD不低于该挡位
            tmp_bat_lev = BAT_LEV_2V9;
        }
        break;
    }
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
    GIE = 1;
}

// 毫秒级延时
// 前提条件：FCPU = FHIRC / 1 / 2 == 8MHz
void delay_ms(u32 xms)
{
    while (xms) {
        u16 i = 531;
        while (i--) {
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
    DDR1 |= 0x01 << 0;     // 输入模式

    // 控制笔头的电源引脚配置 P14
    DDR1 |= 0x01 << 4; // P14 输入模式

    // 滚珠检测引脚配置 P15
    DDR1 |= 0x01 << 5; // 输入模式

    timer0_init();
    timer1_init();

#if USER_DEBUG_ENABLE
    // P14 控制笔头的电源引脚配置为输出模式，作为测试引脚
    DDR1 &= ~(0x01 << 4);
#endif

    delay_ms(1); // 等待系统稳定
}

void pen_pwr_on(void)
{
#if 0
    PDCON &= ~(0x01 << 4); // 下拉电阻 20K 
    // DDR1 |= 0x01 << 4;	   // P14 输入模式
#endif

    PDCON &= ~(0x01 << 4); // 下拉电阻 20K
    DDR1 |= 0x01 << 4;     // P14 输入模式
}

void pen_pwr_off(void)
{
#if 0
    PUCON &= ~(0x01 << 4); // 上拉电阻 20K 
    // DDR1 |= 0x01 << 4;	   // P14 输入模式
#endif

    PUCON |= (0x01 << 4); // 不使能上拉
    PDCON |= (0x01 << 4); // 不使能下拉
    DDR1 |= 0x01 << 4;    // P14 输入模式
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
    P1 |= (0x01 << 1);    // P11 输出高电平
    DDR1 &= ~(0x01 << 1); // P11 输出模式
}

void led2_on(void)
{
    // 不使用的引脚配置为输入模式，如果每次都关所有灯，再点亮指定灯，这一步可以省

    // B端 输出低电平， 公共端 输出高电平
    // P11 LED B端
    P1 &= ~(0x01 << 1);   // P11 输出低电平
    DDR1 &= ~(0x01 << 1); // P11 输出模式

    // P10 LED 公共端
    P1 |= (0x01 << 0);    // P10 输出高电平
    DDR1 &= ~(0x01 << 0); // P10 输出模式
}

void led3_on(void)
{
    // 不使用的引脚配置为输入模式，如果每次都关所有灯，再点亮指定灯，这一步可以省
    // A端 输出低电平，公共端 输出高电平
    // P12 LED A端
    P1 &= ~(0x01 << 2);   // P12 输出低电平
    DDR1 &= ~(0x01 << 2); // P12 输出模式
    // P10 LED 公共端
    P1 |= (0x01 << 0);    // P10 输出高电平
    DDR1 &= ~(0x01 << 0); // P10 输出模式
}

void led4_on(void)
{
    // 不使用的引脚配置为输入模式，如果每次都关所有灯，再点亮指定灯，这一步可以省
    // 公共端 输出低电平，A端 输出高电平
    // P10 LED 公共端
    P1 &= ~(0x01 << 0);   // P10 输出低电平
    DDR1 &= ~(0x01 << 0); // P10 输出模式
    // P12 LED A端
    P1 |= (0x01 << 2);    // P12 输出高电平
    DDR1 &= ~(0x01 << 2); // P12 输出模式
}

// 按键检测，放在10ms定时器中断处理
void key_scan(void)
{
    static volatile u8 last_key_id = KEY_ID_NONE;
    static volatile u8 press_cnt = 0;               // 按键按下的时间计数
    static volatile u8 filter_cnt = 0;              // 按键消抖，使用的变量
    static volatile u8 filter_key_id = KEY_ID_NONE; // 按键消抖时使用的变量
    // 按键短按松开之后，记录时间，用于判断这段时间内是否有再次按下
    static volatile u8 click_delay_cnt = 0;
    static volatile u8 click_cnt = 0; // 记录按键连击的次数
    volatile u8 cur_key_id = KEY_ID_NONE;

    if (flag_is_in_charging) {
        // 充电期间，不检测按键
        last_key_id = 0;
        press_cnt = 0;
        filter_cnt = 0;
        filter_key_id = 0;
        click_delay_cnt = 0;
        click_cnt = 0;
        cur_key_id = 0; 
        return;
    }

    // 按键检测引脚配置 P10
    PUCON &= ~(0x01 << 0); // 上拉电阻 20K
    DDR1 |= 0x01 << 0;     // 输入模式

    if (1 == KEY_SCAN_PIN) {
        cur_key_id = KEY_ID_NONE;
    } else {
        // 按键按下
        cur_key_id = KEY_ID_VAILD;
    }

    if (cur_key_id != filter_key_id) {
        // 如果有按键按下/松开
        filter_cnt = 0;
        filter_key_id = cur_key_id;
        return;
    }

    if (filter_cnt < KEY_FILTER_TIMES) {
        // 如果检测到相同的按键按下/松开
        // 防止计数溢出
        filter_cnt++;
        return;
    }

    // 滤波/消抖完成后，执行到这里
    if (last_key_id != cur_key_id) {
        // last_key_id 为有效键值，而 cur_key_id
        // 为无效键值，说明按键刚开始松开
        if (cur_key_id == KEY_ID_NONE) {
            // 开始计时，等待下次按键按下，最后判断有没有按键连击
            click_delay_cnt = 0;
        } else // cur_key_id 为有效键值，而 last_key_id
               // 为无效键值，说明按键刚按下
        {
            press_cnt = 0; // 重置按键按下时间计数
            click_cnt++;
        }
    }
    // else if (cur_key_id != KEY_ID_NONE)
    else // cur_key_id == last_key_id
    {
        /*
            cur_key_id == last_key_id &&
            cur_key_id == KEY_ID_NONE，说明按键已经松开
        */
        if (cur_key_id == KEY_ID_NONE) { // 没有按键按下，说明松手
            if (click_cnt > 0) {
                if (click_delay_cnt >= KEY_CLICK_DELAY_TIME) {
                    if (click_cnt == 1) {
                        // 单击
                    } else {
                        // click_cnt == 2 // 双击
                        flag_is_dev_working = ~flag_is_dev_working;
                        if (flag_is_dev_working) {
                            pen_pwr_on();
                        } else {
                            pen_pwr_off();
                        }
                    }

                    click_cnt = 0; // 处理完事件后,清空连击的次数
                } else {
                    if (click_delay_cnt < 255) // 防止计数溢出
                        click_delay_cnt++;
                }
            }
        }
        // cur_key_id == last_key_id && cur_key_id !=
        // KEY_ID_NONE，说明按键按下未松开
        else {
            // 如果按键按住不放
            if (press_cnt < 255)
                press_cnt++;
        }
    }

    last_key_id = cur_key_id;
}

// 放在10ms的定时器中断内调用
void led_status_handle(void)
{
    static volatile u8 cnt = 0; // 控制LED闪烁时间间隔
    cnt++;
    if (cnt >= (u8)(500 / 10)) {
        cnt = 0;
        if (flag_is_in_charging) {
            // 充电时，控制指示灯跑马显示

            // 如果刚进入充电，执行跑马灯动画，到对应的电压后停止
            if (flag_is_charge_begin) {
                if (bat_lev > BAT_LEV_3V2) {
                    /*
                        数值大于 BAT_LEV_3V2 ，说明电池电量小于 BAT_LEV_3V2
                        退出刚开始充电的动画
                    */
                    flag_is_charge_begin = 0;
                    charge_anim_phase = 0;
                } else if (bat_lev <= BAT_LEV_3V2 && charge_anim_phase == 0) {
                    // 数值 小于等于 BAT_LEV_3V2 ，说明电池电量 大于等于 BAT_LEV_3V2
                    flag_led_1_on = 1;
                    charge_anim_phase = 1;
                } else if (bat_lev <= BAT_LEV_3V3 && charge_anim_phase == 1) {
                    // 数值 小于等于 BAT_LEV_3V3 ，说明电池电量 大于等于 BAT_LEV_3V3
                    flag_led_2_on = 1;
                    charge_anim_phase = 2;
                } else if (bat_lev <= BAT_LEV_3V6 && charge_anim_phase == 2) {
                    // 数值 小于等于 BAT_LEV_3V6 ，说明电池电量 大于等于 BAT_LEV_3V6
                    flag_led_3_on = 1;
                    charge_anim_phase = 3;
                } else if (bat_lev <= BAT_LEV_4V0 && charge_anim_phase == 3) {
                    // 数值 小于等于 BAT_LEV_4V0 ，说明电池电量 大于等于 BAT_LEV_4V0
                    flag_led_4_on = 1;
                    charge_anim_phase = 4;
                } else {
                    flag_is_charge_begin = 0; // 退出刚开始充电的动画
                    /*
                        退出刚开始充电的动画之后，需要立即进入对应的处理
                        这里给计数值恢复成 500 ms
                    */
                    cnt = (u8)(500 / 10);
                }
            } else {
                // 如果不是刚进入充电，根据电池电量档位来控制指示灯闪烁
                switch (bat_lev) {
                case BAT_LEV_4V0: // 满电
                    flag_led_4_on = 1;
                    break;
                case BAT_LEV_3V6:
                    flag_led_4_on = ~flag_led_4_on;
                    break;
                case BAT_LEV_3V3:
                    flag_led_3_on = ~flag_led_3_on;
                    break;
                case BAT_LEV_3V2:
                    flag_led_2_on = ~flag_led_2_on;
                    break;
                default:
                    flag_led_1_on = ~flag_led_1_on;
                    break;
                }

                if (bat_lev <= BAT_LEV_3V2) {
                    flag_led_1_on = 1; // 指示灯常亮，不闪烁
                } else if (bat_lev <= BAT_LEV_3V3) {
                    flag_led_2_on = 1; // 指示灯常亮，不闪烁
                } else if (bat_lev <= BAT_LEV_3V6) {
                    flag_led_3_on = 1; // 指示灯常亮，不闪烁
                }
            }
        } else {
            // 放电时，控制指示灯常亮

            /*
                USER_TO_DO
                如果这里采用 flag1.byte |= xx;
                flag1.byte &= xx;这种方式来给标志位赋值，
                每个语句块可以节省1个字节空间
            */
            if (bat_lev <= BAT_LEV_4V0) {
                flag_led_4_on = 1;
                flag_led_3_on = 1;
                flag_led_2_on = 1;
                flag_led_1_on = 1;
            } else if (bat_lev <= BAT_LEV_3V6) {
                flag_led_4_on = 0;
                flag_led_3_on = 1;
                flag_led_2_on = 1;
                flag_led_1_on = 1;
            } else if (bat_lev <= BAT_LEV_3V3) {
                flag_led_4_on = 0;
                flag_led_3_on = 0;
                flag_led_2_on = 1;
                flag_led_1_on = 1;
            } else if (bat_lev <= BAT_LEV_3V2) {
                flag_led_4_on = 0;
                flag_led_3_on = 0;
                flag_led_2_on = 0;
                flag_led_1_on = 1;
            } else {
                flag_led_4_on = 0;
                flag_led_3_on = 0;
                flag_led_2_on = 0;
                // 低电量，指示灯闪烁
                flag_led_1_on = ~flag_led_1_on;
            }
        }
    }
}

// 根据电池电量和设备状态，来控制电池电量指示灯
// 目前暂定在 100us 定时器中断里面调用
void led_refresh(void)
{
    static volatile u8 sta = 0;
    led_all_off();

    switch (sta) {
    case 0:
        if (flag_led_1_on) {
            led1_on();
        }
        sta = 1;
        break;
    case 1:
        if (flag_led_2_on) {
            led2_on();
        }
        sta = 2;
        break;
    case 2:
        if (flag_led_3_on) {
            led3_on();
        }
        sta = 3;
        break;
    case 3:
        if (flag_led_4_on) {
            led4_on();
        }
        sta = 0;
        break;

    default:
        break;
    }
}

void main(void)
{
    Sys_Init();
    user_init();

    while (1) {

        /*
            如果不在充电，并且到了关机的时间

            如果不在充电并且设备不在运行，2s后进入低功耗
        */
        if ((flag_is_dev_working && pwr_off_cnt >= (u32)5 * 60 * 1000) ||
            into_low_power_cnt >= (u8)((u16)2000 / 10)) {
        label_low_power_in: // 标签，进入低功耗

            GIE = 0; // 关闭总中断

            // 关闭定时器：
            T0IE = 0; // 屏蔽 timer0 中断
            T1IE = 0; // 屏蔽 timer1 中断
            T1EN = 0; // 不使能定时器

            // 所有引脚配置为输入模式、关闭上下拉
            PUCON = 0xFF; // 0:Effective 1:invalid
            PDCON = 0xFF; // 0:Effective 1:invalid
            DDR1 = 0x00;  // 1:input 0:output

            // 配置唤醒源： 充电、按键
            PUCON &= 0x01 << 0; // 上拉 20K ， P10(按键检测脚)

            // 使能键盘中断
            KBIF = 0; // 清除中断标志
            KBIE = 1; // 使能键盘中断
            // P10(按键检测脚) 、 P13(充电检测脚) 打开键盘中断
            P1KBCR |= 0x01 << 0 | 0x01 << 3;

            // =================================================================
            // 进入低功耗
            Stop();
            Nop();
            Nop();
            // =================================================================

            // 唤醒之后，关闭键盘中断
            P1KBCR &= ~(0x01 << 0 | 0x01 << 3);
            KBIE = 0;
            KBIF = 0;

            // 立即获取一次电池电量
            for (i = 0; i < 40; i++) {
                lvd_scan();
                delay_ms(1);
            }

            bat_lev = tmp_bat_lev;

            // 如果没有在充电、没有按下按键、没有在充电并且低电量，重新回到低功耗
            if ((CHARGE_PIN == 0 && bat_lev > BAT_LEV_3V0) ||
                (CHARGE_PIN == 0 && KEY_SCAN_PIN == 1)) {
                goto label_low_power_in;
            }

            Sys_Init();
            user_init();
        }
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

    if ((T0IF) && (T0IE)) {
        T0IF = 0;
        T0CNT = (u8)(255 - 125); // T0 目前1ms产生一次中断

        {
            static volatile u8 cnt = 0;
            cnt++;
            if (cnt >= 10) {
                // 10ms进入一次
                cnt = 0;
                // P14D = ~P14D;

                key_scan();

                led_status_handle();

                if (flag_is_dev_working) {

                    // 防止计数溢出
                    if (pwr_off_cnt < (u32)5 * 60 * 1000 / 10) {
                        pwr_off_cnt++;
                    }
                } else {
                    // 不在工作，清空计数
                    pwr_off_cnt = 0;
                }

                /*
                    USER_TO_DO
                    用两个u8类型的变量来取代u16类型的变量，进行计数，
                    最后程序占用会少一个字节
                */
                // {
                //     static u8 cnt = 0;
                //     cnt++;
                //     if (cnt >= 100) // 100 * 10ms
                //     {
                //         cnt = 0;
                //         {
                //             static u8 cnt = 0;
                //             cnt++;
                //             if (cnt >= 150) {
                //                 cnt = 0;
                //                 P14D = ~P14D;
                //             }
                //         }
                //     }
                // }

                {
                    static u16 cnt = 0;
                    cnt++;
                    if (cnt >= (u16)150 * 1000 / 10) {
                        // 实际测试有150s，每隔150进入一次这里
                        // 每 150s 更新一次电池电量
                        cnt = 0;

                        bat_lev = tmp_bat_lev;
                    }
                }

                // 充电检测
                {
                    static u8 cnt = 0;
                    if (CHARGE_PIN) {
                        cnt++;
                        if (cnt >= 10) {
                            cnt = 0;
                            if (0 == flag_is_in_charging) {
                                pen_pwr_off();           // 断开笔头的供电
                                led_all_off();           // 关闭所有指示灯，准备充电动画
                                flag_is_dev_working = 0; // 表示设备关闭
                                // 表示刚进入充电
                                flag_is_charge_begin = 1;
                                charge_anim_phase = 0;
                                flag_is_in_charging = 1;
                            }
                        }
                    } else {
                        cnt = 0;
                        flag_is_in_charging = 0;
                    }
                }

                // 如果不在充电，设备也没有在工作
                if (flag_is_in_charging == 0 && flag_is_dev_working == 0) {
                    if (into_low_power_cnt < 255) {
                        into_low_power_cnt++;
                    }
                } else {
                    into_low_power_cnt = 0;
                }
            }
        }

        lvd_scan();
    }

    // 100us 定时器中断
    if ((T1IF) && (T1IE)) {
        T1IF = 0;
        led_refresh();

        // 如果检测到的振动传感器传来的信号，当前检测脚的电平跟上次的不一样
        if ((VIBRATION_SENSOR_PIN && last_vibration_sensor_lev == 0) ||
            (VIBRATION_SENSOR_PIN == 0 && last_vibration_sensor_lev)) {

            // 在这里添加清空关机的计时操作
            pwr_off_cnt = 0;

            last_vibration_sensor_lev = VIBRATION_SENSOR_PIN;
            // printf("detect lev\n"); // 测试可以检测到电平变化
        }
    }

    __asm;
    swapar _statusbuf;
    movra _STATUS;
    swapr _abuf;
    swapar _abuf;
    __endasm;
}

/**************************** end of file
 * *********************************************/
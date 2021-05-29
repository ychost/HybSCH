#include "./Headers/PreMacro.h"
#include "./Headers/HybSCH.h"

//调度器错误代码
static u8 hsch_error_code;
static u8 last_error_code;

//错误代码输出超时
static u16 error_tick_count;

/**
 * 任务结构体
 * @field func   任务函数
 * @field delay  初次执行延时
 * @field period 执行周期间隔
 * @field co_cp  合作/抢占标志
 *               合作：1
 *               抢占：0
 */
typedef data struct
{
    Action func;
    u16 delay;
    u16 period;
    u8 runme;
    u8 co_cp;
} hsTask;

//任务集合
hsTask hsch_tasks[SCH_MAX_TASKS];

/**
 * 使单片机进入休眠
 */
void hsch_to_sleep(void);

/**
 * 扫描任务集合并执行
 */
void hsch_dispatch_tasks(void)
{
    u8 i = 0;
    for (i = 0; i < SCH_MAX_TASKS; ++i)
    {
        if (hsch_tasks[i].runme > 0 && hsch_tasks[i].co_cp)
        {
            hsch_tasks[i].func();
            --hsch_tasks[i].runme;

            if (hsch_tasks[i].period == 0)
            {
                hsch_delete_task(i);
            }
        }
    }
    hsch_report_status();
    hsch_to_sleep();
}

/**
 * 添加任务
 * @param  func   任务函数
 * @param  delay  首次执行延时
 * @param  period 周期执行间隔
 * @param  co_cp  合作/抢占标志
 * @return        任务ID [用于删除任务]
 */
u8 hsch_add_task(Action func, u16 delay, u16 period, u8 co_cp)
{
    u8 i = 0;
    while ((hsch_tasks[i].func != NULL) && (i < SCH_MAX_TASKS))
    {
        ++i;
    }
    if (i == SCH_MAX_TASKS)
    {
        hsch_error_code = ERROR_SCH_TOO_MANY_TASKS;
        return SCH_MAX_TASKS;
    }
    hsch_tasks[i].func = func;
    hsch_tasks[i].delay = delay;
    hsch_tasks[i].period = period;
    hsch_tasks[i].co_cp = co_cp;
    hsch_tasks[i].runme = 0;

    return i;
}

/**
 * 删除任务
 * @param  i 任务ID
 * @return   删除状态
 */
bool hsch_delete_task(u8 i)
{
    bool ret_code;
    if (hsch_tasks[i].func == NULL)
    {
        hsch_error_code = ERROR_SCH_CANNOT_DELETE_TASK;
        ret_code = RETURN_ERROR;
    }
    else
    {
        ret_code = RETURN_NORMAL;
    }
    hsch_tasks[i].func = NULL;
    hsch_tasks[i].delay = 0;
    hsch_tasks[i].period = 0;
    hsch_tasks[i].runme = 0;

    return ret_code;
}

/**
 * 更新调度器，周期由T2设置
 */
void hsch_update(void) interrupt TIMMER2_ITRP
{
    u8 i = 0;
    TF2 = 0;
    for (i = 0; i < SCH_MAX_TASKS; ++i)
    {
        if (hsch_tasks[i].func)
        {
            if (hsch_tasks[i].delay == 0)
            {
                //若是合作式则在中断中只置位,等待到hsch_dispatch_tasks中去执行
                if (hsch_tasks[i].co_cp)
                {
                    ++hsch_tasks[i].runme;
                }
                //若是抢占方则在中断中执行该函数
                else
                {
                    hsch_tasks[i].func();
                    hsch_tasks[i].runme -= 1;
                    if (hsch_tasks[i].period == 0)
                    {
                        hsch_tasks[i].func = NULL;
                    }
                }
                //如果是周期执行则将周期间隔赋予下次执行的延时
                if (hsch_tasks[i].period)
                {
                    hsch_tasks[i].delay = hsch_tasks[i].period;
                }
            }
            //延时计算
            else
            {
                --hsch_tasks[i].delay;
            }
        }
    }
}

/**
 * 用Timmer2作为调度器的时钟源,默认1ms为周期
 */
void hsch_init_timmer2(void)
{
    u8 i = 0;
    for (i = 0; i < SCH_MAX_TASKS; ++i)
    {
        hsch_delete_task(i);
    }
    hsch_error_code = 0;

    T2CON = 0x04;
    T2MOD = 0x00;
    TH2 = 0xfc;
    RCAP2H = 0xfc;
    TL2 = 0x18;
    RCAP2L = 0x18;
    ET2 = 1;
    TR2 = 1;
}

/**
 * 单片机开启休眠
 */
void hsch_to_sleep(void)
{
    PCON |= 0x01;
}

/**
 * 启动调度器
 */
void hsch_start(void)
{
    EA = 1;
}

/**
 * 输出调度器状态
 */
void hsch_report_status(void)
{
#ifdef SCH_REPORT_STATUS
    if (hsch_error_code != last_error_code)
    {
        ERROR_PORT = 0xff - hsch_error_code;
        last_error_code = hsch_error_code;
        if (hsch_error_code != 0)
        {
            error_tick_count = 6000;
        }
        else
        {
            error_tick_count = 0;
        }
    }
    else
    {
        if (error_tick_count != 0)
        {
            if (--error_tick_count == 0)
            {
                hsch_error_code = 0;
            }
        }
    }
#endif
}
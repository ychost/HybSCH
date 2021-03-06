#include "./Headers/PreMacro.h"
#include "./Headers/HybSCH.h"

//调度器错误代码
static u8 hsch_error_code;
static u8 last_error_code;

//错误代码输出超时
static u16 error_tick_count;

extern u16 soft_delay_count;

/**
 * 随机种子
 */
u16 random_seeds = 0;

/**
 * 任务结构体
 * @field func   任务函数
 * @field delay  初次执行延时
 * @field period 执行周期间隔
 * @field exec_once 是否已经执行了一次
 * @field co_cp  合作/抢占标志
 *               合作：1
 *               抢占：0
 */
typedef xdata struct
{
    Action func;
    int delay;
    u16 period;
    u8 runme;
    u8 co_cp;
    u8 exec_once;
    void *param;
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
    for (i = 0; i < SCH_MAX_TASKS; i += 1)
    {
        if (hsch_tasks[i].runme > 0 && hsch_tasks[i].co_cp && hsch_tasks[i].func != NULL)
        {
            hsch_tasks[i].runme -= 1;
            hsch_tasks[i].func(hsch_tasks[i].param);
            // 注销一次性函数
            if (hsch_tasks[i].period == 0 && hsch_tasks[i].runme == 0)
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
 * @param  param func 入参
 * @param  period 周期执行间隔
 * @param  co_cp  合作/抢占标志
 * @return        任务ID [用于删除任务]
 */
u8 hsch_add_task(Action func, void *param, u16 delay, u16 period, u8 co_cp)
{
    u8 i = 0;
    while ((hsch_tasks[i].func != NULL) && (i < SCH_MAX_TASKS))
    {
        i += 1;
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
    hsch_tasks[i].exec_once = false;
    hsch_tasks[i].param = param;
    return i;
}

/**
 * 添加一个一次性任务
 * @param func 回调函数
 * @param para 函调函数的入参
 * @param co_cp 合作/抢占式
 */
u8 hsc_once_task(Action func, void *param, u8 co_cp)
{
    return hsch_add_task(func, param, 0, 0, co_cp);
}

/**
 * 删除任务
 * @param  i 任务ID
 * @return   删除状态
 */
bool hsch_delete_task(const u8 i)
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
    hsch_tasks[i].exec_once = false;
    hsch_tasks[i].param = NULL;

    return ret_code;
}

/**
 * 处理一些全局变量的更新
 */
void hsc_handle_update(void)
{
    if (soft_delay_count > 0)
    {
        soft_delay_count -= 1;
    }
    if (random_seeds == 0xFF)
    {
        random_seeds = 0;
    }
    random_seeds += 1;
}

/**
 * 执行调度任务扫描更新
 */
void exec_hsc_update(void)
{
    u8 i = 0;
    hsc_handle_update();
    for (i = 0; i < SCH_MAX_TASKS; i += 1)
    {
        if (hsch_tasks[i].func == NULL)
        {
            continue;
        }
        if (hsch_tasks[i].delay > 0)
        {
            hsch_tasks[i].delay -= 1;
            continue;
        }
        // 非周期任务已经执行了一次就不能再执行了
        if (hsch_tasks[i].exec_once && hsch_tasks[i].period == 0)
        {
            continue;
        }
        hsch_tasks[i].exec_once = true;
        //若是合作式则在中断中只置位,等待到hsch_dispatch_tasks中去执行
        if (hsch_tasks[i].co_cp)
        {
            ++hsch_tasks[i].runme;
        }
        else
        {
            // 抢占式任务直接执行
            hsch_tasks[i].func(hsch_tasks[i].param);
            if (hsch_tasks[i].period == 0)
            {
                hsch_delete_task(i);
            }
        }
        //如果是周期执行则将周期间隔赋予下次执行的延时
        hsch_tasks[i].delay = hsch_tasks[i].period > 0 ? hsch_tasks[i].period : 0;
    }
}

#ifdef SCH_TASK_TIMER2
/**
 * 更新调度器，周期由T2设置
 */
void hsch_update(void) interrupt TIMMER2_ITRP
{
    TF2 = 0;
    exec_hsc_update();
}

/**
 * 用Timmer2作为调度器的时钟源,默认1ms为周期
 */
void hsch_init_timmer2(void)
{
    u8 i = 0;
    for (i = 0; i < SCH_MAX_TASKS; i += 1)
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
#else
/**
 * 用 Timer1 作为调度器的时钟源，默认 1ms 为周期
 */
void hsch_init_timmer1(void)
{
    u8 i = 0;
    for (i = 0; i < SCH_MAX_TASKS; i += 1)
    {
        hsch_delete_task(i);
    }
    hsch_error_code = 0;
    TMOD &= 0x0F;
    TMOD |= 0x10; //16 位计数器模式
    ET1 = 1;
    TH1 = 0XFC;
    TL1 = 0X65;
    TF1 = 0;
    TR1 = 1;
}
/**
 * 更新调度器，周期由T1设置
 */
void hsch_update(void) interrupt TIMMER1_ITRP
{
    TF1 = 0;
    TH1 = 0XFC;
    TL1 = 0X65;
    exec_hsc_update();
}
#endif
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
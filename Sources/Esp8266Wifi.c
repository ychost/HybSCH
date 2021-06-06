#include "./Headers/Esp8266Wifi.h"
#include "./Headers/Bee.h"
#include "./Headers/Led.h"
#include <stdio.h>
#include <stdlib.h>

WifiResponse wifi_response;
/**
 * 清空 响应数据
 */
void wifi_response_clear()
{
    string_clear(wifi_response.bytes);
    wifi_response.bytes_size = 0;
}

/**
 * 通过定时器 2 来初始化 esp8266，波特率 115200
 */
void wifi_init_timer2()
{
    SCON = 0x50;
    TH2 = 0xFF;
    TL2 = 0xFD;
    RCAP2H = 0xFF;
    RCAP2L = 0xFD;
    TCLK = 1;
    RCLK = 1;
    C_T2 = 0;
    EXEN2 = 0;
    TR2 = 1;
    ES = 1;
}

/**
 * 发送命令
 * @param str 待发送命令
 */
void wifi_send_cmd(byte *send_cmd)
{
    while (*send_cmd != NULL)
    {
        SBUF = *send_cmd;
        while (!TI)
            ;
        TI = 0;
        RI = 0;
        send_cmd += 1;
    }
}

/**
 * wifi 返回的中断数据
 */
void wifi_receive_interrupt() interrupt UART_ITRP
{
    byte read_byte;
    if (RI == 1)
    {
        read_byte = SBUF;
        RI = 0;
        wifi_response.bytes[wifi_response.bytes_size] = read_byte;
        wifi_response.bytes_size += 1;
        if (wifi_response.bytes_size > WIFI_RESPONSE_CACHE_SIZE)
        {
            wifi_response.bytes_size = 0;
        }
    }
}

/**
 * wifi 执行命令
 * @param cmd AT 命令
 * @param resp_ok AT 命令响应 ok 的字符串
 * @param delay_ms 执行命令等待多久延迟才获取响应数据
 */
bool wifi_exec_cmd(char *cmd, char *resp_ok, u16 delay_ms)
{
    bool is_ok = false;
    wifi_response_clear();
    wifi_send_cmd(cmd);
    soft_delay_ms(delay_ms);
    is_ok = string_contains(wifi_response.bytes, resp_ok);
    wifi_response_clear();
    return is_ok;
}

/**
 * 连接指定的服务器，会自动进入透传模式
 * @param server 服务器地址
 */
bool wifi_connect_tcp_server(char code *server)
{
    bool is_ok = false;
    is_ok = wifi_exec_cmd(server, WIFI_RESP_OK, 11000);
    if (!is_ok)
    {
        // wifi 连接 tcp 服务器失败
        led_light_one(0);
    }
    // 进入透传模式
    is_ok &= wifi_enter_auto_trans();
    return is_ok;
}

/**
 * 进入透传模式
 */
bool wifi_enter_auto_trans()
{
    bool is_ok = false;
    is_ok = wifi_exec_cmd(WIFI_CMD_MODE_AUTO_TRANS, WIFI_RESP_OK, 1000);
    if (!is_ok)
    {
        // 设置透传模式失败
        led_light_one(3);
    }
    is_ok &= wifi_exec_cmd(WIFI_CMD_ENTER_AUTO_TRANS, WIFI_RESP_ENTERED_AUTO_TRANS, 1000);
    if (!is_ok)
    {
        // 开始传输设置失败
        led_light_one(5);
    }
    return is_ok;
}

/**
 * 透传模式下发送数据
 */
void wifi_auto_trans(char *package)
{
    wifi_send_cmd(package);
}

/**
 * 操作 influx post api
 * 
 * POST /write?db=mcu HTTP/1.1
 * Host: 192.168.50.11:8086
 * Content-Type:text/plain
 * Content-Length:59
 * 
 * weather,mcu=c51,measure_name=temperature,region=hz value=10
 * 
 * @param post 请求结构体
 */
void wifi_exec_post(void xdata *param)
{
    // 1. 优先取 post->body
    WifiPost xdata *post = (WifiPost *)param;
    bool body_clear = false;
    char *body = post->body;
    // sprintf 仅支持 int
    int content_len = 0;
    char content_len_str[3];
    //2. 其次取 influxdb 写入
    if (string_len(body) == 0)
    {
        body = wifi_influx_write_body(post->influx_db_measure_name, post->influx_db_measure_value);
        body_clear = true;
    }
    //3. 得出最终的 content_len
    content_len = string_len(body);
    sprintf(content_len_str, "%d", content_len);

    //拼接写入字符串
    wifi_auto_trans("POST ");
    wifi_auto_trans(post->path);
    wifi_auto_trans(" HTTP/1.1\r\n");
    wifi_auto_trans("Host: ");
    wifi_auto_trans(post->host);
    wifi_auto_trans("\r\n");
    // 可以忽略 content_type
    if (string_len(post->content_type) != 0)
    {
        wifi_auto_trans("Content-Type: ");
        wifi_auto_trans(post->content_type);
        wifi_auto_trans("\r\n");
    }
    wifi_auto_trans("Content-Length: ");
    wifi_auto_trans(content_len_str);
    wifi_auto_trans("\r\n\r\n");
    wifi_auto_trans(body);
    if (body_clear)
    {
        free(body);
    }
}

/**
 * 获取 influx 写入的 body
 */
char *wifi_influx_write_body(char *measure_name, char *value)
{
    char xdata body[WIFI_HTTP_BODY_MAX_LENGTH] = "embedded,mcu=c51,name=";
    string_append(body, measure_name);
    string_append(body, " value=");
    string_append(body, value);
    return body;
}

/**
 * wifi 首次初始化配置，仅需调用一次
 */
void wifi_first_init()
{

    // 1. 测试响应
    if (!wifi_exec_cmd(WIFI_CMD_TEST, WIFI_RESP_OK, 2000))
    {
        led_light_one(0);
        return;
    }

    // 2. 设置工作模式 STA+AP
    if (!wifi_exec_cmd(WIFI_CMD_MODE_STA, WIFI_RESP_OK, 2000))
    {
        led_light_one(3);
        return;
    }

    // 3. 连接路由器
    if (!wifi_exec_cmd(WIFI_CMD_CONNECT_ROUTER, WIFI_RESP_OK, 10000))
    {
        led_light_one(5);
        return;
    }

    // 4. 设置自动连接路由器
    if (!wifi_exec_cmd(WIFI_CMD_AUTO_CONN, WIFI_RESP_OK, 2000))
    {
        led_light_one(7);
        return;
    }
    // 5. 配置成功点亮所有 LED
    P1 = 0x00;
}
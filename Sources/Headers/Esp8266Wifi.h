#ifndef __ESP_8266_WIFI_H
#define __ESP_8266_WIFI_H
#include "./PreMacro.h"

/**
 * Esp8266 相关资料 https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_cn.pdf
 */

#ifndef WIFI_RESPONSE_CACHE_SIZE
#define WIFI_RESPONSE_CACHE_SIZE 32
#endif

#ifndef WIFI_CMD_CONNECT_ROUTER
#define WIFI_CMD_CONNECT_ROUTER "AT+CWJAP_DEF=\"SSID\",\"PASSWORD\"\r\n"
#endif

#ifndef WIFI_CMD_TCP_SERVER
#define WIFI_CMD_TCP_SERVER "AT+CIPSTART=\"TCP\",\"192.168.50.11\",8086\r\n"
#endif

// 测试响应
#define WIFI_CMD_TEST "AT\r\n"
// 持久化，STA + AP 模式
#define WIFI_CMD_MODE_STA "AT+CWMODE_DEF=3\r\n"
// 持久化，自动连接 AP
#define WIFI_CMD_AUTO_CONN "AT+CWAUTOCONN\r\n"
// 临时，透传模式
#define WIFI_CMD_MODE_AUTO_TRANS "AT+CIPMODE=1\r\n"
// 临时，进入透传
#define WIFI_CMD_ENTER_AUTO_TRANS "AT+CIPSEND\r\n"

// 响应，成功
#define WIFI_RESP_OK "OK"
// 响应，发送成功
#define WIFI_RESP_TCP_SEND_OK "SEND OK"
// 响应，进入透传模式
#define WIFI_RESP_ENTERED_AUTO_TRANS ">"

#define WIFI_HTTP_INFLUXDB_HOST "192.168.50.11:8086"
#define WIFI_HTTP_INFLUXDB_WRITE_PATH "/write?db=telegraf"
#define WIFI_HTTP_INFLUXDB_WRITE_BODY "weather,mcu=c51,measure_name=temperature,region=hz value=10"
#define WIFI_HTTP_CONTENT_TEXT_PLAIN "text/plain"
#define WIFI_HTTP_CONTENT_JSON "application/json"
#define WIFI_HTTP_BODY_MAX_LENGTH 128
/**
 * wifi 响应的数据
 */
typedef data struct
{
    /**
     * 响应的数据
     */
    char bytes[WIFI_RESPONSE_CACHE_SIZE];

    /**
     * 响应数据的长度
     */
    u8 bytes_size;
} WifiResponse;

/**
 * Post 请求结构体
 */
typedef xdata struct
{
    /**
     * 请求的服务器地址，比如 192.168.50.11:8086
     */
    char *host;
    /**
     * 请求路由
    */
    char *path;

    /**
     * body 内容，body 和 influx_xxx 二选一，优先 body
     */
    char *body;

    /**
     * 往 influxdb 写入的 name
     */
    char *influx_db_measure_name;

    /**
     * 往 influxDB 写入的 value
     */
    char *influx_db_measure_value;

    /**
     * text/plain  application/json 等等
     */
    char *content_type;
} WifiPost;

/**
 * 通过定时器 2 来初始化 esp8266
 * 115200 的波特率，仅支持定时器 2
 */
void wifi_init_timer2();

/**
 * 发送字符串
 * @param str 待发送的字符串
 */
void wifi_send_string(char *str);

/**
 * 以 STA 模式连接路由器
 * 
 * @return 连接是否成功
 */
bool wifi_connect_router();

/**
 * 连接指定的服务器，会自动进入透传模式
 * @param server 服务器地址
 */
bool wifi_connect_tcp_server(char code *server);

/**
 * 进入透传模式
 */
bool wifi_enter_auto_trans();

/**
 * 透传模式下发送数据
 */
void wifi_auto_trans(char *package);

/**
 * wifi 执行一次 Post
 * @param param 请求的结构体
 */
void wifi_exec_post(void xdata *param);

/**
 * 获取 influx 写入的 body
 */
char *wifi_influx_write_body(char *measure_name, char *value);

/**
 * WIFI 首次配置，仅需调用一次
 */
void wifi_first_init();

/**
 * 清空 响应数据
 */
void wifi_response_clear();

#endif
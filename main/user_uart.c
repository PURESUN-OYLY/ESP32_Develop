#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"

//UART1
#define RX1_BUF_SIZE (1024)
#define TX1_BUF_SIZE (512)
#define TXD1_PIN (GPIO_NUM_5)
#define RXD1_PIN (GPIO_NUM_4)

//UART2
#define RX2_BUF_SIZE (1024)
#define TX2_BUF_SIZE (512)
#define TXD2_PIN (GPIO_NUM_12)
#define RXD2_PIN (GPIO_NUM_13)

//Wifi
#define AP_ID "CSATA_001"
#define AP_PWD "CSATA2018"
#define WIFI_SSID "CSATA_HOST"
#define WIFI_PWD "CSATA2018"

static const char *TAG = "CSATA_001";
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

void uart_init(void);
void uart1_rx_task();
void uart2_rx_task();

// void TaskTest();
void Memory()
{
    while (1)
    {
        ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
        vTaskDelay(200);
    }
}

static esp_err_t event_handler(void *ctx, system_event_t *event);
static void WiFi_Start_AP();
static void WiFi_Start_STA();

bool ECHO_FLAG = false;

//用户程序入口点
void app_main()
{
    // TaskHandle_t testhandle;
    TaskHandle_t Memory_remain;
    //串口初始化
    uart_init();
    ESP_ERROR_CHECK(nvs_flash_init());                                                           //初始化NVS存储器
    xTaskCreate(uart1_rx_task, "uart1_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);     //Create Task Uart 1
    xTaskCreate(uart2_rx_task, "uart2_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL); //Create Task Uart 2
    xTaskCreate(Memory, "Memory remain", 2048, NULL, configMAX_PRIORITIES - 2, &Memory_remain);
    //xTaskCreate(TaskTest, "TaskTest", 2048, NULL, configMAX_PRIORITIES - 2, &testhandle);        //Creat Uart Test Task
    //Wifi_Start_AP();                                                                             //Open WiFi as a AP
    //WiFi_Start_STA();       //开启Station模式
    // vTaskDelay(1000);
    // if (testhandle != NULL)
    //     vTaskDelete(testhandle); //Delete Test Task
    // uart_write_bytes(UART_NUM_1, "Test Task is stoped.\r\n", strlen("Test Task is stoped.\r\n"));
}

// void TaskTest()
// {
//     while (1)
//     {
//         uart_write_bytes(UART_NUM_1, "TaskTest\r\n", strlen("TaskTest\r\n"));
//         vTaskDelay(100);
//     }
// }

void int2str(int num)
{
    char *temp;
    temp = (char *)malloc(sizeof(char));
    if (num < 0)
    {
        uart_write_bytes(UART_NUM_1, "-", 1);
        num = -num;
    }

    if (num >= 10000)
    {
        *temp = num / 10000 + '0';
        uart_write_bytes(UART_NUM_1, temp, 1);
    }
    if (num >= 1000)
    {
        *temp = num / 1000 % 10 + '0';
        uart_write_bytes(UART_NUM_1, temp, 1);
    }
    if (num >= 100)
    {
        *temp = num / 100 % 10 + '0';
        uart_write_bytes(UART_NUM_1, temp, 1);
    }
    if (num >= 10)
    {
        *temp = num / 100 % 10 + '0';
        uart_write_bytes(UART_NUM_1, temp, 1);
    }
    *temp = num % 10 + '0';
    uart_write_bytes(UART_NUM_1, temp, 1);
}

void uart_init(void)
{
    //串口配置结构体
    uart_config_t uart1_config, uart2_config;
    //串口参数配置->uart1
    uart1_config.baud_rate = 115200;                   //波特率
    uart1_config.data_bits = UART_DATA_8_BITS;         //数据位
    uart1_config.parity = UART_PARITY_DISABLE;         //校验位
    uart1_config.stop_bits = UART_STOP_BITS_1;         //停止位
    uart1_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE; //硬件流控
    uart_param_config(UART_NUM_1, &uart1_config);      //设置串口
    //IO映射-> T:IO4  R:IO5
    uart_set_pin(UART_NUM_1, TXD1_PIN, RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    //注册串口服务即使能+设置缓存区大小
    uart_driver_install(UART_NUM_1, RX1_BUF_SIZE * 2, TX1_BUF_SIZE * 2, 0, NULL, 0);

    //串口参数配置->uart2
    uart2_config.baud_rate = 115200;                   //波特率
    uart2_config.data_bits = UART_DATA_8_BITS;         //数据位
    uart2_config.parity = UART_PARITY_DISABLE;         //校验位
    uart2_config.stop_bits = UART_STOP_BITS_1;         //停止位
    uart2_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE; //硬件流控
    uart_param_config(UART_NUM_2, &uart2_config);      //设置串口
    //IO映射-> T:IO12  R:IO13
    uart_set_pin(UART_NUM_2, TXD2_PIN, RXD2_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    //注册串口服务即使能+设置缓存区大小
    uart_driver_install(UART_NUM_2, RX2_BUF_SIZE * 2, TX2_BUF_SIZE * 2, 0, NULL, 0);
}

void uart1_rx_task()
{
    uint8_t *data = (uint8_t *)malloc(RX1_BUF_SIZE + 1);
    while (1)
    {
        char *temp_str;
        //获取串口1接收的数据
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX1_BUF_SIZE, 10 / portTICK_RATE_MS);
        temp_str = (char *)malloc(sizeof(char) * (rxBytes + 1));
        if (rxBytes > 0)
        {
            strncpy(temp_str, (const char *)data, rxBytes);
            // uart_write_bytes(UART_NUM_1, temp_str, strlen(temp_str));
            // int2str(sizeof(temp_str));
            // int2str(sizeof(*temp_str));
            if (strncmp(temp_str, "Open Wifi AP\r\n", rxBytes) == 0)
            {
                uart_write_bytes(UART_NUM_1, "Opening WiFi AP...\r\n", strlen("Opening WiFi AP...\r\n"));
                // WiFi_Start_AP();
                tcpip_adapter_init();
                ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
                wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                ESP_ERROR_CHECK(esp_wifi_init(&cfg));
                wifi_config_t wifi_config = {
                    .ap = {
                        .ssid = AP_ID,
                        .ssid_len = 0,
                        /* 最多只能被4个station同时连接,这里设置为只能被一个station连接 */
                        .max_connection = 1,
                        .password = AP_PWD,
                        .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                    },
                };
                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP)); //Set WiFi mode as asscss point
                ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
                ESP_ERROR_CHECK(esp_wifi_start());
            }
            else if (strncmp(temp_str, "Open Wifi STA\r\n", rxBytes) == 0)
            {
                uart_write_bytes(UART_NUM_1, "Opening WiFi...\r\n", strlen("Opening WiFi...\r\n"));
                // WiFi_Start_STA();
                tcpip_adapter_init();
                wifi_event_group = xEventGroupCreate();
                ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
                wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                ESP_ERROR_CHECK(esp_wifi_init(&cfg));
                ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
                wifi_config_t wifi_config = {
                    .sta = {
                        .ssid = WIFI_SSID,    //要连接的热点
                        .password = WIFI_PWD, //目标WiFi密码
                    },
                };
                ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
                ESP_ERROR_CHECK(esp_wifi_start());
            }
            uart_write_bytes(UART_NUM_1, "Relase tem_str", 14);
            free(temp_str); //释放temp_str
            int2str(sizeof(temp_str));
        }
    }
    free(data);
}

void uart2_rx_task()
{
    uint8_t *data = (uint8_t *)malloc(RX2_BUF_SIZE + 1);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX2_BUF_SIZE, 10 / portTICK_RATE_MS);
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            uart_write_bytes(UART_NUM_2, (char *)data, rxBytes);
        }
    }
    free(data);
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_AP_START:
        printf("\nwifi_softap_start\n");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        printf("\nwifi_softap_connectted\n");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        printf("\nwifi_softap_disconnectted\n");
        break;
    case SYSTEM_EVENT_STA_START:
        printf("\nConnecting...\n");
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        printf("\nWiFi Got ip.\n");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        printf("\nDisConnected, Connecting...\n");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void WiFi_Start_AP()
{
    // ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_ID,
            .ssid_len = 0,
            /* 最多只能被4个station同时连接,这里设置为只能被一个station连接 */
            .max_connection = 1,
            .password = AP_PWD,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP)); //Set WiFi mode as asscss point
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void WiFi_Start_STA()
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,    //要连接的热点
            .password = WIFI_PWD, //目标WiFi密码
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

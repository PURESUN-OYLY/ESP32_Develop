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
#include "driver/ledc.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"

//UART1
#define RX1_BUF_SIZE 		(1024)
#define TX1_BUF_SIZE 		(512)
#define TXD1_PIN 			(GPIO_NUM_5)
#define RXD1_PIN 			(GPIO_NUM_4)

//UART2
#define RX2_BUF_SIZE 		(1024)
#define TX2_BUF_SIZE 		(512)
#define TXD2_PIN 			(GPIO_NUM_12)
#define RXD2_PIN 			(GPIO_NUM_13)

//Wifi
#define AP_ID       "CSATA_001"
#define AP_PWD      "CSATA2018"
#define WIFI_SSID   "CSATA_HOST"
#define WIFI_PWD    "CSATA_2018"

void uart_init(void);
void uart1_rx_task();
void uart2_rx_task();

void TaskTest();

static esp_err_t event_handler(void *ctx, system_event_t *event);
void Wifi_Start_AP();
void WiFi_Start_STA();

//用户程序入口点
void app_main()
{    
    TaskHandle_t testhandle;
	//串口初始化
	uart_init();
	xTaskCreate(uart1_rx_task, "uart1_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);      //Create Task Uart 1
	xTaskCreate(uart2_rx_task, "uart2_rx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);    //Create Task Uart 2
    xTaskCreate(TaskTest,"TaskTest",2048,NULL,configMAX_PRIORITIES-2,&testhandle);      //Creat Uart Test Task
    Wifi_Start_AP();        //Open WiFi as a AP
    vTaskDelay(1000);
    if(testhandle!=NULL)
        vTaskDelete(testhandle);  //Delete Test Task
    uart_write_bytes(UART_NUM_1,"Test Task is stoped.\r\n",strlen("Test Task is stoped.\r\n"));
}


void TaskTest()
{
    while(1)
    {
        uart_write_bytes(UART_NUM_1,"TaskTest\r\n",strlen("TaskTest\r\n"));
        vTaskDelay(100);
    }
}


void uart_init(void)
{
	//串口配置结构体
	uart_config_t uart1_config,uart2_config;
	//串口参数配置->uart1
	uart1_config.baud_rate = 115200;					//波特率
	uart1_config.data_bits = UART_DATA_8_BITS;			//数据位
	uart1_config.parity = UART_PARITY_DISABLE;			//校验位
	uart1_config.stop_bits = UART_STOP_BITS_1;			//停止位
	uart1_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;	//硬件流控
	uart_param_config(UART_NUM_1, &uart1_config);		//设置串口
	//IO映射-> T:IO4  R:IO5
	uart_set_pin(UART_NUM_1, TXD1_PIN, RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//注册串口服务即使能+设置缓存区大小
	uart_driver_install(UART_NUM_1, RX1_BUF_SIZE * 2, TX1_BUF_SIZE * 2, 0, NULL, 0);

	//串口参数配置->uart2
	uart2_config.baud_rate = 115200;					//波特率
	uart2_config.data_bits = UART_DATA_8_BITS;			//数据位
	uart2_config.parity = UART_PARITY_DISABLE;			//校验位
	uart2_config.stop_bits = UART_STOP_BITS_1;			//停止位
	uart2_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;	//硬件流控
	uart_param_config(UART_NUM_2, &uart2_config);		//设置串口
	//IO映射-> T:IO12  R:IO13
	uart_set_pin(UART_NUM_2, TXD2_PIN, RXD2_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//注册串口服务即使能+设置缓存区大小
	uart_driver_install(UART_NUM_2, RX2_BUF_SIZE * 2, TX2_BUF_SIZE * 2, 0, NULL, 0);


}

void uart1_rx_task()
{
    uint8_t* data = (uint8_t*) malloc(RX1_BUF_SIZE+1);
    while (1) {
        //获取串口1接收的数据
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX1_BUF_SIZE, 10 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;      //清除有效数据后面的一个数据位，防止对前面的数据产生影响
			//将接收到的数据发出去
			uart_write_bytes(UART_NUM_1, (char *)data, rxBytes);
        }
    }
    free(data);
}

void uart2_rx_task()
{
    uint8_t* data = (uint8_t*) malloc(RX2_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX2_BUF_SIZE, 10 / portTICK_RATE_MS);
        if (rxBytes > 0) {
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
    default:
        break;
    }
    return ESP_OK;
}


void Wifi_Start_AP()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
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
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

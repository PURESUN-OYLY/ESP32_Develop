// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"

// #include "esp_system.h"
// #include "esp_wifi.h"
// #include "esp_event_loop.h"
// #include "esp_log.h"

// #include "nvs_flash.h"

// #define EXAMPLE_WIFI_SSID "CSATA_HOST"
// #define EXAMPLE_WIFI_PASS "CSATA2018"

// static const char *TAG = "CSATA_001";

// /* FreeRTOS event group to signal when we are connected & ready to make a request */
// static EventGroupHandle_t wifi_event_group;

// /* The event group allows multiple bits for each event,
//    but we only care about one event - are we connected
//    to the AP with an IP? */
// const int CONNECTED_BIT = BIT0;

// static esp_err_t event_handler(void *ctx, system_event_t *event)
// {
//     switch (event->event_id)
//     {
//     case SYSTEM_EVENT_STA_START:
//         printf("Begin connect WiFi...\r\n");
//         esp_wifi_connect();
//         break;
//     case SYSTEM_EVENT_STA_GOT_IP:
//         printf("WiFi Got IP Adreess...\r\n");
//         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
//         break;
//     case SYSTEM_EVENT_STA_DISCONNECTED:
//         /* This is a workaround as ESP32 WiFi libs don't currently
//            auto-reassociate. */
//         printf("WiFi Disconnect...\r\n");
//         esp_wifi_connect();
//         xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
//         break;
//     default:
//         break;
//     }
//     return ESP_OK;
// }

// static void initialise_wifi(void)
// {
//     tcpip_adapter_init(); //初始化TCP/IP协议栈
//     wifi_event_group = xEventGroupCreate();
//     ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//     ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = EXAMPLE_WIFI_SSID,
//             .password = EXAMPLE_WIFI_PASS,
//         },
//     };
//     ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
//     printf("WiFi Station mode Setting...\r\n");
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     printf("WiFi Info Configing...\r\n");
//     ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
//     printf("Open WiFi...\r\n");
//     ESP_ERROR_CHECK(esp_wifi_start());
// }

// void app_main()
// {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     initialise_wifi();
// }
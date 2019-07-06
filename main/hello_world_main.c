// /****************************/
// //    ESP32 Develop
// //    Date 20190629
// //    Developer:PURESUN
// //
// /****************************/
// #include "stdio.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_system.h"
// #include "esp_spi_flash.h"
// #include "esp_wifi.h"
// #include "connet_wifi.h"
// #include "driver/uart.h"
// #include "driver/gpio.h"

// #define WiFi_HOST "CSATA_HOST"
// #define WiFi_PWD "CSATA2018"

// //UART data buf size is 2 KiB
// const char uart_buf[2048];
// void IRAM_ATTR uart2_irq_handler(void *arg);
// void uart2_init(void);

// void app_main()
// {
//     printf("Hello world!\n");
//     printf("this is the second time build\r\n");

//     /* Print chip information */
//     esp_chip_info_t chip_info;
//     esp_chip_info(&chip_info);
//     printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
//             chip_info.cores,
//             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
//             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

//     printf("silicon revision %d, ", chip_info.revision);

//     printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
//             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

//     for (int i = 5; i >= 0; i--) {
//         printf("Restarting in %d seconds...\r", i);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
//     //uart2_init();
//     printf("Restarting now.\n");
//     //The VS Code Editor will report errors, but this is OK
//     //This code line will print all data in stdout on screen
//     //fflush(stdout);
//     esp_restart();
// }

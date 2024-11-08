#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"
#include "esp_spiffs.h"
#include "esp_http_client.h"
#include "esp_attr.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_http_server.h"
#include "esp_mac.h"
#include "esp_chip_info.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/spi_master.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_hal.h"

static const char *TAG = "MAIN";

void app_main()
{   
    ESP_ERROR_CHECK( nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    hal_setup();

    hal_loop();

    return 0;
}


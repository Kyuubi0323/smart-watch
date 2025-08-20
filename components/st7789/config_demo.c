/*
 * ESP32-C3 Configuration Verification
 * This file demonstrates that the configuration system is working correctly
 */

#include "esp_log.h"
#include "../custominclude/main.h"

static const char* TAG = "CONFIG_TEST";

void print_display_config(void) {
    ESP_LOGI(TAG, "=== Display Configuration ===");
    ESP_LOGI(TAG, "Resolution: %dx%d", WIDTH, HEIGHT);
    ESP_LOGI(TAG, "Display Offset: X=%d, Y=%d", OFFSET_X, OFFSET_Y);
    ESP_LOGI(TAG, "RGB Order: %s", RGB_ORDER ? "RGB" : "BGR");
    
    ESP_LOGI(TAG, "=== Pin Configuration ===");
    ESP_LOGI(TAG, "SCLK: GPIO %d", SCLK);
    ESP_LOGI(TAG, "MOSI: GPIO %d", MOSI);
    ESP_LOGI(TAG, "DC: GPIO %d", DC);
    ESP_LOGI(TAG, "CS: GPIO %d", CS);
    ESP_LOGI(TAG, "RST: GPIO %d", RST);
    ESP_LOGI(TAG, "BL: GPIO %d", BL);
    
    ESP_LOGI(TAG, "=== Other Settings ===");
    ESP_LOGI(TAG, "SPI Host: %d", SPI);
    ESP_LOGI(TAG, "Max File Open: %d", MAX_FILE_OPEN);
    
#ifdef ESPC3_1_69
    ESP_LOGI(TAG, "Active Configuration: ESP32-C3 1.69 inch LCD");
#elif defined(ESPC3)
    ESP_LOGI(TAG, "Active Configuration: ESP32-C3 1.28 inch LCD");
#elif defined(ESPS3_1_69)  
    ESP_LOGI(TAG, "Active Configuration: ESP32-S3 1.69 inch LCD");
#elif defined(ESPS3_1_28)
    ESP_LOGI(TAG, "Active Configuration: ESP32-S3 1.28 inch LCD");
#elif defined(ELECROW_C3)
    ESP_LOGI(TAG, "Active Configuration: ElecRow ESP32-C3");
#else
    ESP_LOGI(TAG, "Active Configuration: Default/Unknown");
#endif
}
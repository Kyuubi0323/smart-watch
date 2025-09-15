/**
 * @file example_usage.c
 * @brief Example usage of QMI8658C component
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "qmi8658c.h"

#define I2C_MASTER_SCL_IO           22    /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21    /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0     /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0     /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0     /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void app_main(void)
{
    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI("MAIN", "I2C initialized successfully");

    // Example 1: Using the global instance (Arduino-style)
    ESP_LOGI("MAIN", "=== QMI8658C Global Instance Example ===");
    
    // Initialize with default configuration
    esp_err_t ret = qmi8658c_init_default(I2C_MASTER_NUM);
    if (ret != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize QMI8658C: %s", esp_err_to_name(ret));
        return;
    }

    // Test WHO_AM_I using global instance
    uint8_t who_am_i;
    ret = qmi8658c.read_who_am_i(&qmi8658c, &who_am_i);
    if (ret == ESP_OK) {
        ESP_LOGI("MAIN", "WHO_AM_I: 0x%02X", who_am_i);
    }

    // Read temperature
    float temperature;
    ret = qmi8658c.read_temperature(&qmi8658c, &temperature);
    if (ret == ESP_OK) {
        ESP_LOGI("MAIN", "Temperature: %.2f°C", temperature);
    }

    // Calibrate gyroscope
    ESP_LOGI("MAIN", "Calibrating gyroscope (keep sensor still)...");
    ret = qmi8658c.calibrate_gyro(&qmi8658c, 100);
    if (ret == ESP_OK) {
        ESP_LOGI("MAIN", "Gyroscope calibration completed");
    }

    // Read sensor data for 10 seconds
    ESP_LOGI("MAIN", "Reading sensor data...");
    for (int i = 0; i < 100; i++) {
        qmi8658c_scaled_data_t acc_data, gyro_data;
        
        ret = qmi8658c.read_all_scaled(&qmi8658c, &acc_data, &gyro_data);
        if (ret == ESP_OK) {
            ESP_LOGI("MAIN", "ACC: X=%.3fg Y=%.3fg Z=%.3fg | GYRO: X=%.1f°/s Y=%.1f°/s Z=%.1f°/s",
                     acc_data.x, acc_data.y, acc_data.z,
                     gyro_data.x, gyro_data.y, gyro_data.z);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Example 2: Using custom instance
    ESP_LOGI("MAIN", "=== QMI8658C Custom Instance Example ===");
    
    qmi8658c_t my_sensor;
    qmi8658c_create(&my_sensor);
    
    // Create custom configuration
    qmi8658c_config_t config = {
        .i2c_port = I2C_MASTER_NUM,
        .i2c_address = QMI8658C_I2C_ADDR_DEFAULT,
        .acc_range = QMI8658C_ACC_RANGE_4G,
        .gyro_range = QMI8658C_GYRO_RANGE_512DPS,
        .acc_odr = QMI8658C_ODR_250HZ,
        .gyro_odr = QMI8658C_ODR_250HZ,
        .enable_acc = true,
        .enable_gyro = true
    };
    
    ret = my_sensor.init(&my_sensor, &config);
    if (ret != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize custom sensor: %s", esp_err_to_name(ret));
        return;
    }

    // Test self-test
    bool acc_test_pass, gyro_test_pass;
    ret = my_sensor.self_test(&my_sensor, &acc_test_pass, &gyro_test_pass);
    if (ret == ESP_OK) {
        ESP_LOGI("MAIN", "Self test - ACC: %s, GYRO: %s", 
                 acc_test_pass ? "PASS" : "FAIL",
                 gyro_test_pass ? "PASS" : "FAIL");
    }

    // Change accelerometer range
    ESP_LOGI("MAIN", "Changing accelerometer range to ±8g");
    ret = my_sensor.set_acc_range(&my_sensor, QMI8658C_ACC_RANGE_8G);
    if (ret == ESP_OK) {
        ESP_LOGI("MAIN", "Accelerometer range changed successfully");
    }

    // Read some data with new range
    ESP_LOGI("MAIN", "Reading data with new range...");
    for (int i = 0; i < 10; i++) {
        qmi8658c_scaled_data_t acc_data;
        
        ret = my_sensor.read_acc_scaled(&my_sensor, &acc_data);
        if (ret == ESP_OK) {
            ESP_LOGI("MAIN", "ACC (±8g): X=%.3fg Y=%.3fg Z=%.3fg",
                     acc_data.x, acc_data.y, acc_data.z);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Example 3: Using standalone functions (backward compatibility)
    ESP_LOGI("MAIN", "=== QMI8658C Standalone Functions Example ===");
    
    // Deinitialize the custom instance first
    my_sensor.deinit(&my_sensor);
    
    // Use standalone functions
    qmi8658c_config_t standalone_config = QMI8658C_DEFAULT_CONFIG();
    standalone_config.i2c_port = I2C_MASTER_NUM;
    standalone_config.acc_range = QMI8658C_ACC_RANGE_16G;
    standalone_config.gyro_range = QMI8658C_GYRO_RANGE_1024DPS;
    
    ret = qmi8658c_init_with_config(&standalone_config);
    if (ret != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize with standalone functions: %s", esp_err_to_name(ret));
        return;
    }

    // Test standalone functions
    ret = qmi8658c_read_who_am_i(&who_am_i);
    if (ret == ESP_OK) {
        ESP_LOGI("MAIN", "WHO_AM_I (standalone): 0x%02X", who_am_i);
    }

    // Read data using standalone functions
    ESP_LOGI("MAIN", "Reading data with standalone functions...");
    for (int i = 0; i < 10; i++) {
        qmi8658c_raw_data_t raw_acc, raw_gyro;
        qmi8658c_scaled_data_t scaled_acc, scaled_gyro;
        
        ret = qmi8658c_read_acc_raw(&raw_acc);
        if (ret == ESP_OK) {
            ESP_LOGI("MAIN", "ACC Raw: X=%d Y=%d Z=%d", raw_acc.x, raw_acc.y, raw_acc.z);
        }
        
        ret = qmi8658c_read_acc_scaled(&scaled_acc);
        if (ret == ESP_OK) {
            ESP_LOGI("MAIN", "ACC Scaled: X=%.3fg Y=%.3fg Z=%.3fg", 
                     scaled_acc.x, scaled_acc.y, scaled_acc.z);
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // Calibrate gyroscope using standalone function
    ESP_LOGI("MAIN", "Calibrating gyroscope (standalone)...");
    ret = qmi8658c_calibrate_gyro(50);
    if (ret == ESP_OK) {
        ESP_LOGI("MAIN", "Gyroscope calibration completed (standalone)");
    }

    // Read gyroscope data after calibration
    ESP_LOGI("MAIN", "Reading calibrated gyroscope data...");
    for (int i = 0; i < 10; i++) {
        qmi8658c_scaled_data_t gyro_data;
        
        ret = qmi8658c_read_gyro_scaled(&gyro_data);
        if (ret == ESP_OK) {
            ESP_LOGI("MAIN", "GYRO Calibrated: X=%.1f°/s Y=%.1f°/s Z=%.1f°/s", 
                     gyro_data.x, gyro_data.y, gyro_data.z);
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // Clean up
    qmi8658c_deinit();
    ESP_LOGI("MAIN", "QMI8658C example completed");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

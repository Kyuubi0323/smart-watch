/**
 * @file qmi8658c.h
 * @brief ESP-IDF QMI8658C Accelerometer/Gyroscope Component
 * @author Your Name
 * @date 2025
 */

#ifndef _QMI8658C_H_
#define _QMI8658C_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

// QMI8658C I2C Address
#define QMI8658C_I2C_ADDR_DEFAULT   0x6B
#define QMI8658C_I2C_ADDR_ALT       0x6A

// QMI8658C Register Addresses
#define QMI8658C_REG_WHO_AM_I       0x00
#define QMI8658C_REG_REVISION       0x01
#define QMI8658C_REG_CTRL1          0x02
#define QMI8658C_REG_CTRL2          0x03
#define QMI8658C_REG_CTRL3          0x04
#define QMI8658C_REG_CTRL4          0x05
#define QMI8658C_REG_CTRL5          0x06
#define QMI8658C_REG_CTRL6          0x07
#define QMI8658C_REG_CTRL7          0x08
#define QMI8658C_REG_CTRL9          0x0A
#define QMI8658C_REG_TEMP_L         0x33
#define QMI8658C_REG_TEMP_H         0x34
#define QMI8658C_REG_AX_L           0x35
#define QMI8658C_REG_AX_H           0x36
#define QMI8658C_REG_AY_L           0x37
#define QMI8658C_REG_AY_H           0x38
#define QMI8658C_REG_AZ_L           0x39
#define QMI8658C_REG_AZ_H           0x3A
#define QMI8658C_REG_GX_L           0x3B
#define QMI8658C_REG_GX_H           0x3C
#define QMI8658C_REG_GY_L           0x3D
#define QMI8658C_REG_GY_H           0x3E
#define QMI8658C_REG_GZ_L           0x3F
#define QMI8658C_REG_GZ_H           0x40
#define QMI8658C_REG_RESET          0x60

// QMI8658C Constants
#define QMI8658C_CHIP_ID            0x05

// Scale sensitivity constants (from Arduino library)
#define QMI8658C_ACC_SCALE_SENSITIVITY_2G        (1 << 14)  // 16384
#define QMI8658C_ACC_SCALE_SENSITIVITY_4G        (1 << 13)  // 8192
#define QMI8658C_ACC_SCALE_SENSITIVITY_8G        (1 << 12)  // 4096
#define QMI8658C_ACC_SCALE_SENSITIVITY_16G       (1 << 11)  // 2048

#define QMI8658C_GYRO_SCALE_SENSITIVITY_16DPS    (1 << 11)  // 2048
#define QMI8658C_GYRO_SCALE_SENSITIVITY_32DPS    (1 << 10)  // 1024
#define QMI8658C_GYRO_SCALE_SENSITIVITY_64DPS    (1 << 9)   // 512
#define QMI8658C_GYRO_SCALE_SENSITIVITY_128DPS   (1 << 8)   // 256
#define QMI8658C_GYRO_SCALE_SENSITIVITY_256DPS   (1 << 7)   // 128
#define QMI8658C_GYRO_SCALE_SENSITIVITY_512DPS   (1 << 6)   // 64
#define QMI8658C_GYRO_SCALE_SENSITIVITY_1024DPS  (1 << 5)   // 32
#define QMI8658C_GYRO_SCALE_SENSITIVITY_2048DPS  (1 << 4)   // 16

#define QMI8658C_TEMPERATURE_SENSOR_RESOLUTION   (1 << 8)   // 256

/**
 * @brief Accelerometer full-scale range options
 */
typedef enum {
    QMI8658C_ACC_RANGE_2G = 0,      ///< ±2g
    QMI8658C_ACC_RANGE_4G,          ///< ±4g
    QMI8658C_ACC_RANGE_8G,          ///< ±8g
    QMI8658C_ACC_RANGE_16G          ///< ±16g
} qmi8658c_acc_range_t;

/**
 * @brief Gyroscope full-scale range options
 */
typedef enum {
    QMI8658C_GYRO_RANGE_16DPS = 0,  ///< ±16 degrees per second
    QMI8658C_GYRO_RANGE_32DPS,      ///< ±32 degrees per second
    QMI8658C_GYRO_RANGE_64DPS,      ///< ±64 degrees per second
    QMI8658C_GYRO_RANGE_128DPS,     ///< ±128 degrees per second
    QMI8658C_GYRO_RANGE_256DPS,     ///< ±256 degrees per second
    QMI8658C_GYRO_RANGE_512DPS,     ///< ±512 degrees per second
    QMI8658C_GYRO_RANGE_1024DPS,    ///< ±1024 degrees per second
    QMI8658C_GYRO_RANGE_2048DPS     ///< ±2048 degrees per second
} qmi8658c_gyro_range_t;

/**
 * @brief Output Data Rate (ODR) options for accelerometer
 */
typedef enum {
    QMI8658C_ACC_ODR_8000HZ = 0,    ///< 8000 Hz
    QMI8658C_ACC_ODR_4000HZ,        ///< 4000 Hz
    QMI8658C_ACC_ODR_2000HZ,        ///< 2000 Hz
    QMI8658C_ACC_ODR_1000HZ,        ///< 1000 Hz
    QMI8658C_ACC_ODR_500HZ,         ///< 500 Hz
    QMI8658C_ACC_ODR_250HZ,         ///< 250 Hz
    QMI8658C_ACC_ODR_125HZ,         ///< 125 Hz
    QMI8658C_ACC_ODR_62_5HZ,        ///< 62.5 Hz
    QMI8658C_ACC_ODR_31_25HZ,       ///< 31.25 Hz
    QMI8658C_ACC_ODR_128HZ = 12,    ///< 128 Hz
    QMI8658C_ACC_ODR_21HZ,          ///< 21 Hz
    QMI8658C_ACC_ODR_11HZ,          ///< 11 Hz
    QMI8658C_ACC_ODR_3HZ            ///< 3 Hz
} qmi8658c_acc_odr_t;
    
/**
 * @brief Output Data Rate (ODR) options for gyroscope
 */
typedef enum {
    QMI8658C_GYRO_ODR_8000HZ = 0,   ///< 8000 Hz
    QMI8658C_GYRO_ODR_4000HZ,       ///< 4000 Hz
    QMI8658C_GYRO_ODR_2000HZ,       ///< 2000 Hz
    QMI8658C_GYRO_ODR_1000HZ,       ///< 1000 Hz
    QMI8658C_GYRO_ODR_500HZ,        ///< 500 Hz
    QMI8658C_GYRO_ODR_250HZ,        ///< 250 Hz
    QMI8658C_GYRO_ODR_125HZ,        ///< 125 Hz
    QMI8658C_GYRO_ODR_62_5HZ,       ///< 62.5 Hz
    QMI8658C_GYRO_ODR_31_25HZ       ///< 31.25 Hz
} qmi8658c_gyro_odr_t;

/**
 * @brief Operating mode options
 */
typedef enum {
    QMI8658C_MODE_ACC_ONLY = 1,     ///< Accelerometer only
    QMI8658C_MODE_GYRO_ONLY,        ///< Gyroscope only
    QMI8658C_MODE_DUAL              ///< Both accelerometer and gyroscope
} qmi8658c_mode_t;

/**
 * @brief Data structure for raw sensor readings
 */
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} qmi8658c_raw_data_t;

/**
 * @brief Data structure for scaled sensor readings
 */
typedef struct {
    float x;
    float y;
    float z;
} qmi8658c_scaled_data_t;

/**
 * @brief Configuration structure for QMI8658C
 */
typedef struct {
    i2c_port_t i2c_port;               ///< I2C port number
    uint8_t i2c_address;               ///< I2C device address
    qmi8658c_mode_t mode;              ///< Operating mode
    qmi8658c_acc_range_t acc_range;    ///< Accelerometer range
    qmi8658c_gyro_range_t gyro_range;  ///< Gyroscope range
    qmi8658c_acc_odr_t acc_odr;        ///< Accelerometer ODR
    qmi8658c_gyro_odr_t gyro_odr;      ///< Gyroscope ODR
} qmi8658c_config_t;

// Forward declaration
typedef struct qmi8658c qmi8658c_t;

/**
 * @brief QMI8658C structure with function pointers (C-style object)
 */
struct qmi8658c {
    // Private members
    qmi8658c_config_t config;
    uint16_t acc_sensitivity;
    uint16_t gyro_sensitivity;
    bool initialized;
    
    // Function pointers (methods)
    esp_err_t (*init)(qmi8658c_t* self, const qmi8658c_config_t* config);
    esp_err_t (*deinit)(qmi8658c_t* self);
    esp_err_t (*reset)(qmi8658c_t* self);
    esp_err_t (*set_mode)(qmi8658c_t* self, qmi8658c_mode_t mode);
    esp_err_t (*set_acc_range)(qmi8658c_t* self, qmi8658c_acc_range_t range);
    esp_err_t (*set_gyro_range)(qmi8658c_t* self, qmi8658c_gyro_range_t range);
    esp_err_t (*set_acc_odr)(qmi8658c_t* self, qmi8658c_acc_odr_t odr);
    esp_err_t (*set_gyro_odr)(qmi8658c_t* self, qmi8658c_gyro_odr_t odr);
    
    esp_err_t (*read_who_am_i)(qmi8658c_t* self, uint8_t* who_am_i);
    esp_err_t (*read_revision)(qmi8658c_t* self, uint8_t* revision);
    esp_err_t (*read_temperature)(qmi8658c_t* self, float* temperature);
    esp_err_t (*read_acc_raw)(qmi8658c_t* self, qmi8658c_raw_data_t* data);
    esp_err_t (*read_gyro_raw)(qmi8658c_t* self, qmi8658c_raw_data_t* data);
    esp_err_t (*read_acc_scaled)(qmi8658c_t* self, qmi8658c_scaled_data_t* data);
    esp_err_t (*read_gyro_scaled)(qmi8658c_t* self, qmi8658c_scaled_data_t* data);
    esp_err_t (*read_all_raw)(qmi8658c_t* self, qmi8658c_raw_data_t* acc_data, qmi8658c_raw_data_t* gyro_data);
    esp_err_t (*read_all_scaled)(qmi8658c_t* self, qmi8658c_scaled_data_t* acc_data, qmi8658c_scaled_data_t* gyro_data);
    
    esp_err_t (*calibrate_gyro)(qmi8658c_t* self, uint16_t samples);
    esp_err_t (*self_test)(qmi8658c_t* self, bool* acc_test_pass, bool* gyro_test_pass);
};

/**
 * @brief Initialize a QMI8658C instance
 * @param qmi8658c_instance Pointer to QMI8658C instance to initialize
 */
void qmi8658c_create(qmi8658c_t* qmi8658c_instance);

/**
 * @brief Global QMI8658C instance
 */
extern qmi8658c_t qmi8658c;

/**
 * @brief Initialize the QMI8658C component with default configuration
 * @param i2c_port I2C port number
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_init_default(i2c_port_t i2c_port);

// ============================================================================
// Standalone functions (for backward compatibility)
// ============================================================================

/**
 * @brief Initialize QMI8658C with configuration
 * @param config Configuration structure
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_init_with_config(const qmi8658c_config_t* config);

/**
 * @brief Deinitialize QMI8658C
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_deinit(void);

/**
 * @brief Read WHO_AM_I register
 * @param who_am_i Pointer to store WHO_AM_I value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_read_who_am_i(uint8_t* who_am_i);

/**
 * @brief Read temperature
 * @param temperature Pointer to store temperature in Celsius
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_read_temperature(float* temperature);

/**
 * @brief Read accelerometer data (raw)
 * @param data Pointer to store raw accelerometer data
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_read_acc_raw(qmi8658c_raw_data_t* data);

/**
 * @brief Read gyroscope data (raw)
 * @param data Pointer to store raw gyroscope data
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_read_gyro_raw(qmi8658c_raw_data_t* data);

/**
 * @brief Read accelerometer data (scaled in g)
 * @param data Pointer to store scaled accelerometer data
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_read_acc_scaled(qmi8658c_scaled_data_t* data);

/**
 * @brief Read gyroscope data (scaled in degrees per second)
 * @param data Pointer to store scaled gyroscope data
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_read_gyro_scaled(qmi8658c_scaled_data_t* data);

/**
 * @brief Set accelerometer range
 * @param range Accelerometer range
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_set_acc_range(qmi8658c_acc_range_t range);

/**
 * @brief Set gyroscope range
 * @param range Gyroscope range
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_set_gyro_range(qmi8658c_gyro_range_t range);

/**
 * @brief Set accelerometer ODR
 * @param odr Accelerometer ODR
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_set_acc_odr(qmi8658c_acc_odr_t odr);

/**
 * @brief Set gyroscope ODR
 * @param odr Gyroscope ODR
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_set_gyro_odr(qmi8658c_gyro_odr_t odr);

/**
 * @brief Set operating mode
 * @param mode Operating mode
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_set_mode(qmi8658c_mode_t mode);

/**
 * @brief Read revision ID register
 * @param revision Pointer to store revision ID value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_read_revision(uint8_t* revision);

/**
 * @brief Calibrate gyroscope (calculate offset)
 * @param samples Number of samples for calibration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t qmi8658c_calibrate_gyro(uint16_t samples);

// Convenience macros
#define QMI8658C_DEFAULT_CONFIG() { \
    .i2c_port = I2C_NUM_0, \
    .i2c_address = QMI8658C_I2C_ADDR_DEFAULT, \
    .mode = QMI8658C_MODE_DUAL, \
    .acc_range = QMI8658C_ACC_RANGE_8G, \
    .gyro_range = QMI8658C_GYRO_RANGE_64DPS, \
    .acc_odr = QMI8658C_ACC_ODR_1000HZ, \
    .gyro_odr = QMI8658C_GYRO_ODR_125HZ \
}

#ifdef __cplusplus
}
#endif

#endif // _QMI8658C_H_

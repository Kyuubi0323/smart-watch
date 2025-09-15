# QMI8658C ESP-IDF Component

A comprehensive ESP-IDF component for the QMI8658C 6-axis accelerometer and gyroscope sensor with C struct function pointers, following the same design pattern as the slog component.

## Features
- **Dual API Support**: 
  - Object-oriented approach with custom instances
  - Global instance for simple usage
  - Standalone functions for backward compatibility
- **Full QMI8658C Support**:
  - 3-axis accelerometer with configurable range (±2g, ±4g, ±8g, ±16g)
  - 3-axis gyroscope with configurable range (±16°/s to ±2048°/s)
  - Configurable Output Data Rate (ODR)
  - Temperature sensor
  - Gyroscope calibration
  - Self-test functionality
- **I2C Communication**: Efficient I2C communication with error handling
- **ESP-IDF Integration**: Fully integrated with ESP-IDF framework

## Hardware Connection

| QMI8658C Pin | ESP32 Pin | Description |
|--------------|-----------|-------------|
| VDD          | 3.3V      | Power supply |
| GND          | GND       | Ground |
| SCL          | GPIO22    | I2C Clock (configurable) |
| SDA          | GPIO21    | I2C Data (configurable) |
| CS           | 3.3V      | I2C mode (pull high) |
| SA0          | GND/3.3V  | I2C Address select |

## Installation

### Method 1: ESP Component Manager

Add to your project's `idf_component.yml`:

```yaml
dependencies:
  qmi8658c:
    git: https://github.com/Kyuubi0323/qmi8658c
```

### Method 2: Git Submodule

```bash
cd your_project/components
git submodule add https://github.com/Kyuubi0323/qmi8658c
```

### Method 3: Manual Copy

Copy the entire `qmi8658c` folder to your project's `components` directory.

## Usage Examples

### 1. Global Instance 

```c
#include "qmi8658c.h"

void app_main(void)
{
    // Initialize I2C first
    // ... I2C initialization code ...
    
    // Initialize QMI8658C with default configuration
    esp_err_t ret = qmi8658c_init_default(I2C_NUM_0);
    if (ret != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize QMI8658C");
        return;
    }
    
    // Read sensor data
    qmi8658c_scaled_data_t acc_data, gyro_data;
    ret = qmi8658c.read_all_scaled(&qmi8658c, &acc_data, &gyro_data);
    if (ret == ESP_OK) {
        printf("ACC: X=%.3fg Y=%.3fg Z=%.3fg\n", acc_data.x, acc_data.y, acc_data.z);
        printf("GYRO: X=%.1f°/s Y=%.1f°/s Z=%.1f°/s\n", gyro_data.x, gyro_data.y, gyro_data.z);
    }
}
```

### 2. Custom Instance

```c
#include "qmi8658c.h"

void app_main(void)
{
    // Create and initialize custom instance
    qmi8658c_t my_sensor;
    qmi8658c_create(&my_sensor);
    
    // Custom configuration
    qmi8658c_config_t config = {
        .i2c_port = I2C_NUM_0,
        .i2c_address = QMI8658C_I2C_ADDR_DEFAULT,
        .acc_range = QMI8658C_ACC_RANGE_4G,
        .gyro_range = QMI8658C_GYRO_RANGE_512DPS,
        .acc_odr = QMI8658C_ODR_250HZ,
        .gyro_odr = QMI8658C_ODR_250HZ,
        .enable_acc = true,
        .enable_gyro = true
    };
    
    esp_err_t ret = my_sensor.init(&my_sensor, &config);
    if (ret != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize sensor");
        return;
    }
    
    // Calibrate gyroscope
    ret = my_sensor.calibrate_gyro(&my_sensor, 100);
    if (ret == ESP_OK) {
        ESP_LOGI("MAIN", "Gyroscope calibrated");
    }
    
    // Change accelerometer range
    my_sensor.set_acc_range(&my_sensor, QMI8658C_ACC_RANGE_8G);
    
    // Read temperature
    float temperature;
    my_sensor.read_temperature(&my_sensor, &temperature);
    printf("Temperature: %.2f°C\n", temperature);
    
    // Self test
    bool acc_test_pass, gyro_test_pass;
    my_sensor.self_test(&my_sensor, &acc_test_pass, &gyro_test_pass);
    printf("Self test - ACC: %s, GYRO: %s\n", 
           acc_test_pass ? "PASS" : "FAIL",
           gyro_test_pass ? "PASS" : "FAIL");
    
    // Clean up
    my_sensor.deinit(&my_sensor);
}
```

### 3. Standalone Functions (Backward Compatibility)

```c
#include "qmi8658c.h"

void app_main(void)
{
    // Initialize with configuration
    qmi8658c_config_t config = QMI8658C_DEFAULT_CONFIG();
    config.i2c_port = I2C_NUM_0;
    
    esp_err_t ret = qmi8658c_init_with_config(&config);
    if (ret != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize");
        return;
    }
    
    // Read WHO_AM_I
    uint8_t who_am_i;
    qmi8658c_read_who_am_i(&who_am_i);
    printf("WHO_AM_I: 0x%02X\n", who_am_i);
    
    // Read raw data
    qmi8658c_raw_data_t raw_acc;
    qmi8658c_read_acc_raw(&raw_acc);
    printf("Raw ACC: X=%d Y=%d Z=%d\n", raw_acc.x, raw_acc.y, raw_acc.z);
    
    // Read scaled data
    qmi8658c_scaled_data_t scaled_acc;
    qmi8658c_read_acc_scaled(&scaled_acc);
    printf("Scaled ACC: X=%.3fg Y=%.3fg Z=%.3fg\n", scaled_acc.x, scaled_acc.y, scaled_acc.z);
    
    // Clean up
    qmi8658c_deinit();
}
```

## API Reference

### Configuration Structure

```c
typedef struct {
    i2c_port_t i2c_port;               // I2C port number
    uint8_t i2c_address;               // I2C device address
    qmi8658c_acc_range_t acc_range;    // Accelerometer range
    qmi8658c_gyro_range_t gyro_range;  // Gyroscope range
    qmi8658c_odr_t acc_odr;            // Accelerometer ODR
    qmi8658c_odr_t gyro_odr;           // Gyroscope ODR
    bool enable_acc;                   // Enable accelerometer
    bool enable_gyro;                  // Enable gyroscope
} qmi8658c_config_t;
```

### Data Structures

```c
// Raw 16-bit sensor data
typedef struct {
    int16_t x, y, z;
} qmi8658c_raw_data_t;

// Scaled floating-point sensor data
typedef struct {
    float x, y, z;
} qmi8658c_scaled_data_t;
```

### Object-Oriented Methods

```c
// Configuration methods
esp_err_t (*init)(qmi8658c_t* self, const qmi8658c_config_t* config);
esp_err_t (*deinit)(qmi8658c_t* self);
esp_err_t (*reset)(qmi8658c_t* self);
esp_err_t (*set_acc_range)(qmi8658c_t* self, qmi8658c_acc_range_t range);
esp_err_t (*set_gyro_range)(qmi8658c_t* self, qmi8658c_gyro_range_t range);

// Data reading methods
esp_err_t (*read_acc_raw)(qmi8658c_t* self, qmi8658c_raw_data_t* data);
esp_err_t (*read_gyro_raw)(qmi8658c_t* self, qmi8658c_raw_data_t* data);
esp_err_t (*read_acc_scaled)(qmi8658c_t* self, qmi8658c_scaled_data_t* data);
esp_err_t (*read_gyro_scaled)(qmi8658c_t* self, qmi8658c_scaled_data_t* data);
esp_err_t (*read_all_scaled)(qmi8658c_t* self, qmi8658c_scaled_data_t* acc_data, qmi8658c_scaled_data_t* gyro_data);

// Utility methods
esp_err_t (*read_temperature)(qmi8658c_t* self, float* temperature);
esp_err_t (*calibrate_gyro)(qmi8658c_t* self, uint16_t samples);
esp_err_t (*self_test)(qmi8658c_t* self, bool* acc_test_pass, bool* gyro_test_pass);
```

## Configuration Options

### Accelerometer Ranges
- `QMI8658C_ACC_RANGE_2G`: ±2g
- `QMI8658C_ACC_RANGE_4G`: ±4g  
- `QMI8658C_ACC_RANGE_8G`: ±8g
- `QMI8658C_ACC_RANGE_16G`: ±16g

### Gyroscope Ranges
- `QMI8658C_GYRO_RANGE_16DPS`: ±16°/s
- `QMI8658C_GYRO_RANGE_32DPS`: ±32°/s
- `QMI8658C_GYRO_RANGE_64DPS`: ±64°/s
- `QMI8658C_GYRO_RANGE_128DPS`: ±128°/s
- `QMI8658C_GYRO_RANGE_256DPS`: ±256°/s
- `QMI8658C_GYRO_RANGE_512DPS`: ±512°/s
- `QMI8658C_GYRO_RANGE_1024DPS`: ±1024°/s
- `QMI8658C_GYRO_RANGE_2048DPS`: ±2048°/s

### Output Data Rates
- `QMI8658C_ODR_8000HZ`: 8000 Hz
- `QMI8658C_ODR_4000HZ`: 4000 Hz
- `QMI8658C_ODR_2000HZ`: 2000 Hz
- `QMI8658C_ODR_1000HZ`: 1000 Hz
- `QMI8658C_ODR_500HZ`: 500 Hz
- `QMI8658C_ODR_250HZ`: 250 Hz
- `QMI8658C_ODR_125HZ`: 125 Hz
- `QMI8658C_ODR_62_5HZ`: 62.5 Hz
- `QMI8658C_ODR_31_25HZ`: 31.25 Hz





## License

This project is licensed under the MIT License - see the LICENSE file for details.





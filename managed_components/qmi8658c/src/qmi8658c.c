/**
 * @file qmi8658c.c
 * @brief ESP-IDF QMI8658C Accelerometer/Gyroscope Component Implementation
 */

#include "qmi8658c.h"

static const char *TAG = "QMI8658C";

// Static variables for standalone functions
static qmi8658c_config_t global_config;
static uint16_t global_acc_sensitivity = QMI8658C_ACC_SCALE_SENSITIVITY_2G;
static uint16_t global_gyro_sensitivity = QMI8658C_GYRO_SCALE_SENSITIVITY_16DPS;
static bool global_initialized = false;
static qmi8658c_scaled_data_t gyro_offset = {0.0f, 0.0f, 0.0f};


static esp_err_t qmi8658c_obj_set_mode(qmi8658c_t* self, qmi8658c_mode_t mode);
static esp_err_t qmi8658c_obj_set_acc_odr(qmi8658c_t* self, qmi8658c_acc_odr_t odr);
static esp_err_t qmi8658c_obj_set_acc_range(qmi8658c_t* self, qmi8658c_acc_range_t range);
static esp_err_t qmi8658c_obj_set_gyro_odr(qmi8658c_t* self, qmi8658c_gyro_odr_t odr);
static esp_err_t qmi8658c_obj_set_gyro_range(qmi8658c_t* self, qmi8658c_gyro_range_t range);
static esp_err_t qmi8658c_obj_enable_acc(qmi8658c_t* self, bool enable);
static esp_err_t qmi8658c_obj_enable_gyro(qmi8658c_t* self, bool enable);


// Sensitivity lookup tables (from Arduino library)
static const uint16_t acc_scale_sensitivity_table[4] = {
    QMI8658C_ACC_SCALE_SENSITIVITY_2G,   // ±2g
    QMI8658C_ACC_SCALE_SENSITIVITY_4G,   // ±4g
    QMI8658C_ACC_SCALE_SENSITIVITY_8G,   // ±8g
    QMI8658C_ACC_SCALE_SENSITIVITY_16G   // ±16g
};

static const uint16_t gyro_scale_sensitivity_table[8] = {
    QMI8658C_GYRO_SCALE_SENSITIVITY_16DPS,   // ±16 dps
    QMI8658C_GYRO_SCALE_SENSITIVITY_32DPS,   // ±32 dps
    QMI8658C_GYRO_SCALE_SENSITIVITY_64DPS,   // ±64 dps
    QMI8658C_GYRO_SCALE_SENSITIVITY_128DPS,  // ±128 dps
    QMI8658C_GYRO_SCALE_SENSITIVITY_256DPS,  // ±256 dps
    QMI8658C_GYRO_SCALE_SENSITIVITY_512DPS,  // ±512 dps
    QMI8658C_GYRO_SCALE_SENSITIVITY_1024DPS, // ±1024 dps
    QMI8658C_GYRO_SCALE_SENSITIVITY_2048DPS  // ±2048 dps
};

/**
 * @brief Get accelerometer sensitivity value
 */
static uint16_t get_acc_sensitivity(qmi8658c_acc_range_t range)
{
    if (range < 4) {
        return acc_scale_sensitivity_table[range];
    }
    return QMI8658C_ACC_SCALE_SENSITIVITY_2G; // Default fallback
}

/**
 * @brief Get gyroscope sensitivity value
 */
static uint16_t get_gyro_sensitivity(qmi8658c_gyro_range_t range)
{
    if (range < 8) {
        return gyro_scale_sensitivity_table[range];
    }
    return QMI8658C_GYRO_SCALE_SENSITIVITY_16DPS; // Default fallback
}

/**
 * @brief Write register to QMI8658C
 */
static esp_err_t qmi8658c_write_reg(i2c_port_t i2c_port, uint8_t i2c_addr, uint8_t reg, uint8_t data)
{
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(i2c_port, i2c_addr, write_buf, 2, 1000 / portTICK_PERIOD_MS);
}

/**
 * @brief Read register from QMI8658C
 */
static esp_err_t qmi8658c_read_reg(i2c_port_t i2c_port, uint8_t i2c_addr, uint8_t reg, uint8_t* data)
{
    return i2c_master_write_read_device(i2c_port, i2c_addr, &reg, 1, data, 1, 1000 / portTICK_PERIOD_MS);
}

/**
 * @brief Read multiple registers from QMI8658C
 */
static esp_err_t qmi8658c_read_regs(i2c_port_t i2c_port, uint8_t i2c_addr, uint8_t reg, uint8_t* data, size_t len)
{
    return i2c_master_write_read_device(i2c_port, i2c_addr, &reg, 1, data, len, 1000 / portTICK_PERIOD_MS);
}

// ============================================================================
// QMI8658C method implementations
// ============================================================================

/**
 * @brief Initialize QMI8658C instance
 */
static esp_err_t qmi8658c_obj_init(qmi8658c_t* self, const qmi8658c_config_t* config)
{
    if (!self || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Copy configuration
    self->config = *config;
    
    // Set sensitivity values
    self->acc_sensitivity = get_acc_sensitivity(config->acc_range);
    self->gyro_sensitivity = get_gyro_sensitivity(config->gyro_range);
    
    // Reset device (from Arduino library)
    esp_err_t ret = qmi8658c_write_reg(config->i2c_port, config->i2c_address, QMI8658C_REG_RESET, 0xB0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset device");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // Wait for reset
    
    // Check WHO_AM_I
    uint8_t who_am_i;
    ret = qmi8658c_read_reg(config->i2c_port, config->i2c_address, QMI8658C_REG_WHO_AM_I, &who_am_i);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read WHO_AM_I");
        return ret;
    }
    
    if (who_am_i != QMI8658C_CHIP_ID) {
        ESP_LOGE(TAG, "Invalid chip ID: 0x%02X, expected 0x%02X", who_am_i, QMI8658C_CHIP_ID);
        return ESP_ERR_NOT_FOUND;
    }

    // Set initialized flag before calling other functions that check it
    self->initialized = true;

    // Set operating mode
    ret = qmi8658c_obj_set_mode(self, config->mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mode");
        return ret;
    }
    
    // Configure accelerometer ODR and range
    ret = qmi8658c_obj_set_acc_odr(self, config->acc_odr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set accelerometer ODR");
        return ret;
    }
    
    ret = qmi8658c_obj_set_acc_range(self, config->acc_range);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set accelerometer range");
        return ret;
    }
    
    // Configure gyroscope ODR and range
    ret = qmi8658c_obj_set_gyro_odr(self, config->gyro_odr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set gyroscope ODR");
        return ret;
    }
    
    ret = qmi8658c_obj_set_gyro_range(self, config->gyro_range);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set gyroscope range");
        return ret;
    }

    ESP_LOGI(TAG, "QMI8658C initialized successfully");

    return ESP_OK;
}

/**
 * @brief Deinitialize QMI8658C instance
 */
static esp_err_t qmi8658c_obj_deinit(qmi8658c_t* self)
{
    if (!self || !self->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Disable all sensors (clear CTRL7 lower 4 bits)
    uint8_t ctrl7_reg;
    esp_err_t ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL7, &ctrl7_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL7 register");
        return ret;
    }
    
    ctrl7_reg &= 0xF0; // Disable accelerometer, gyroscope, magnetometer and attitude engine
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL7, ctrl7_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable sensors");
        return ret;
    }
    
    // Disable sensor by turning off the internal 2 MHz oscillator
    uint8_t ctrl1_reg;
    ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL1, &ctrl1_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL1 register");
        return ret;
    }
    
    ctrl1_reg |= (1 << 0);
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL1, ctrl1_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable oscillator");
        return ret;
    }
    
    self->initialized = false;
    ESP_LOGI(TAG, "QMI8658C deinitialized");
    
    return ESP_OK;
}

/**
 * @brief Reset QMI8658C instance
 */
static esp_err_t qmi8658c_obj_reset(qmi8658c_t* self)
{
    if (!self || !self->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_RESET, 0xB0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset device");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_LOGI(TAG, "QMI8658C reset completed");
    
    return ESP_OK;
}

/**
 * @brief Set operating mode
 */
static esp_err_t qmi8658c_obj_set_mode(qmi8658c_t* self, qmi8658c_mode_t mode)
{
    if (!self) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t ctrl7_reg;
    esp_err_t ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL7, &ctrl7_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL7 register");
        return ret;
    }
    
    ctrl7_reg = (ctrl7_reg & 0xFC) | mode;
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL7, ctrl7_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mode");
        return ret;
    }
    
    self->config.mode = mode;
    ESP_LOGI(TAG, "Operating mode set to %d", mode);
    
    return ESP_OK;
}

/**
 * @brief Set accelerometer range
 */
static esp_err_t qmi8658c_obj_set_acc_range(qmi8658c_t* self, qmi8658c_acc_range_t range)
{
    if (!self || !self->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl2_reg;
    esp_err_t ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL2, &ctrl2_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL2 register");
        return ret;
    }
    
    self->config.acc_range = range;
    self->acc_sensitivity = get_acc_sensitivity(range);
    ctrl2_reg = (ctrl2_reg & 0x8F) | (range << 4);
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL2, ctrl2_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Accelerometer range set to %d", range);
    }
    
    return ret;
}

/**
 * @brief Set gyroscope range
 */
static esp_err_t qmi8658c_obj_set_gyro_range(qmi8658c_t* self, qmi8658c_gyro_range_t range)
{
    if (!self || !self->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl3_reg;
    esp_err_t ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL3, &ctrl3_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL3 register");
        return ret;
    }
    
    self->config.gyro_range = range;
    self->gyro_sensitivity = get_gyro_sensitivity(range);
    
    ctrl3_reg = (ctrl3_reg & 0x8F) | (range << 4);
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL3, ctrl3_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Gyroscope range set to %d", range);
    }
    
    return ret;
}

/**
 * @brief Set accelerometer ODR
 */
static esp_err_t qmi8658c_obj_set_acc_odr(qmi8658c_t* self, qmi8658c_acc_odr_t odr)
{
    if (!self) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t ctrl2_reg;
    esp_err_t ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL2, &ctrl2_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL2 register");
        return ret;
    }
    
    self->config.acc_odr = odr;
    
    ctrl2_reg = (ctrl2_reg & 0xF0) | odr;
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL2, ctrl2_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Accelerometer ODR set to %d", odr);
    }
    
    return ret;
}

/**
 * @brief Set gyroscope ODR
 */
static esp_err_t qmi8658c_obj_set_gyro_odr(qmi8658c_t* self, qmi8658c_gyro_odr_t odr)
{
    if (!self) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t ctrl3_reg;
    esp_err_t ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL3, &ctrl3_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL3 register");
        return ret;
    }
    
    self->config.gyro_odr = odr;
    
    ctrl3_reg = (ctrl3_reg & 0xF0) | odr;
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL3, ctrl3_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Gyroscope ODR set to %d", odr);
    }
    
    return ret;
}

/**
 * @brief Enable/disable accelerometer
 */
static esp_err_t qmi8658c_obj_enable_acc(qmi8658c_t* self, bool enable)
{
    if (!self || !self->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl7_reg;
    esp_err_t ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL7, &ctrl7_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL7 register");
        return ret;
    }
    
    if (enable) {
        ctrl7_reg |= 0x01;
    } else {
        ctrl7_reg &= ~0x01;
    }
    
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL7, ctrl7_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Accelerometer %s", enable ? "enabled" : "disabled");
    }
    
    return ret;
}

/**
 * @brief Enable/disable gyroscope
 */
static esp_err_t qmi8658c_obj_enable_gyro(qmi8658c_t* self, bool enable)
{
    if (!self || !self->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl7_reg;
    esp_err_t ret = qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL7, &ctrl7_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL7 register");
        return ret;
    }
    
    if (enable) {
        ctrl7_reg |= 0x02;
    } else {
        ctrl7_reg &= ~0x02;
    }
    
    ret = qmi8658c_write_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_CTRL7, ctrl7_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Gyroscope %s", enable ? "enabled" : "disabled");
    }
    
    return ret;
}

/**
 * @brief Read WHO_AM_I register
 */
static esp_err_t qmi8658c_obj_read_who_am_i(qmi8658c_t* self, uint8_t* who_am_i)
{
    if (!self || !who_am_i) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_WHO_AM_I, who_am_i);
}

/**
 * @brief Read revision register
 */
static esp_err_t qmi8658c_obj_read_revision(qmi8658c_t* self, uint8_t* revision)
{
    if (!self || !revision) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return qmi8658c_read_reg(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_REVISION, revision);
}

/**
 * @brief Read temperature
 */
static esp_err_t qmi8658c_obj_read_temperature(qmi8658c_t* self, float* temperature)
{
    if (!self || !temperature || !self->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t temp_data[2];
    esp_err_t ret = qmi8658c_read_regs(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_TEMP_L, temp_data, 2);
    
    if (ret == ESP_OK) {
        int16_t temp_raw = (int16_t)(temp_data[1] << 8 | temp_data[0]);
        *temperature = (float)temp_raw / QMI8658C_TEMPERATURE_SENSOR_RESOLUTION; // Arduino library formula
    }
    
    return ret;
}

/**
 * @brief Read raw accelerometer data
 */
static esp_err_t qmi8658c_obj_read_acc_raw(qmi8658c_t* self, qmi8658c_raw_data_t* data)
{
    if (!self || !data || !self->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t acc_data[6];
    esp_err_t ret = qmi8658c_read_regs(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_AX_L, acc_data, 6);
    
    if (ret == ESP_OK) {
        data->x = (int16_t)(acc_data[1] << 8 | acc_data[0]);
        data->y = (int16_t)(acc_data[3] << 8 | acc_data[2]);
        data->z = (int16_t)(acc_data[5] << 8 | acc_data[4]);
    }
    
    return ret;
}

/**
 * @brief Read raw gyroscope data
 */
static esp_err_t qmi8658c_obj_read_gyro_raw(qmi8658c_t* self, qmi8658c_raw_data_t* data)
{
    if (!self || !data || !self->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t gyro_data[6];
    esp_err_t ret = qmi8658c_read_regs(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_GX_L, gyro_data, 6);
    
    if (ret == ESP_OK) {
        data->x = (int16_t)(gyro_data[1] << 8 | gyro_data[0]);
        data->y = (int16_t)(gyro_data[3] << 8 | gyro_data[2]);
        data->z = (int16_t)(gyro_data[5] << 8 | gyro_data[4]);
    }
    
    return ret;
}

/**
 * @brief Read scaled accelerometer data
 */
static esp_err_t qmi8658c_obj_read_acc_scaled(qmi8658c_t* self, qmi8658c_scaled_data_t* data)
{
    if (!self || !data || !self->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    qmi8658c_raw_data_t raw_data;
    esp_err_t ret = qmi8658c_obj_read_acc_raw(self, &raw_data);
    
    if (ret == ESP_OK) {
        // Use Arduino library formula: raw_value / sensitivity
        data->x = (float)raw_data.x / self->acc_sensitivity;
        data->y = (float)raw_data.y / self->acc_sensitivity;
        data->z = (float)raw_data.z / self->acc_sensitivity;
    }
    
    return ret;
}

/**
 * @brief Read scaled gyroscope data
 */
static esp_err_t qmi8658c_obj_read_gyro_scaled(qmi8658c_t* self, qmi8658c_scaled_data_t* data)
{
    if (!self || !data || !self->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    qmi8658c_raw_data_t raw_data;
    esp_err_t ret = qmi8658c_obj_read_gyro_raw(self, &raw_data);
    
    if (ret == ESP_OK) {
        // Use Arduino library formula: raw_value / sensitivity
        data->x = (float)raw_data.x / self->gyro_sensitivity;
        data->y = (float)raw_data.y / self->gyro_sensitivity;
        data->z = (float)raw_data.z / self->gyro_sensitivity;
    }
    
    return ret;
}

/**
 * @brief Read all raw data (accelerometer and gyroscope)
 */
static esp_err_t qmi8658c_obj_read_all_raw(qmi8658c_t* self, qmi8658c_raw_data_t* acc_data, qmi8658c_raw_data_t* gyro_data)
{
    if (!self || !acc_data || !gyro_data || !self->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Read all sensor data in one transaction for better efficiency
    uint8_t sensor_data[12];
    esp_err_t ret = qmi8658c_read_regs(self->config.i2c_port, self->config.i2c_address, QMI8658C_REG_AX_L, sensor_data, 12);
    
    if (ret == ESP_OK) {
        // Accelerometer data
        acc_data->x = (int16_t)(sensor_data[1] << 8 | sensor_data[0]);
        acc_data->y = (int16_t)(sensor_data[3] << 8 | sensor_data[2]);
        acc_data->z = (int16_t)(sensor_data[5] << 8 | sensor_data[4]);
        
        // Gyroscope data
        gyro_data->x = (int16_t)(sensor_data[7] << 8 | sensor_data[6]);
        gyro_data->y = (int16_t)(sensor_data[9] << 8 | sensor_data[8]);
        gyro_data->z = (int16_t)(sensor_data[11] << 8 | sensor_data[10]);
    }
    
    return ret;
}

/**
 * @brief Read all scaled data (accelerometer and gyroscope)
 */
static esp_err_t qmi8658c_obj_read_all_scaled(qmi8658c_t* self, qmi8658c_scaled_data_t* acc_data, qmi8658c_scaled_data_t* gyro_data)
{
    if (!self || !acc_data || !gyro_data || !self->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    qmi8658c_raw_data_t raw_acc, raw_gyro;
    esp_err_t ret = qmi8658c_obj_read_all_raw(self, &raw_acc, &raw_gyro);
    
    if (ret == ESP_OK) {
        // Scale accelerometer data using Arduino library formula
        acc_data->x = (float)raw_acc.x / self->acc_sensitivity;
        acc_data->y = (float)raw_acc.y / self->acc_sensitivity;
        acc_data->z = (float)raw_acc.z / self->acc_sensitivity;
        
        // Scale gyroscope data using Arduino library formula
        gyro_data->x = (float)raw_gyro.x / self->gyro_sensitivity;
        gyro_data->y = (float)raw_gyro.y / self->gyro_sensitivity;
        gyro_data->z = (float)raw_gyro.z / self->gyro_sensitivity;
    }
    
    return ret;
}

/**
 * @brief Calibrate gyroscope
 */
static esp_err_t qmi8658c_obj_calibrate_gyro(qmi8658c_t* self, uint16_t samples)
{
    if (!self || !self->initialized || samples == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Starting gyroscope calibration with %d samples", samples);
    
    qmi8658c_scaled_data_t sum = {0.0f, 0.0f, 0.0f};
    qmi8658c_scaled_data_t gyro_data;
    
    for (uint16_t i = 0; i < samples; i++) {
        esp_err_t ret = qmi8658c_obj_read_gyro_scaled(self, &gyro_data);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read gyroscope data during calibration");
            return ret;
        }
        
        sum.x += gyro_data.x;
        sum.y += gyro_data.y;
        sum.z += gyro_data.z;
        
        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay between readings
    }
    
    // Calculate averages (these are the offsets)
    gyro_offset.x = sum.x / samples;
    gyro_offset.y = sum.y / samples;
    gyro_offset.z = sum.z / samples;
    
    ESP_LOGI(TAG, "Gyroscope calibration completed. Offsets: X=%.3f, Y=%.3f, Z=%.3f", 
             gyro_offset.x, gyro_offset.y, gyro_offset.z);
    
    return ESP_OK;
}

/**
 * @brief Self test
 */
static esp_err_t qmi8658c_obj_self_test(qmi8658c_t* self, bool* acc_test_pass, bool* gyro_test_pass)
{
    if (!self || !acc_test_pass || !gyro_test_pass || !self->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Simple self test: check if sensors are responding
    qmi8658c_raw_data_t acc_data, gyro_data;
    
    esp_err_t ret_acc = qmi8658c_obj_read_acc_raw(self, &acc_data);
    esp_err_t ret_gyro = qmi8658c_obj_read_gyro_raw(self, &gyro_data);
    
    *acc_test_pass = (ret_acc == ESP_OK) && (acc_data.x != 0 || acc_data.y != 0 || acc_data.z != 0);
    *gyro_test_pass = (ret_gyro == ESP_OK);
    
    ESP_LOGI(TAG, "Self test completed. ACC: %s, GYRO: %s", 
             *acc_test_pass ? "PASS" : "FAIL", 
             *gyro_test_pass ? "PASS" : "FAIL");
    
    return ESP_OK;
}

// ============================================================================
// Public functions
// ============================================================================

/**
 * @brief Initialize a QMI8658C instance
 */
void qmi8658c_create(qmi8658c_t* qmi8658c_instance)
{
    // Initialize member variables
    memset(&qmi8658c_instance->config, 0, sizeof(qmi8658c_config_t));
    qmi8658c_instance->acc_sensitivity = QMI8658C_ACC_SCALE_SENSITIVITY_2G;
    qmi8658c_instance->gyro_sensitivity = QMI8658C_GYRO_SCALE_SENSITIVITY_16DPS;
    qmi8658c_instance->initialized = false;
    
    // Initialize function pointers
    qmi8658c_instance->init = qmi8658c_obj_init;
    qmi8658c_instance->deinit = qmi8658c_obj_deinit;
    qmi8658c_instance->reset = qmi8658c_obj_reset;
    qmi8658c_instance->set_mode = qmi8658c_obj_set_mode;
    qmi8658c_instance->set_acc_range = qmi8658c_obj_set_acc_range;
    qmi8658c_instance->set_gyro_range = qmi8658c_obj_set_gyro_range;
    qmi8658c_instance->set_acc_odr = qmi8658c_obj_set_acc_odr;
    qmi8658c_instance->set_gyro_odr = qmi8658c_obj_set_gyro_odr;
    
    qmi8658c_instance->read_who_am_i = qmi8658c_obj_read_who_am_i;
    qmi8658c_instance->read_revision = qmi8658c_obj_read_revision;
    qmi8658c_instance->read_temperature = qmi8658c_obj_read_temperature;
    qmi8658c_instance->read_acc_raw = qmi8658c_obj_read_acc_raw;
    qmi8658c_instance->read_gyro_raw = qmi8658c_obj_read_gyro_raw;
    qmi8658c_instance->read_acc_scaled = qmi8658c_obj_read_acc_scaled;
    qmi8658c_instance->read_gyro_scaled = qmi8658c_obj_read_gyro_scaled;
    qmi8658c_instance->read_all_raw = qmi8658c_obj_read_all_raw;
    qmi8658c_instance->read_all_scaled = qmi8658c_obj_read_all_scaled;
    
    qmi8658c_instance->calibrate_gyro = qmi8658c_obj_calibrate_gyro;
    qmi8658c_instance->self_test = qmi8658c_obj_self_test;
}

// Global QMI8658C instance
qmi8658c_t qmi8658c;

esp_err_t qmi8658c_init_default(i2c_port_t i2c_port)
{
    // Initialize the global QMI8658C instance
    qmi8658c_create(&qmi8658c);
    // Create default configuration
    qmi8658c_config_t config = QMI8658C_DEFAULT_CONFIG();
    config.i2c_port = i2c_port;
    
    esp_err_t ret = qmi8658c.init(&qmi8658c, &config);
   
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "QMI8658C component initialized with default configuration");
    }
    
    return ret;
}

// ============================================================================
// Standalone functions (for backward compatibility)
// ============================================================================

esp_err_t qmi8658c_init_with_config(const qmi8658c_config_t* config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Copy configuration
    global_config = *config;
    
    // Set sensitivity values
    global_acc_sensitivity = get_acc_sensitivity(config->acc_range);
    global_gyro_sensitivity = get_gyro_sensitivity(config->gyro_range);
    
    // Reset device (Arduino library method)
    esp_err_t ret = qmi8658c_write_reg(config->i2c_port, config->i2c_address, QMI8658C_REG_RESET, 0xB0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset device");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Check WHO_AM_I
    uint8_t who_am_i;
    ret = qmi8658c_read_reg(config->i2c_port, config->i2c_address, QMI8658C_REG_WHO_AM_I, &who_am_i);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read WHO_AM_I");
        return ret;
    }
    
    if (who_am_i != QMI8658C_CHIP_ID) {
        ESP_LOGE(TAG, "Invalid chip ID: 0x%02X, expected 0x%02X", who_am_i, QMI8658C_CHIP_ID);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Set operating mode
    ret = qmi8658c_set_mode(config->mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mode");
        return ret;
    }
    
    // Configure accelerometer ODR and range
    ret = qmi8658c_set_acc_odr(config->acc_odr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set accelerometer ODR");
        return ret;
    }
    
    ret = qmi8658c_set_acc_range(config->acc_range);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set accelerometer range");
        return ret;
    }
    
    // Configure gyroscope ODR and range
    ret = qmi8658c_set_gyro_odr(config->gyro_odr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set gyroscope ODR");
        return ret;
    }
    
    ret = qmi8658c_set_gyro_range(config->gyro_range);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set gyroscope range");
        return ret;
    }
    
    global_initialized = true;
    ESP_LOGI(TAG, "QMI8658C initialized successfully (standalone)");
    
    return ESP_OK;
}

esp_err_t qmi8658c_deinit(void)
{
    if (!global_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = qmi8658c_write_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL7, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable sensors");
        return ret;
    }
    
    global_initialized = false;
    ESP_LOGI(TAG, "QMI8658C deinitialized (standalone)");
    
    return ESP_OK;
}

esp_err_t qmi8658c_read_revision(uint8_t* revision)
{
    if (!revision || !global_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return qmi8658c_read_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_REVISION, revision);
}

esp_err_t qmi8658c_set_mode(qmi8658c_mode_t mode)
{
    if (!global_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl7_reg;
    esp_err_t ret = qmi8658c_read_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL7, &ctrl7_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL7 register");
        return ret;
    }
    
    ctrl7_reg = (ctrl7_reg & 0xFC) | mode;
    ret = qmi8658c_write_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL7, ctrl7_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mode");
        return ret;
    }
    
    global_config.mode = mode;
    ESP_LOGI(TAG, "Operating mode set to %d (standalone)", mode);
    
    return ESP_OK;
}

esp_err_t qmi8658c_set_acc_odr(qmi8658c_acc_odr_t odr)
{
    if (!global_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl2_reg;
    esp_err_t ret = qmi8658c_read_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL2, &ctrl2_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL2 register");
        return ret;
    }
    
    global_config.acc_odr = odr;
    
    ctrl2_reg = (ctrl2_reg & 0xF0) | odr;
    ret = qmi8658c_write_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL2, ctrl2_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Accelerometer ODR set to %d (standalone)", odr);
    }
    
    return ret;
}

esp_err_t qmi8658c_set_gyro_odr(qmi8658c_gyro_odr_t odr)
{
    if (!global_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl3_reg;
    esp_err_t ret = qmi8658c_read_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL3, &ctrl3_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL3 register");
        return ret;
    }
    
    global_config.gyro_odr = odr;
    
    ctrl3_reg = (ctrl3_reg & 0xF0) | odr;
    ret = qmi8658c_write_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL3, ctrl3_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Gyroscope ODR set to %d (standalone)", odr);
    }
    
    return ret;
}
esp_err_t qmi8658c_read_who_am_i(uint8_t* who_am_i)
{
    if (!who_am_i || !global_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return qmi8658c_read_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_WHO_AM_I, who_am_i);
}

esp_err_t qmi8658c_read_temperature(float* temperature)
{
    if (!temperature || !global_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t temp_data[2];
    esp_err_t ret = qmi8658c_read_regs(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_TEMP_L, temp_data, 2);
    
    if (ret == ESP_OK) {
        int16_t temp_raw = (int16_t)(temp_data[1] << 8 | temp_data[0]);
        *temperature = (float)temp_raw / QMI8658C_TEMPERATURE_SENSOR_RESOLUTION;
    }
    
    return ret;
}

esp_err_t qmi8658c_read_acc_raw(qmi8658c_raw_data_t* data)
{
    if (!data || !global_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t acc_data[6];
    esp_err_t ret = qmi8658c_read_regs(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_AX_L, acc_data, 6);
    
    if (ret == ESP_OK) {
        data->x = (int16_t)(acc_data[1] << 8 | acc_data[0]);
        data->y = (int16_t)(acc_data[3] << 8 | acc_data[2]);
        data->z = (int16_t)(acc_data[5] << 8 | acc_data[4]);
    }
    
    return ret;
}

esp_err_t qmi8658c_read_gyro_raw(qmi8658c_raw_data_t* data)
{
    if (!data || !global_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t gyro_data[6];
    esp_err_t ret = qmi8658c_read_regs(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_GX_L, gyro_data, 6);
    
    if (ret == ESP_OK) {
        data->x = (int16_t)(gyro_data[1] << 8 | gyro_data[0]);
        data->y = (int16_t)(gyro_data[3] << 8 | gyro_data[2]);
        data->z = (int16_t)(gyro_data[5] << 8 | gyro_data[4]);
    }
    
    return ret;
}

esp_err_t qmi8658c_read_acc_scaled(qmi8658c_scaled_data_t* data)
{
    if (!data || !global_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    qmi8658c_raw_data_t raw_data;
    esp_err_t ret = qmi8658c_read_acc_raw(&raw_data);
    
    if (ret == ESP_OK) {
        data->x = (float)raw_data.x / global_acc_sensitivity;
        data->y = (float)raw_data.y / global_acc_sensitivity;
        data->z = (float)raw_data.z / global_acc_sensitivity;
    }
    
    return ret;
}

esp_err_t qmi8658c_read_gyro_scaled(qmi8658c_scaled_data_t* data)
{
    if (!data || !global_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    qmi8658c_raw_data_t raw_data;
    esp_err_t ret = qmi8658c_read_gyro_raw(&raw_data);
    
    if (ret == ESP_OK) {
        data->x = (float)raw_data.x / global_gyro_sensitivity - gyro_offset.x;
        data->y = (float)raw_data.y / global_gyro_sensitivity - gyro_offset.y;
        data->z = (float)raw_data.z / global_gyro_sensitivity - gyro_offset.z;
    }
    
    return ret;
}

esp_err_t qmi8658c_set_acc_range(qmi8658c_acc_range_t range)
{
    if (!global_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl2_reg;
    esp_err_t ret = qmi8658c_read_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL2, &ctrl2_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL2 register");
        return ret;
    }
    
    global_config.acc_range = range;
    global_acc_sensitivity = get_acc_sensitivity(range);
    
    ctrl2_reg = (ctrl2_reg & 0x8F) | (range << 4);
    ret = qmi8658c_write_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL2, ctrl2_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Accelerometer range set to %d (standalone)", range);
    }
    
    return ret;
}

esp_err_t qmi8658c_set_gyro_range(qmi8658c_gyro_range_t range)
{
    if (!global_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t ctrl3_reg;
    esp_err_t ret = qmi8658c_read_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL3, &ctrl3_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL3 register");
        return ret;
    }
    
    global_config.gyro_range = range;
    global_gyro_sensitivity = get_gyro_sensitivity(range);
    
    ctrl3_reg = (ctrl3_reg & 0x8F) | (range << 4);
    ret = qmi8658c_write_reg(global_config.i2c_port, global_config.i2c_address, QMI8658C_REG_CTRL3, ctrl3_reg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Gyroscope range set to %d (standalone)", range);
    }
    
    return ret;
}

esp_err_t qmi8658c_calibrate_gyro(uint16_t samples)
{
    if (!global_initialized || samples == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Starting gyroscope calibration with %d samples (standalone)", samples);
    
    qmi8658c_scaled_data_t sum = {0.0f, 0.0f, 0.0f};
    qmi8658c_raw_data_t raw_data;
    
    for (uint16_t i = 0; i < samples; i++) {
        esp_err_t ret = qmi8658c_read_gyro_raw(&raw_data);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read gyroscope data during calibration");
            return ret;
        }
        
        // Convert raw to scaled and accumulate
        sum.x += (float)raw_data.x / global_gyro_sensitivity;
        sum.y += (float)raw_data.y / global_gyro_sensitivity;
        sum.z += (float)raw_data.z / global_gyro_sensitivity;
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Calculate new offsets
    gyro_offset.x = sum.x / samples;
    gyro_offset.y = sum.y / samples;
    gyro_offset.z = sum.z / samples;
    
    ESP_LOGI(TAG, "Gyroscope calibration completed (standalone). Offsets: X=%.3f, Y=%.3f, Z=%.3f", 
             gyro_offset.x, gyro_offset.y, gyro_offset.z);
    
    return ESP_OK;
}

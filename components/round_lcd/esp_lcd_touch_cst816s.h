/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new CST816S touch driver
 *
 * @note The I2C communication should be initialized before use this function.
 *
 * @param io LCD panel IO handle
 * @param config Touch configuration
 * @param tp Touch handle
 * @return
 *      - ESP_OK: on success
 */
esp_err_t esp_lcd_touch_new_i2c_cst816s(i2c_port_t i2c_num, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *tp);

#ifdef __cplusplus
}
#endif
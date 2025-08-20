/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_check.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_touch.h>

#include "esp_lcd_touch_cst816s.h"

static const char *TAG = "CST816S";

/* CST816S I2C address and registers */
#define CST816S_I2C_ADDR        0x15
#define CST816S_REG_STATUS      0x00
#define CST816S_REG_FINGERNUM   0x02
#define CST816S_REG_X_LOW       0x03
#define CST816S_REG_X_HIGH      0x04
#define CST816S_REG_Y_LOW       0x05
#define CST816S_REG_Y_HIGH      0x06

typedef struct {
    esp_lcd_touch_t base;
    i2c_port_t i2c_num;
    uint8_t i2c_addr;
    gpio_num_t reset_gpio;
    gpio_num_t interrupt_gpio;
} cst816s_handle_t;

static esp_err_t cst816s_read_data(esp_lcd_touch_handle_t tp);
static bool cst816s_get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);
static esp_err_t cst816s_del(esp_lcd_touch_handle_t tp);

static esp_err_t cst816s_i2c_read(i2c_port_t i2c_num, uint8_t reg, uint8_t *data, uint8_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (CST816S_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (CST816S_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

static esp_err_t cst816s_read_data(esp_lcd_touch_handle_t tp)
{
    cst816s_handle_t *cst816s = (cst816s_handle_t *)tp;
    uint8_t data[7];
    
    ESP_RETURN_ON_ERROR(cst816s_i2c_read(cst816s->i2c_num, CST816S_REG_STATUS, data, sizeof(data)), TAG, "I2C read failed");
    
    /* Parse touch data */
    if (data[CST816S_REG_FINGERNUM] > 0) {
        uint16_t x = ((data[CST816S_REG_X_HIGH] & 0x0F) << 8) | data[CST816S_REG_X_LOW];
        uint16_t y = ((data[CST816S_REG_Y_HIGH] & 0x0F) << 8) | data[CST816S_REG_Y_LOW];
        
        /* Store touch data */
        tp->data.coords[0].x = x;
        tp->data.coords[0].y = y;
        tp->data.coords[0].strength = 50; // Default strength
        tp->data.points = 1;
    } else {
        tp->data.points = 0;
    }
    
    return ESP_OK;
}

static bool cst816s_get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    assert(tp && x && y && point_num);
    
    *point_num = tp->data.points > max_point_num ? max_point_num : tp->data.points;
    
    if (*point_num > 0) {
        *x = tp->data.coords[0].x;
        *y = tp->data.coords[0].y;
        if (strength) {
            *strength = tp->data.coords[0].strength;
        }
        return true;
    }
    
    return false;
}

static esp_err_t cst816s_del(esp_lcd_touch_handle_t tp)
{
    assert(tp);
    
    /* Reset GPIO */
    cst816s_handle_t *cst816s = (cst816s_handle_t *)tp;
    if (cst816s->reset_gpio != GPIO_NUM_NC) {
        gpio_reset_pin(cst816s->reset_gpio);
    }
    if (cst816s->interrupt_gpio != GPIO_NUM_NC) {
        gpio_reset_pin(cst816s->interrupt_gpio);
    }
    
    free(tp);
    return ESP_OK;
}

esp_err_t esp_lcd_touch_new_i2c_cst816s(i2c_port_t i2c_num, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *tp)
{
    ESP_RETURN_ON_FALSE(config && tp, ESP_ERR_INVALID_ARG, TAG, "Invalid arguments");
    
    cst816s_handle_t *cst816s = calloc(1, sizeof(cst816s_handle_t));
    ESP_RETURN_ON_FALSE(cst816s, ESP_ERR_NO_MEM, TAG, "No memory for CST816S");
    
    /* Save configuration */
    cst816s->i2c_num = i2c_num;
    cst816s->i2c_addr = CST816S_I2C_ADDR;
    cst816s->reset_gpio = config->rst_gpio_num;
    cst816s->interrupt_gpio = config->int_gpio_num;
    
    /* Configure base touch functions */
    cst816s->base.read_data = cst816s_read_data;
    cst816s->base.get_xy = cst816s_get_xy;
    cst816s->base.del = cst816s_del;
    
    /* Configure parameters */
    cst816s->base.data.coords[0].x = 0;
    cst816s->base.data.coords[0].y = 0;
    cst816s->base.data.points = 0;
    
    /* Configure interrupt pin */
    if (cst816s->interrupt_gpio != GPIO_NUM_NC) {
        gpio_config_t int_gpio_config = {
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = 1ULL << cst816s->interrupt_gpio,
            .pull_up_en = GPIO_PULLUP_ENABLE,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&int_gpio_config), err, TAG, "GPIO config failed");
    }
    
    *tp = (esp_lcd_touch_handle_t)cst816s;
    
    ESP_LOGI(TAG, "CST816S touch controller initialized");
    return ESP_OK;
    
err:
    free(cst816s);
    return ESP_FAIL;
}
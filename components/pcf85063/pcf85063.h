#ifndef _PCF85063_H_
#define _PCF85063_H_


#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"


esp_err_t rtc_read_reg(uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t rtc_write_reg(uint8_t reg_addr, uint8_t *data, size_t len);
uint8_t bcd_to_dec(uint8_t val);
uint8_t dec_to_bcd(uint8_t val);
esp_err_t rtc_get_time(void);
esp_err_t rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t month, uint8_t year);
#endif

#ifndef _ST7789_H_
#define  _ST7789_H_

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
#include "lv_conf.h"


esp_err_t app_lcd_init(void);
esp_err_t app_lvgl_init(void);
void _app_button_cb(lv_event_t*);
void app_main_display(void);
#endif

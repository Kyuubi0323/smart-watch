/**
 * @file st7789.h
 * @brief ESP-IDF ST7789 Component
 * @author khoi.nv0323.work@gmail.com
 * @date 2025
 */

#ifndef _ST7789_H_
#define  _ST7789_H_

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"


typedef struct {
    // GPIO pins
    gpio_num_t pin_bl;      // Backlight pin
    gpio_num_t pin_dc;      // Data/Command pin
    gpio_num_t pin_cs;      // Chip Select pin
    gpio_num_t pin_sclk;    // SPI Clock pin
    gpio_num_t pin_mosi;    // SPI MOSI pin
    gpio_num_t pin_rst;     // Reset pin
    
    // Display dimensions
    uint16_t h_res;         // Horizontal resolution
    uint16_t v_res;         // Vertical resolution
    
    // SPI configuration
    spi_host_device_t spi_host;
    uint32_t pixel_clk_hz;
    uint8_t cmd_bits;
    uint8_t param_bits;
    
    // Display settings
    esp_lcd_color_space_t color_space;
    uint8_t bits_per_pixel;
    uint8_t bl_on_level;    // Backlight on level (0 or 1)
    
    // Optional settings
    bool mirror_x;
    bool mirror_y;
    bool invert_color;
    uint16_t gap_x;
    uint16_t gap_y;
} st7789_config_t;



typedef struct {
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel_handle;
    st7789_config_t config;
} st7789_handle_t;

// API functions
esp_err_t st7789_init(st7789_handle_t *handle, const st7789_config_t *config);
esp_err_t st7789_deinit(st7789_handle_t *handle);
esp_err_t st7789_set_backlight(st7789_handle_t *handle, bool on);
esp_err_t st7789_display_on_off(st7789_handle_t *handle, bool on);
esp_err_t st7789_draw_bitmap(st7789_handle_t *handle, int x_start, int y_start, int x_end, int y_end, const void *color_data);

// Helper function to get default configuration for round ST7789
st7789_config_t st7789_get_round_default_config(void);





#endif 

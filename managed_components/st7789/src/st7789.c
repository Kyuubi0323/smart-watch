#include "st7789.h"

static const char* TAG = "ST7789";

st7789_config_t st7789_get_round_default_config(void)
{
    st7789_config_t config = {
        // GPIO pins (user should modify these based on their hardware)
        .pin_bl = GPIO_NUM_15,
        .pin_dc = GPIO_NUM_4,
        .pin_cs = GPIO_NUM_5,
        .pin_sclk = GPIO_NUM_6,
        .pin_mosi = GPIO_NUM_7,
        .pin_rst = GPIO_NUM_8,
        
        // Round display dimensions (240x240 is common for round ST7789)
        .h_res = 240,
        .v_res = 240,
        
        // SPI configuration
        .spi_host = SPI3_HOST,
        .pixel_clk_hz = 40 * 1000 * 1000,  // 40 MHz
        .cmd_bits = 8,
        .param_bits = 8,
        
        // Display settings
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
        .bl_on_level = 1,
        
        // Optional settings for round display
        .mirror_x = true,
        .mirror_y = true,
        .invert_color = true,
        .gap_x = 0,
        .gap_y = 20,  // Common offset for round displays
    };
    
    return config;
}

esp_err_t st7789_init(st7789_handle_t *handle, const st7789_config_t *config)
{
    if (!handle || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ESP_OK;
    
    // Copy configuration
    handle->config = *config;
    handle->io_handle = NULL;
    handle->panel_handle = NULL;
    
    // Configure backlight GPIO
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << config->pin_bl
    };
    ESP_RETURN_ON_ERROR(gpio_config(&bk_gpio_config), TAG, "Backlight GPIO config failed");
    
    // Initialize SPI bus
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = config->pin_sclk,
        .mosi_io_num = config->pin_mosi,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = config->h_res * config->v_res * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(config->spi_host, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");
    
    // Install panel IO
    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = config->pin_dc,
        .cs_gpio_num = config->pin_cs,
        .pclk_hz = config->pixel_clk_hz,
        .lcd_cmd_bits = config->cmd_bits,
        .lcd_param_bits = config->param_bits,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)config->spi_host, &io_config, &handle->io_handle), 
                      err, TAG, "New panel IO failed");
    
    // Install LCD driver
    ESP_LOGD(TAG, "Install ST7789 LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = config->pin_rst,
        .color_space = config->color_space,
        .bits_per_pixel = config->bits_per_pixel,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7789(handle->io_handle, &panel_config, &handle->panel_handle), 
                      err, TAG, "New panel failed");
    
    // Initialize panel
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(handle->panel_handle), TAG, "Panel reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(handle->panel_handle), TAG, "Panel init failed");
    
    // Apply configuration settings
    if (config->mirror_x || config->mirror_y) {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_mirror(handle->panel_handle, config->mirror_x, config->mirror_y), 
                           TAG, "Panel mirror failed");
    }
    
    if (config->gap_x > 0 || config->gap_y > 0) {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_set_gap(handle->panel_handle, config->gap_x, config->gap_y), 
                           TAG, "Panel set gap failed");
    }
    
    if (config->invert_color) {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(handle->panel_handle, true), 
                           TAG, "Panel invert color failed");
    }
    
    // Turn on display
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(handle->panel_handle, true), 
                       TAG, "Panel display on failed");
    
    // Turn on backlight
    ESP_RETURN_ON_ERROR(st7789_set_backlight(handle, true), TAG, "Backlight on failed");
    
    ESP_LOGI(TAG, "ST7789 initialized successfully (%dx%d)", config->h_res, config->v_res);
    return ESP_OK;

err:
    st7789_deinit(handle);
    return ret;
}

esp_err_t st7789_deinit(st7789_handle_t *handle)
{
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Turn off backlight
    st7789_set_backlight(handle, false);
    
    // Clean up panel
    if (handle->panel_handle) {
        esp_lcd_panel_del(handle->panel_handle);
        handle->panel_handle = NULL;
    }
    
    // Clean up panel IO
    if (handle->io_handle) {
        esp_lcd_panel_io_del(handle->io_handle);
        handle->io_handle = NULL;
    }
    
    // Free SPI bus
    spi_bus_free(handle->config.spi_host);
    
    ESP_LOGI(TAG, "ST7789 deinitialized");
    return ESP_OK;
}

esp_err_t st7789_set_backlight(st7789_handle_t *handle, bool on)
{
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t level = on ? handle->config.bl_on_level : (1 - handle->config.bl_on_level);
    return gpio_set_level(handle->config.pin_bl, level);
}

esp_err_t st7789_display_on_off(st7789_handle_t *handle, bool on)
{
    if (!handle || !handle->panel_handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return esp_lcd_panel_disp_on_off(handle->panel_handle, on);
}

esp_err_t st7789_draw_bitmap(st7789_handle_t *handle, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    if (!handle || !handle->panel_handle || !color_data) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return esp_lcd_panel_draw_bitmap(handle->panel_handle, x_start, y_start, x_end, y_end, color_data);
}
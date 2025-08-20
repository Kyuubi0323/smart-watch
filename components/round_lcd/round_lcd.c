#include "round_lcd.h"
#include "esp_lcd_gc9a01.h"
#include <math.h>

/* ESP32-C3 Mini Round LCD Pin Configuration (based on fbiego/esp32-c3-mini) */
#define LCD_GPIO_BL GPIO_NUM_3
#define LCD_GPIO_DC GPIO_NUM_2
#define LCD_GPIO_CS GPIO_NUM_10
#define LCD_GPIO_SCLK GPIO_NUM_6
#define LCD_GPIO_MOSI GPIO_NUM_7
#define LCD_GPIO_RST GPIO_NUM_NC     // Reset not used on this board

/* Round LCD Resolution - GC9A01 240x240 */
#define LCD_H_RES   240
#define LCD_DRAW_BUFF_HEIGHT    60
#define LCD_V_RES   240

/* ESP32-C3 uses SPI2_HOST for display */
#define LCD_SPI_NUM         SPI2_HOST
#define LCD_PIXEL_CLK_HZ    80 * 1000 * 1000
#define LCD_CMD_BITS        8
#define LCD_PARAM_BITS      8
#define LCD_COLOR_SPACE     ESP_LCD_COLOR_SPACE_BGR
#define LCD_BITS_PER_PIXEL  16
#define LCD_DRAW_BUFF_DOUBLE 1
#define LCD_BL_ON_LEVEL     1

/* Touch Controller Configuration - CST816S */
#define TOUCH_I2C_NUM       I2C_NUM_0
#define TOUCH_GPIO_SDA      GPIO_NUM_4
#define TOUCH_GPIO_SCL      GPIO_NUM_5
#define TOUCH_GPIO_INT      GPIO_NUM_0
#define TOUCH_GPIO_RST      GPIO_NUM_1
#define TOUCH_I2C_CLK_HZ    400000

esp_lcd_panel_io_handle_t lcd_io = NULL;
esp_lcd_panel_handle_t lcd_panel = NULL;
esp_lcd_touch_handle_t touch_handle = NULL;
lv_obj_t *avatar;
/* LVGL display and touch */
lv_display_t *lvgl_disp = NULL;
lv_indev_t *lvgl_touch_indev = NULL;

static const char* TAG = "ROUND_LCD";

esp_err_t app_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    /* LCD backlight */
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_GPIO_BL
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    /* LCD initialization */
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_GPIO_SCLK,
        .mosi_io_num = LCD_GPIO_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_GPIO_DC,
        .cs_gpio_num = LCD_GPIO_CS,
        .pclk_hz = LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_NUM, &io_config, &lcd_io), err, TAG, "New panel IO failed");

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_GPIO_RST,
        .color_space = LCD_COLOR_SPACE,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_gc9a01(lcd_io, &panel_config, &lcd_panel), err, TAG, "New panel failed");

    esp_lcd_panel_reset(lcd_panel);
    esp_lcd_panel_init(lcd_panel);
    esp_lcd_panel_mirror(lcd_panel, false, false);
    esp_lcd_panel_disp_on_off(lcd_panel, true);

    /* LCD backlight on */
    ESP_ERROR_CHECK(gpio_set_level(LCD_GPIO_BL, LCD_BL_ON_LEVEL));

    // No need to set gap or invert for GC9A01 round display
    
    return ret;

err:
    if (lcd_panel)
    {
        esp_lcd_panel_del(lcd_panel);
    }
    if (lcd_io)
    {
        esp_lcd_panel_io_del(lcd_io);
    }
    spi_bus_free(LCD_SPI_NUM);
    return ret;
}

esp_err_t app_touch_init(void)
{
    esp_err_t ret = ESP_OK;

    /* Initialize I2C for touch controller */
    ESP_LOGI(TAG, "Initialize I2C bus for touch");
    const i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_GPIO_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = TOUCH_GPIO_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = TOUCH_I2C_CLK_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(TOUCH_I2C_NUM, &i2c_conf), TAG, "I2C config failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(TOUCH_I2C_NUM, I2C_MODE_MASTER, 0, 0, 0), TAG, "I2C install failed");

    /* Configure touch controller reset and interrupt pins */
    if (TOUCH_GPIO_RST != GPIO_NUM_NC) {
        gpio_config_t rst_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << TOUCH_GPIO_RST
        };
        ESP_RETURN_ON_ERROR(gpio_config(&rst_gpio_config), TAG, "Touch RST GPIO config failed");
        gpio_set_level(TOUCH_GPIO_RST, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(TOUCH_GPIO_RST, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    /* Initialize touch controller - Generic I2C touch for CST816S */
    ESP_LOGI(TAG, "Initialize CST816S touch controller");
    
    // Note: We'll implement a simple CST816S touch handler since ESP-IDF may not have built-in support
    // For now, we'll set up the basic I2C communication
    
    return ret;
}



esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
        .double_buffer = LCD_DRAW_BUFF_DOUBLE,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        /* Rotation values for round display */
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    /* Add touch input if touch controller is initialized */
    if (touch_handle != NULL) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lvgl_disp,
            .handle = touch_handle,
        };
        lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);
    }

    return ESP_OK;
}


void _app_button_cb(lv_event_t *e)
{
    lv_disp_rotation_t rotation = lv_disp_get_rotation(lvgl_disp);
    rotation++;
    if (rotation > LV_DISPLAY_ROTATION_270)
    {
        rotation = LV_DISPLAY_ROTATION_0;
    }

    /* LCD HW rotation */
    lv_disp_set_rotation(lvgl_disp, rotation);
}

void app_main_display(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* Task lock */
    lvgl_port_lock(0);

    /* Set background color to black */
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    /* Create a circular mask for round display */
    static lv_draw_mask_radius_param_t mask_rout_param;
    lv_draw_mask_radius_init(&mask_rout_param, &(lv_area_t){0, 0, LCD_H_RES-1, LCD_V_RES-1}, LCD_H_RES/2, false);
    int16_t mask_id = lv_draw_mask_add(&mask_rout_param, NULL);

    /* Create main container for watch face */
    lv_obj_t *watch_face = lv_obj_create(scr);
    lv_obj_set_size(watch_face, LCD_H_RES, LCD_V_RES);
    lv_obj_set_style_bg_color(watch_face, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(watch_face, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(watch_face, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(watch_face, 0, LV_PART_MAIN);
    lv_obj_center(watch_face);

    /* Create circular border */
    lv_obj_t *border = lv_obj_create(watch_face);
    lv_obj_set_size(border, LCD_H_RES - 4, LCD_V_RES - 4);
    lv_obj_set_style_bg_opa(border, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(border, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(border, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(border, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_center(border);

    /* Create time label */
    lv_obj_t *time_label = lv_label_create(watch_face);
    lv_label_set_text(time_label, "12:34");
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(time_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -20);

    /* Create date label */
    lv_obj_t *date_label = lv_label_create(watch_face);
    lv_label_set_text(date_label, "MON 15");
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(date_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 20);

    /* Add some dots around the circle for watch markers */
    for (int i = 0; i < 12; i++) {
        lv_obj_t *dot = lv_obj_create(watch_face);
        lv_obj_set_size(dot, 6, 6);
        lv_obj_set_style_bg_color(dot, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        
        /* Position dots around the circle */
        float angle = i * 30.0f * 3.14159f / 180.0f; // 30 degrees apart
        int x = (LCD_H_RES / 2) + (LCD_H_RES / 2 - 20) * cos(angle) - 3;
        int y = (LCD_V_RES / 2) + (LCD_V_RES / 2 - 20) * sin(angle) - 3;
        lv_obj_set_pos(dot, x, y);
    }

    /* Remove the mask */
    lv_draw_mask_remove_id(mask_id);

    ESP_LOGI(TAG, "Round watch face displayed successfully");

    /* Task unlock */
    lvgl_port_unlock();
}
#include "round_lcd.h"
#include "esp_lcd_gc9a01.h"
#include "esp_lcd_touch_cst816s.h"
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

    /* Initialize CST816S touch controller */
    ESP_LOGI(TAG, "Initialize CST816S touch controller");
    esp_lcd_touch_config_t touch_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = TOUCH_GPIO_RST,
        .int_gpio_num = TOUCH_GPIO_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    
    ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_cst816s(TOUCH_I2C_NUM, &touch_cfg, &touch_handle), TAG, "Touch controller init failed");
    
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


/* Global variables */
static int current_watchface = 0;
static lv_obj_t *watchface_container = NULL;

void create_digital_watchface(lv_obj_t *parent);
void create_analog_watchface(lv_obj_t *parent);

void _app_button_cb(lv_event_t *e)
{
    /* Cycle through different watch faces on touch */
    current_watchface = (current_watchface + 1) % 2;
    
    if (watchface_container) {
        lv_obj_del(watchface_container);
    }
    
    watchface_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(watchface_container, LCD_H_RES, LCD_V_RES);
    lv_obj_set_style_bg_color(watchface_container, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(watchface_container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(watchface_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(watchface_container, 0, LV_PART_MAIN);
    lv_obj_center(watchface_container);
    
    if (current_watchface == 0) {
        create_digital_watchface(watchface_container);
    } else {
        create_analog_watchface(watchface_container);
    }
    
    ESP_LOGI(TAG, "Switched to watchface %d", current_watchface);
}

void create_digital_watchface(lv_obj_t *parent) 
{
    /* Create circular border */
    lv_obj_t *border = lv_obj_create(parent);
    lv_obj_set_size(border, LCD_H_RES - 4, LCD_V_RES - 4);
    lv_obj_set_style_bg_opa(border, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(border, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(border, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(border, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_center(border);

    /* Create time label */
    lv_obj_t *time_label = lv_label_create(parent);
    lv_label_set_text(time_label, "12:34");
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(time_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -20);

    /* Create date label */
    lv_obj_t *date_label = lv_label_create(parent);
    lv_label_set_text(date_label, "MON 15");
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(date_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 20);

    /* Add hour markers */
    for (int i = 0; i < 12; i++) {
        lv_obj_t *dot = lv_obj_create(parent);
        lv_obj_set_size(dot, 4, 4);
        lv_obj_set_style_bg_color(dot, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        
        float angle = (i * 30.0f - 90.0f) * M_PI / 180.0f;
        int x = (LCD_H_RES / 2) + (LCD_H_RES / 2 - 25) * cos(angle) - 2;
        int y = (LCD_V_RES / 2) + (LCD_V_RES / 2 - 25) * sin(angle) - 2;
        lv_obj_set_pos(dot, x, y);
    }

    /* Info label */
    lv_obj_t *info_label = lv_label_create(parent);
    lv_label_set_text(info_label, "Digital\nWatch");
    lv_obj_set_style_text_font(info_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_style_text_color(info_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(info_label, LV_ALIGN_CENTER, 0, 60);
}

void create_analog_watchface(lv_obj_t *parent)
{
    /* Create circular border */
    lv_obj_t *border = lv_obj_create(parent);
    lv_obj_set_size(border, LCD_H_RES - 4, LCD_V_RES - 4);
    lv_obj_set_style_bg_opa(border, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(border, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_set_style_border_width(border, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(border, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_center(border);

    /* Center dot */
    lv_obj_t *center = lv_obj_create(parent);
    lv_obj_set_size(center, 8, 8);
    lv_obj_set_style_bg_color(center, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(center, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(center, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(center, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_center(center);

    /* Hour hand (pointing to 12) */
    lv_obj_t *hour_hand = lv_obj_create(parent);
    lv_obj_set_size(hour_hand, 4, 60);
    lv_obj_set_style_bg_color(hour_hand, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(hour_hand, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(hour_hand, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(hour_hand, 2, LV_PART_MAIN);
    lv_obj_align(hour_hand, LV_ALIGN_CENTER, 0, -30);

    /* Minute hand (pointing to 7) */  
    lv_obj_t *minute_hand = lv_obj_create(parent);
    lv_obj_set_size(minute_hand, 2, 80);
    lv_obj_set_style_bg_color(minute_hand, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(minute_hand, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(minute_hand, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(minute_hand, 1, LV_PART_MAIN);
    lv_obj_align(minute_hand, LV_ALIGN_CENTER, 25, 25);

    /* Numbers at 12, 3, 6, 9 */
    const char* numbers[] = {"12", "3", "6", "9"};
    int positions[][2] = {{0, -80}, {70, 0}, {0, 80}, {-70, 0}};
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t *num_label = lv_label_create(parent);
        lv_label_set_text(num_label, numbers[i]);
        lv_obj_set_style_text_font(num_label, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(num_label, lv_color_white(), LV_PART_MAIN);
        lv_obj_align(num_label, LV_ALIGN_CENTER, positions[i][0], positions[i][1]);
    }

    /* Info label */
    lv_obj_t *info_label = lv_label_create(parent);
    lv_label_set_text(info_label, "Analog");
    lv_obj_set_style_text_font(info_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_style_text_color(info_label, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_align(info_label, LV_ALIGN_CENTER, 0, 50);
}

void app_main_display(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* Task lock */
    lvgl_port_lock(0);

    /* Set background color to black */
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    /* Add touch event to screen for watchface cycling */
    lv_obj_add_event_cb(scr, _app_button_cb, LV_EVENT_CLICKED, NULL);

    /* Create initial watchface container */
    watchface_container = lv_obj_create(scr);
    lv_obj_set_size(watchface_container, LCD_H_RES, LCD_V_RES);
    lv_obj_set_style_bg_color(watchface_container, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(watchface_container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(watchface_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(watchface_container, 0, LV_PART_MAIN);
    lv_obj_center(watchface_container);

    /* Start with digital watchface */
    create_digital_watchface(watchface_container);

    /* Create instruction label */
    lv_obj_t *instruction_label = lv_label_create(scr);
    lv_label_set_text(instruction_label, "Touch to switch faces");
    lv_obj_set_style_text_font(instruction_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_style_text_color(instruction_label, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_align(instruction_label, LV_ALIGN_BOTTOM_MID, 0, -10);

    ESP_LOGI(TAG, "Round watch display initialized (GC9A01 240x240) - Touch screen to cycle watchfaces");
    ESP_LOGI(TAG, "Integration of Felix Biego's round LCD concepts complete");

    /* Task unlock */
    lvgl_port_unlock();
}
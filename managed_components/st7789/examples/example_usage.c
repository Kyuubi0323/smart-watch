/**
 * @file example_usage.c
 * @brief Comprehensive usage example for ST7789 display component
 * 
 * This example demonstrates:
 * - Basic ST7789 initialization with configurable settings
 * - Drawing different shapes and patterns
 * - Color space and bitmap operations
 * - Display control (backlight, on/off)
 * - Performance testing and optimization
 * - Error handling best practices
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "st7789.h"

static const char *TAG = "ST7789_EXAMPLE";

// Color definitions in RGB565 format
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_MAGENTA 0xF81F
#define COLOR_CYAN    0x07FF
#define COLOR_ORANGE  0xFD20
#define COLOR_PURPLE  0x8010

// Global display handle
static st7789_handle_t display;

/**
 * @brief Fill a rectangular area with solid color
 */
esp_err_t fill_rectangle(int x, int y, int width, int height, uint16_t color)
{
    if (x < 0 || y < 0 || width <= 0 || height <= 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Calculate total pixels
    int total_pixels = width * height;
    
    // Allocate buffer for the rectangle
    uint16_t *buffer = malloc(total_pixels * sizeof(uint16_t));
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate %d bytes for rectangle", total_pixels * 2);
        return ESP_ERR_NO_MEM;
    }
    
    // Fill buffer with color
    for (int i = 0; i < total_pixels; i++) {
        buffer[i] = color;
    }
    
    // Draw to display
    esp_err_t ret = st7789_draw_bitmap(&display, x, y, x + width, y + height, buffer);
    
    // Clean up
    free(buffer);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to draw rectangle: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief Clear entire display with specified color
 */
esp_err_t clear_display(uint16_t color)
{
    ESP_LOGI(TAG, "Clearing display with color 0x%04X", color);
    return fill_rectangle(0, 0, display.config.h_res, display.config.v_res, color);
}

/**
 * @brief Draw a gradient pattern
 */
void draw_gradient(void)
{
    ESP_LOGI(TAG, "Drawing gradient pattern...");
    
    int width = display.config.h_res;
    int height = display.config.v_res;
    
    // Create line buffer
    uint16_t *line_buffer = malloc(width * sizeof(uint16_t));
    if (!line_buffer) {
        ESP_LOGE(TAG, "Failed to allocate line buffer for gradient");
        return;
    }
    
    // Draw gradient line by line
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create RGB gradient
            uint8_t red = (x * 31) / width;           // 0-31 (5 bits)
            uint8_t green = (y * 63) / height;       // 0-63 (6 bits)
            uint8_t blue = ((x + y) * 31) / (width + height); // 0-31 (5 bits)
            
            // Convert to RGB565
            line_buffer[x] = (red << 11) | (green << 5) | blue;
        }
        
        // Draw this line
        st7789_draw_bitmap(&display, 0, y, width, y + 1, line_buffer);
    }
    
    free(line_buffer);
    ESP_LOGI(TAG, "Gradient pattern completed");
}

/**
 * @brief Draw color test pattern
 */
void draw_color_test_pattern(void)
{
    ESP_LOGI(TAG, "Drawing color test pattern...");
    
    int width = display.config.h_res;
    int height = display.config.v_res;
    
    // Define colors to test
    uint16_t colors[] = {
        COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW,
        COLOR_MAGENTA, COLOR_CYAN, COLOR_ORANGE, COLOR_PURPLE
    };
    
    int num_colors = sizeof(colors) / sizeof(colors[0]);
    int strip_height = height / num_colors;
    
    // Draw horizontal color strips
    for (int i = 0; i < num_colors; i++) {
        int y = i * strip_height;
        int h = (i == num_colors - 1) ? (height - y) : strip_height; // Last strip fills remainder
        
        fill_rectangle(0, y, width, h, colors[i]);
        ESP_LOGD(TAG, "Drew color strip %d: 0x%04X", i, colors[i]);
    }
    
    ESP_LOGI(TAG, "Color test pattern completed");
}

/**
 * @brief Draw geometric shapes
 */
void draw_geometric_shapes(void)
{
    ESP_LOGI(TAG, "Drawing geometric shapes...");
    
    clear_display(COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    int width = display.config.h_res;
    int height = display.config.v_res;
    int center_x = width / 2;
    int center_y = height / 2;
    
    // Draw concentric rectangles
    for (int i = 0; i < 5; i++) {
        int size = 20 + i * 20;
        int x = center_x - size / 2;
        int y = center_y - size / 2;
        uint16_t color = (i % 2) ? COLOR_WHITE : COLOR_RED;
        
        // Draw rectangle border (4 lines)
        fill_rectangle(x, y, size, 3, color);           // Top
        fill_rectangle(x, y + size - 3, size, 3, color); // Bottom
        fill_rectangle(x, y, 3, size, color);           // Left
        fill_rectangle(x + size - 3, y, 3, size, color); // Right
    }
    
    // Draw corner markers
    fill_rectangle(0, 0, 20, 20, COLOR_GREEN);                    // Top-left
    fill_rectangle(width - 20, 0, 20, 20, COLOR_BLUE);           // Top-right
    fill_rectangle(0, height - 20, 20, 20, COLOR_YELLOW);        // Bottom-left
    fill_rectangle(width - 20, height - 20, 20, 20, COLOR_CYAN); // Bottom-right
    
    ESP_LOGI(TAG, "Geometric shapes completed");
}

/**
 * @brief Test display boundaries and orientation
 */
void test_display_boundaries(void)
{
    ESP_LOGI(TAG, "Testing display boundaries and orientation...");
    
    clear_display(COLOR_BLACK);
    
    int width = display.config.h_res;
    int height = display.config.v_res;
    
    // Draw border around entire display
    fill_rectangle(0, 0, width, 5, COLOR_RED);           // Top border
    fill_rectangle(0, height - 5, width, 5, COLOR_BLUE); // Bottom border
    fill_rectangle(0, 0, 5, height, COLOR_GREEN);        // Left border
    fill_rectangle(width - 5, 0, 5, height, COLOR_YELLOW); // Right border
    
    // Draw crosshair in center
    fill_rectangle(0, height / 2 - 1, width, 3, COLOR_WHITE);     // Horizontal line
    fill_rectangle(width / 2 - 1, 0, 3, height, COLOR_WHITE);    // Vertical line
    
    // Add resolution text simulation (using rectangles)
    // This creates a simple pattern indicating the display resolution
    int marker_size = 10;
    for (int i = 0; i < width / marker_size; i++) {
        if (i % 10 == 0) { // Every 10th marker is larger
            fill_rectangle(i * marker_size, 10, 2, 15, COLOR_CYAN);
            fill_rectangle(i * marker_size, height - 25, 2, 15, COLOR_CYAN);
        } else {
            fill_rectangle(i * marker_size, 15, 1, 5, COLOR_CYAN);
            fill_rectangle(i * marker_size, height - 20, 1, 5, COLOR_CYAN);
        }
    }
    
    ESP_LOGI(TAG, "Boundary test completed - Resolution: %dx%d", width, height);
}

/**
 * @brief Performance test - measure drawing speed
 */
void performance_test(void)
{
    ESP_LOGI(TAG, "Running performance tests...");
    
    int64_t start_time, end_time;
    
    // Test 1: Full screen clear
    start_time = esp_timer_get_time();
    clear_display(COLOR_RED);
    end_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Full screen clear: %lld µs", end_time - start_time);
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Test 2: Small rectangle drawing
    start_time = esp_timer_get_time();
    for (int i = 0; i < 100; i++) {
        fill_rectangle(i % 200, (i * 2) % 200, 10, 10, COLOR_GREEN);
    }
    end_time = esp_timer_get_time();
    ESP_LOGI(TAG, "100 small rectangles: %lld µs", end_time - start_time);
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Test 3: Line drawing simulation
    start_time = esp_timer_get_time();
    for (int i = 0; i < display.config.h_res; i += 5) {
        fill_rectangle(i, 0, 1, display.config.v_res, COLOR_BLUE);
    }
    end_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Vertical lines: %lld µs", end_time - start_time);
    
    ESP_LOGI(TAG, "Performance tests completed");
}

/**
 * @brief Test display control functions
 */
void test_display_controls(void)
{
    ESP_LOGI(TAG, "Testing display control functions...");
    
    clear_display(COLOR_WHITE);
    
    // Test backlight control
    ESP_LOGI(TAG, "Testing backlight control...");
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "Backlight OFF");
        st7789_set_backlight(&display, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        ESP_LOGI(TAG, "Backlight ON");
        st7789_set_backlight(&display, true);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Test display on/off
    ESP_LOGI(TAG, "Testing display on/off...");
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "Display OFF");
        st7789_display_on_off(&display, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        ESP_LOGI(TAG, "Display ON");
        st7789_display_on_off(&display, true);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    ESP_LOGI(TAG, "Display control tests completed");
}

/**
 * @brief Demonstrate configuration options
 */
void demonstrate_configuration(void)
{
    ESP_LOGI(TAG, "=== Display Configuration ===");
    ESP_LOGI(TAG, "Resolution: %dx%d", display.config.h_res, display.config.v_res);
    ESP_LOGI(TAG, "SPI Host: SPI%d", display.config.spi_host + 1);
    ESP_LOGI(TAG, "SPI Frequency: %lu Hz (%.1f MHz)", 
             display.config.pixel_clk_hz, display.config.pixel_clk_hz / 1000000.0);
    ESP_LOGI(TAG, "GPIO Pins:");
    ESP_LOGI(TAG, "  Backlight: %d", display.config.pin_bl);
    ESP_LOGI(TAG, "  DC: %d", display.config.pin_dc);
    ESP_LOGI(TAG, "  CS: %d", display.config.pin_cs);
    ESP_LOGI(TAG, "  SCLK: %d", display.config.pin_sclk);
    ESP_LOGI(TAG, "  MOSI: %d", display.config.pin_mosi);
    ESP_LOGI(TAG, "  RST: %d", display.config.pin_rst);
    ESP_LOGI(TAG, "Display Settings:");
    ESP_LOGI(TAG, "  Color Space: %s", 
             display.config.color_space == ESP_LCD_COLOR_SPACE_BGR ? "BGR" : "RGB");
    ESP_LOGI(TAG, "  Mirror X: %s", display.config.mirror_x ? "Yes" : "No");
    ESP_LOGI(TAG, "  Mirror Y: %s", display.config.mirror_y ? "Yes" : "No");
    ESP_LOGI(TAG, "  Invert Color: %s", display.config.invert_color ? "Yes" : "No");
    ESP_LOGI(TAG, "  Gap X: %d, Gap Y: %d", display.config.gap_x, display.config.gap_y);
    ESP_LOGI(TAG, "============================");
}
void app_main(void)
{
    ESP_LOGI(TAG, "=== ST7789 Comprehensive Example ===");
    
    // Get default configuration for round display
    st7789_config_t config = st7789_get_round_default_config();
    
    // Optional: Customize configuration for your specific hardware
    // config.pin_bl = GPIO_NUM_10;       // Different backlight pin
    // config.h_res = 320;               // Different resolution
    // config.v_res = 320;
    // config.pixel_clk_hz = 20000000;   // Lower frequency for stability
    // config.mirror_x = false;          // Adjust orientation
    // config.invert_color = false;      // Adjust colors
    
    ESP_LOGI(TAG, "Initializing ST7789 display...");
    esp_err_t ret = st7789_init(&display, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display initialization failed: %s", esp_err_to_name(ret));
        ESP_LOGE(TAG, "Check your wiring and configuration!");
        return;
    }
    
    ESP_LOGI(TAG, "Display initialized successfully!");
    
    // Show current configuration
    demonstrate_configuration();
    
    // Wait for display to stabilize
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Run comprehensive tests
    while (1) {
        ESP_LOGI(TAG, "\n--- Starting test cycle ---");
        
        // Test 1: Color test pattern
        draw_color_test_pattern();
        vTaskDelay(pdMS_TO_TICKS(3000));
        
        // Test 2: Gradient pattern  
        draw_gradient();
        vTaskDelay(pdMS_TO_TICKS(3000));
        
        // Test 3: Geometric shapes
        draw_geometric_shapes();
        vTaskDelay(pdMS_TO_TICKS(3000));
        
        // Test 4: Boundary and orientation test
        test_display_boundaries();
        vTaskDelay(pdMS_TO_TICKS(3000));
        
        // Test 5: Display controls (backlight, on/off)
        test_display_controls();
        
        // Test 6: Performance benchmarks
        performance_test();
        
        ESP_LOGI(TAG, "--- Test cycle completed ---");
        ESP_LOGI(TAG, "Restarting tests in 5 seconds...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
    // Cleanup (never reached in this example)
    ESP_LOGI(TAG, "Cleaning up display...");
    st7789_deinit(&display);
}

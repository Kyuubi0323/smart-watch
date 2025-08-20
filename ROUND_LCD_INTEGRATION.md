# Round LCD Integration - ESP32-C3 Mini

This project integrates Felix Biego's (@fbiego/esp32-c3-mini) round LCD concepts into an ESP-IDF based smart watch project.

## Hardware Configuration

### Display - GC9A01 Round LCD (240x240)
- **Controller**: GC9A01
- **Resolution**: 240x240 pixels
- **Interface**: SPI (SPI2_HOST)
- **Pins**:
  - SCLK: GPIO_NUM_6
  - MOSI: GPIO_NUM_7
  - DC: GPIO_NUM_2
  - CS: GPIO_NUM_10
  - BL: GPIO_NUM_3 (Backlight)
  - RST: Not connected

### Touch Controller - CST816S
- **Controller**: CST816S Capacitive Touch
- **Interface**: I2C (I2C_NUM_0)
- **Pins**:
  - SDA: GPIO_NUM_4
  - SCL: GPIO_NUM_5
  - INT: GPIO_NUM_0
  - RST: GPIO_NUM_1

## Features Implemented

### Round Display Support
- Custom GC9A01 driver for ESP-IDF
- 240x240 round display configuration
- Circular UI elements and borders
- LVGL integration with round-specific layouts

### Touch Interface
- Custom CST816S touch driver
- Touch event handling for watch face switching
- I2C communication with proper initialization sequence

### Watch Face System
- Digital watch face with time, date, and hour markers
- Analog watch face with clock hands and numbers
- Touch-to-switch functionality between faces
- Extensible design for adding more watch faces

## Key Differences from Felix's Implementation

### Framework
- **Felix's Version**: Arduino/PlatformIO with LovyanGFX
- **This Version**: ESP-IDF with esp_lcd_panel and LVGL

### Display Driver
- **Felix's Version**: LovyanGFX Panel_GC9A01
- **This Version**: Custom esp_lcd_gc9a01.c driver

### Touch Handling
- **Felix's Version**: Arduino Touch_CST816S library
- **This Version**: Custom esp_lcd_touch_cst816s.c driver

### Build System
- **Felix's Version**: PlatformIO
- **This Version**: ESP-IDF CMake build system

## Extending the Implementation

To add more of Felix's features:

1. **Weather Display**: Integrate with WiFi and weather APIs
2. **Bluetooth**: Add BLE support for notifications
3. **Real-time Clock**: Add RTC for accurate timekeeping
4. **More Watch Faces**: Port Felix's additional watch face designs
5. **Settings Menu**: Add brightness, timeout, and other settings
6. **Navigation**: Implement screen swiping and navigation

## Usage

1. Build and flash to ESP32-C3 mini with round LCD
2. The display will show a digital watch face initially
3. Touch the screen to cycle between digital and analog faces
4. The implementation demonstrates successful integration of round LCD concepts

## Credits

- Original round LCD implementation: Felix Biego (@fbiego/esp32-c3-mini)
- Hardware design: ESP32-C3 mini development board
- Integration: ESP-IDF compatible round LCD system
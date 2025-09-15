# ST7789 Display Driver Component

A reusable, low-coupling ESP-IDF component for ST7789 TFT displays, specifically optimized for round displays commonly used in smartwatches and wearable devices.

## Features

- ✅ **Hardware Agnostic**: Configurable GPIO pins and display parameters
- ✅ **No Global State**: Multiple display instances supported
- ✅ **Low Coupling**: No dependencies on LVGL or other graphics libraries
- ✅ **Round Display Optimized**: Pre-configured settings for round ST7789 displays
- ✅ **Clean API**: Simple initialization and control functions
- ✅ **Memory Efficient**: Minimal RAM footprint
- ✅ **Error Handling**: Comprehensive ESP-IDF error handling

## Supported Displays

- Round ST7789 displays (240x240 pixels) - Default configuration
- Rectangular ST7789 displays (customizable resolution)
- Any ST7789-based TFT display with SPI interface

## Hardware Requirements

- ESP32/ESP32-S3/ESP32-C3 or compatible microcontroller
- ST7789 TFT display with SPI interface
- Minimum 6 GPIO pins for display control

## Dependencies

- ESP-IDF v4.4 or later
- esp_lcd component
- driver component  
- gpio component
- freertos component

## Installation

### As a Git Submodule

1. Add this repository as a submodule to your ESP-IDF project's `components` directory:
   ```bash
   cd your_project/components
   git submodule add https://github.com/Kyuubi0323/st7789 st7789
   ```

2. To use a specific version (recommended for stability):
   ```bash
   cd your_project/components/st7789
   git checkout main
   ```

3. Update your project's CMakeLists.txt to include the component (if not automatically detected):
   ```cmake
   # In your main CMakeLists.txt
   set(EXTRA_COMPONENT_DIRS components/st7789)
   ```

### Using ESP Component Registry

Add to your project's `idf_component.yml`:
```yaml
dependencies:
  st7789:
    git: https://github.com/Kyuubi0323/st7789
    version: main  # Use specific version for stability
```

## Hardware Setup

Connect your ST7789 display to the ESP32 using these default pins:

| ST7789 Pin | ESP32 GPIO | Function |
|------------|------------|----------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCL/CLK | GPIO_6 | SPI Clock |
| SDA/MOSI | GPIO_7 | SPI Data |
| RES/RST | GPIO_8 | Reset |
| DC | GPIO_4 | Data/Command |
| CS | GPIO_5 | Chip Select |
| BLK | GPIO_15 | Backlight |

## License

This project is licensed under the MIT License. See the main project LICENSE file for details.


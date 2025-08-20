# ESP32-C3 1.69" LCD Integration Guide

## Overview
This integration brings support for ESP32-C3 microcontrollers with 1.69" rectangle LCD displays (240x280 resolution) based on the @fbiego/esp32-c3-mini project design patterns.

## Available Board Configurations

### ESP32-C3 with 1.69" Rectangle LCD (`ESPC3_1_69`)
- **Resolution**: 240x280 pixels
- **Color Order**: RGB  
- **Display Offset**: X=0, Y=20
- **Pin Configuration**:
  - SCLK: GPIO 6
  - MOSI: GPIO 7  
  - DC: GPIO 4
  - CS: GPIO 5
  - RST: GPIO 8
  - BL: GPIO 15

### ESP32-C3 with 1.28" Round LCD (`ESPC3`)  
- **Resolution**: 240x240 pixels
- **Color Order**: BGR
- **Display Offset**: X=0, Y=0
- **Pin Configuration**:
  - SCLK: GPIO 6
  - MOSI: GPIO 7
  - DC: GPIO 2  
  - CS: GPIO 10
  - RST: -1 (not used)
  - BL: GPIO 3

### ESP32-S3 with 1.69" Rectangle LCD (`ESPS3_1_69`) - Default
- **Resolution**: 240x280 pixels  
- **Color Order**: RGB
- **Display Offset**: X=0, Y=20
- **Pin Configuration**:
  - SCLK: GPIO 6
  - MOSI: GPIO 7
  - DC: GPIO 4
  - CS: GPIO 5  
  - RST: GPIO 8
  - BL: GPIO 15

## How to Build

### Option 1: Using CMake Build Flags (Recommended)
```bash
# For ESP32-C3 with 1.69" LCD:
idf.py set-target esp32c3
idf.py -DBOARD_CONFIG=ESPC3_1_69 build

# For ESP32-C3 with 1.28" LCD:
idf.py set-target esp32c3  
idf.py -DBOARD_CONFIG=ESPC3 build

# For ESP32-S3 with 1.69" LCD (default):
idf.py set-target esp32s3
idf.py build
```

### Option 2: Manual Configuration in Code
Edit `components/custominclude/main.h` and ensure the desired configuration block is enabled by uncommenting the appropriate `#define`.

## Code Changes Summary

### 1. Added New Configuration (`ESPC3_1_69`)
The new configuration supports the ESP32-C3 with 1.69" rectangle LCD with the exact pin mappings from @fbiego/esp32-c3-mini project.

### 2. Refactored ST7789 Component
- **Dynamic Configuration**: Removed hardcoded values, now reads from `main.h`
- **Pin Flexibility**: Supports disabled pins (value -1) 
- **Resolution Flexibility**: Supports different WIDTH/HEIGHT combinations
- **Color Space**: Automatically sets RGB/BGR based on configuration

### 3. Enhanced Build System
- Added CMake support for runtime board configuration
- Maintains backward compatibility with existing builds
- Provides clear feedback on selected configuration

## Technical Details

### Pin Mapping Comparison
| Function | ESPC3_1_69 | ESPC3 | ESPS3_1_69 |
|----------|------------|-------|-------------|
| SCLK     | GPIO 6     | GPIO 6| GPIO 6      |
| MOSI     | GPIO 7     | GPIO 7| GPIO 7      |  
| DC       | GPIO 4     | GPIO 2| GPIO 4      |
| CS       | GPIO 5     | GPIO 10| GPIO 5     |
| RST      | GPIO 8     | -1    | GPIO 8      |
| BL       | GPIO 15    | GPIO 3| GPIO 15     |

### Display Specifications  
| Config     | Width | Height | Offset Y | Color Order |
|------------|-------|--------|----------|-------------|
| ESPC3_1_69 | 240   | 280    | 20       | RGB         |
| ESPC3      | 240   | 240    | 0        | BGR         |
| ESPS3_1_69 | 240   | 280    | 20       | RGB         |

## Troubleshooting

### Common Issues:
1. **Wrong target selected**: Ensure `idf.py set-target esp32c3` is run for ESP32-C3 boards
2. **GPIO conflicts**: Check pin assignments don't conflict with other components
3. **Display not working**: Verify color order (RGB vs BGR) and offset settings
4. **Build errors**: Ensure all required ESP-IDF components are available

### Verification:
The configuration can be tested by checking the macro definitions. The integration has been validated with both ESPC3_1_69 and ESPC3 configurations.

## Integration Benefits
- ✅ **Arduino-to-ESP-IDF**: Successfully adapted @fbiego/esp32-c3-mini Arduino/PlatformIO patterns to ESP-IDF
- ✅ **Multiple Board Support**: Single codebase supports various ESP32-C3/S3 boards  
- ✅ **Configuration-Driven**: Hardware differences handled via compile-time configuration
- ✅ **Minimal Changes**: Surgical modifications that preserve existing functionality
- ✅ **Future-Proof**: Easy to add more board configurations using the same pattern
#!/bin/bash

# ESP32-C3 Smart Watch Build Script
# Usage: ./build_config.sh [config_name]
# Available configs: espc3_1_69, espc3_1_28, esps3_1_69 (default)

set -e

CONFIG=${1:-"esps3_1_69"}

case $CONFIG in
    "espc3_1_69")
        echo "Building for ESP32-C3 with 1.69\" LCD (240x280)"
        idf.py set-target esp32c3
        idf.py -DBOARD_CONFIG=ESPC3_1_69 build
        ;;
    "espc3_1_28")
        echo "Building for ESP32-C3 with 1.28\" LCD (240x240)"
        idf.py set-target esp32c3
        idf.py -DBOARD_CONFIG=ESPC3 build
        ;;
    "esps3_1_69")
        echo "Building for ESP32-S3 with 1.69\" LCD (240x280) - Default"
        idf.py set-target esp32s3
        idf.py build
        ;;
    "elecrow_c3")
        echo "Building for ElecRow ESP32-C3 board"
        idf.py set-target esp32c3
        idf.py -DBOARD_CONFIG=ELECROW_C3 build
        ;;
    "clean")
        echo "Cleaning build directory"
        idf.py clean
        ;;
    "help"|"-h"|"--help")
        echo "ESP32-C3 Smart Watch Build Script"
        echo ""
        echo "Usage: $0 [config_name]"
        echo ""
        echo "Available configurations:"
        echo "  espc3_1_69   - ESP32-C3 with 1.69\" LCD (240x280)"
        echo "  espc3_1_28   - ESP32-C3 with 1.28\" LCD (240x240)"  
        echo "  esps3_1_69   - ESP32-S3 with 1.69\" LCD (240x280) [default]"
        echo "  elecrow_c3   - ElecRow ESP32-C3 board"
        echo "  clean        - Clean build directory"
        echo "  help         - Show this help"
        echo ""
        echo "Examples:"
        echo "  $0 espc3_1_69     # Build for ESP32-C3 1.69\" LCD"
        echo "  $0 clean          # Clean build"
        echo "  $0                # Build default (ESP32-S3 1.69\")"
        ;;
    *)
        echo "Error: Unknown configuration '$CONFIG'"
        echo "Use '$0 help' to see available configurations"
        exit 1
        ;;
esac
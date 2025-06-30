# NimBLE-Arduino ESP-IDF Component
file(GLOB_RECURSE nimble_src "src/*.cpp" "src/*.c")

set(nimble_req espressif__arduino-esp32 bt nvs_flash)

idf_component_register(SRCS ${nimble_src}
                       INCLUDE_DIRS "src"
                       REQUIRES ${nimble_req})

# Set C++ standard for this component
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_11)
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

# Add BT include directories - try multiple paths since ESP32S3 might use esp32 headers
target_include_directories(${COMPONENT_LIB} PRIVATE
    "$ENV{IDF_PATH}/components/bt/include/esp32/include"
    "$ENV{IDF_PATH}/components/bt/include/esp32s3/include"
    "$ENV{IDF_PATH}/components/bt/include/${IDF_TARGET}/include")

file(GLOB app_src "*.c" "*.cpp")
set(pri_req esp_event esp_http_server esp_http_client esp_https_ota esp_wifi esp_partition mqtt bt esp_lcd
freertos espressif__esp_lvgl_port spiffs nvs_flash driver vfs json log wear_levelling espressif__arduino-esp32)
set(priv_req custominclude customui LovyanGFX timber arduinojson)

idf_component_register(SRCS ${app_src}
                       INCLUDE_DIRS "."
                    REQUIRES ${pri_req}
                    PRIV_REQUIRES ${priv_req})
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
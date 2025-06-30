# Collect all face source files from subdirectories
file(GLOB_RECURSE face_src "*.c")

set(pri_req esp_event esp_http_server esp_http_client esp_https_ota esp_wifi esp_partition mqtt bt esp_lcd
freertos espressif__esp_lvgl_port spiffs nvs_flash driver vfs json log customhal)

# Include all face subdirectories
set(face_include_dirs ".")
file(GLOB face_dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
foreach(dir ${face_dirs})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
        list(APPEND face_include_dirs ${dir})
    endif()
endforeach()

idf_component_register(SRCS ${face_src}
                       INCLUDE_DIRS ${face_include_dirs}
                    REQUIRES ${pri_req})
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

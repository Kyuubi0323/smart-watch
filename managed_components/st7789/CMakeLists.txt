# file(GLOB app_src "*.c")
# set(pri_req esp_lcd driver freertos log)

# idf_component_register(SRCS ${app_src}                          
#                        INCLUDE_DIRS "."
#                     REQUIRES ${pri_req})
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

idf_component_register(
    SRCS "src/st7789.c"
    INCLUDE_DIRS "include"
    REQUIRES driver freertos esp_common esp_lcd
)

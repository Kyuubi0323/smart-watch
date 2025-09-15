idf_component_register(
    SRCS "src/qmi8658c.c"
    INCLUDE_DIRS "include" "."
    REQUIRES driver freertos esp_common
)

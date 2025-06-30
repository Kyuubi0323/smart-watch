# Timber ESP-IDF Component
file(GLOB timber_src "*.cpp" "*.c")

set(timber_req espressif__arduino-esp32)

idf_component_register(SRCS ${timber_src}
                       INCLUDE_DIRS "."
                       REQUIRES ${timber_req})

# Set C++ standard for this component
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_11)
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

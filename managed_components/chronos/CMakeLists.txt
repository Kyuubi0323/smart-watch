# ChronosESP32 ESP-IDF Component
file(GLOB_RECURSE chronos_src "src/*.cpp" "src/*.c")

set(chronos_req espressif__arduino-esp32 lwip)

idf_component_register(SRCS ${chronos_src}
                       INCLUDE_DIRS "src"
                       REQUIRES ${chronos_req})

# Set C++ standard for this component
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_11)
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

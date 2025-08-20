# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/components
    ${CMAKE_CURRENT_LIST_DIR}/components/apps/qmi8658c
    ${CMAKE_CURRENT_LIST_DIR}/components/apps/navigation
    ${CMAKE_CURRENT_LIST_DIR}/components/apps/contacts
    ${CMAKE_CURRENT_LIST_DIR}/components/apps/piobot
)

# Optional build configuration for board type
# Set via: idf.py -DBOARD_CONFIG=ESPC3_1_69 build
if(DEFINED BOARD_CONFIG)
    add_compile_definitions(${BOARD_CONFIG}=1)
    message(STATUS "Board configuration: ${BOARD_CONFIG}")
else()
    # Default to current configuration (ESP32-S3 1.69")
    add_compile_definitions(ESPS3_1_69=1)
    message(STATUS "Using default board configuration: ESPS3_1_69")
endif()

project(smart-watch)

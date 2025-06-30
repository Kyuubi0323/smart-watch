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
                            
                        


project(smart-watch)

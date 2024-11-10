idf_component_register(SRCS "app_main.c" "img_test.c"                         
                       INCLUDE_DIRS ".")
                       
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

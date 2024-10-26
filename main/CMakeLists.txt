FILE(GLOB_RECURSE app_src
        ${CMAKE_SOURCE_DIR}/main/*.cpp
)

idf_component_register(SRCS ${app_src} INCLUDE_DIRS "." REQUIRES core-esp32 led_strip mp3)

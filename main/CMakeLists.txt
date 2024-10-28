FILE(GLOB_RECURSE app_src
        ${CMAKE_SOURCE_DIR}/main/*.cpp
        ${CMAKE_SOURCE_DIR}/main/led/*.cpp
        ${CMAKE_SOURCE_DIR}/main/servo/*.cpp
)

idf_component_register(SRCS ${app_src} INCLUDE_DIRS "." REQUIRES mp3 core-esp32 led-strip-service)

spiffs_create_partition_image(storage ../data)
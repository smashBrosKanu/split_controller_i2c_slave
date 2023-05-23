cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
include_directories(${CMAKE_SOURCE_DIR}/i2c_slave/include)

project(your_project)

pico_sdk_init()

add_executable(your_project
    main.cpp
)

add_dependencies(your_project tinyusb_device)

pico_enable_stdio_usb(your_project 1)
pico_enable_stdio_uart(your_project 0)

target_link_libraries(your_project
    pico_stdlib
    hardware_i2c
    i2c_slave
)

add_subdirectory(i2c_slave)

pico_enable_stdio_usb(i2c_slave 1)
pico_enable_stdio_uart(your_project 0)

pico_add_extra_outputs(your_project)

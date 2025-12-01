cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(scooter C CXX ASM)
pico_sdk_init()

add_executable(scooter
    main.c
)

# Link against the libraries you use
target_link_libraries(scooter
    pico_stdlib
    hardware_gpio
    hardware_adc
    hardware_timer
    hardware_i2c
)

pico_add_extra_outputs(scooter)


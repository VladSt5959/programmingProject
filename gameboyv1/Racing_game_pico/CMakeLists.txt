cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)
include(pico_sdk_import.cmake)

project(Racing_game C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(Racing_game
    demo/main.c
    src/racing_game.c
    src/ili9341.c
    src/buttons.c
)

target_include_directories(Racing_game PRIVATE include)

target_link_libraries(Racing_game
    pico_stdlib
    hardware_spi
    hardware_gpio
    pico_rand
)

pico_add_extra_outputs(Racing_game)
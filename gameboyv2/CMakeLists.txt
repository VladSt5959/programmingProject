# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)

set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico2_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(gameboyv2 C CXX ASM)

pico_sdk_init()

add_subdirectory(libs/ili9341)
add_subdirectory(libs/gfx)

add_executable(gameboyv2
    gameboyv2.c
    menu.c
    settings.c
    games/snake.c
    games/platformer.c
    libs/ownLibs/btnReader.c
)

target_include_directories(gameboyv2 PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(gameboyv2
    pico_stdlib
    hardware_spi
    hardware_adc
    ili9341
    gfx
)
pico_enable_stdio_usb(gameboyv2 1)
pico_enable_stdio_uart(gameboyv2 0)
pico_set_program_name(gameboyv2 "gameboyv2")
pico_set_program_version(gameboyv2 "0.1")

# pico_enable_stdio_uart(gameboyv2 0)
# pico_enable_stdio_usb(gameboyv2 0)

pico_add_extra_outputs(gameboyv2)

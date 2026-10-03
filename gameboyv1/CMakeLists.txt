cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pico VS Code extension setup
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


# IMPORTANT:
# Pico 2 W = pico2_w
set(PICO_BOARD pico2_w CACHE STRING "Board type")


# Import Pico SDK
include(pico_sdk_import.cmake)

project(gameboyv1 C CXX ASM)

# Initialize Pico SDK
pico_sdk_init()

add_subdirectory(libs/ili9341)
add_subdirectory(libs/gfx)

add_executable(gameboyv1
    gameboyv1.c
)

target_link_libraries(gameboyv1
    pico_stdlib
    hardware_spi
    ili9341
    gfx
)
# pico_enable_stdio_usb(your_target 1)
# pico_enable_stdio_uart(your_target 0)




pico_set_program_name(gameboyv1 "gameboyv1")
pico_set_program_version(gameboyv1 "0.1")


# USB/UART output
pico_enable_stdio_uart(gameboyv1 0)
pico_enable_stdio_usb(gameboyv1 0)


# Include project directory
target_include_directories(gameboyv1 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)




# Generate .uf2, .bin etc.
pico_add_extra_outputs(gameboyv1)
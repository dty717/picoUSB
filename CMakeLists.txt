cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hostUSB C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(hostUSB
        src/main.c
)

if (DEFINED ENV{hostUSB_LED})
        message("hostUSB_LED is defined as " $ENV{hostUSB_LED})
        target_compile_definitions(hostUSB PRIVATE hostUSB_LED=$ENV{hostUSB_LED})
endif()

set(DBG_PIN_COUNT=4)

target_include_directories(hostUSB PRIVATE src)

target_link_libraries(hostUSB PRIVATE pico_stdlib pico_unique_id pico_multicore hardware_pio hardware_resets hardware_irq)

# enable usb output, disable uart output
pico_enable_stdio_usb(hostUSB 0)
pico_enable_stdio_uart(hostUSB 1)

pico_add_extra_outputs(hostUSB)
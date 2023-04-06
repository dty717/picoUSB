cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_usb C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(pico_usb
        src/led.c
        src/main.c
        src/usb_descriptors.c
        src/cdc_uart.c
        src/get_serial.c
)

if (DEFINED ENV{PICO_USB_LED})
        message("PICO_USB_LED is defined as " $ENV{PICO_USB_LED})
        target_compile_definitions(pico_usb PRIVATE PICO_USB_LED=$ENV{PICO_USB_LED})
endif()

set(DBG_PIN_COUNT=4)

target_include_directories(pico_usb PRIVATE src)

target_link_libraries(pico_usb PRIVATE pico_stdlib pico_unique_id pico_multicore tinyusb_device tinyusb_board hardware_pio)

# enable usb output, disable uart output
pico_enable_stdio_usb(pico_usb 1)
pico_enable_stdio_uart(pico_usb 0)

pico_add_extra_outputs(pico_usb)

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
pico_generate_pio_header(pico_usb ${CMAKE_CURRENT_LIST_DIR}/src/uart_rx.pio)
pico_generate_pio_header(pico_usb ${CMAKE_CURRENT_LIST_DIR}/src/uart_tx.pio)

if (DEFINED ENV{PICO_USB_LED})
        message("PICO_USB_LED is defined as " $ENV{PICO_USB_LED})
        target_compile_definitions(pico_usb PRIVATE PICO_USB_LED=$ENV{PICO_USB_LED})
endif()

set(DBG_PIN_COUNT=4)

target_include_directories(pico_usb PRIVATE src)

target_link_libraries(pico_usb PRIVATE pico_stdlib pico_unique_id pico_multicore hardware_gpio hardware_irq hardware_pio tinyusb_device tinyusb_board)

# enable usb output, disable uart output
pico_enable_stdio_usb(pico_usb 0)
pico_enable_stdio_uart(pico_usb 0)

pico_add_extra_outputs(pico_usb)

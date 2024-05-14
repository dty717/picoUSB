
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/Users/18751/Desktop/work/Electronics/rasberrayPi/pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)


if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()


project(hostUSB C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(hostUSB
        src/hid_app.c
        src/main.c
        src/msc_app.c
        src/cdc_app.c
        # src/rp2040_usb.c
        # src/hcd_rp2040.c
)

if (DEFINED ENV{hostUSB_LED})
        message("hostUSB_LED is defined as " $ENV{hostUSB_LED})
        target_compile_definitions(hostUSB PRIVATE hostUSB_LED=$ENV{hostUSB_LED})
endif()

set(DBG_PIN_COUNT=4)

target_include_directories(hostUSB PRIVATE src)

target_link_libraries(hostUSB PRIVATE pico_stdlib tinyusb_host tinyusb_board pico_multicore hardware_flash)

# enable usb output, disable uart output
pico_enable_stdio_usb(hostUSB 0)
pico_enable_stdio_uart(hostUSB 1)

pico_add_extra_outputs(hostUSB)



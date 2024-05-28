#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
// #define USE_485
#ifdef USE_485
    #define UART0_EN_PIN 2
#endif
#define UART0_TX_PIN 0
#define UART0_RX_PIN 1
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define firstLine 0

#define UsingDualUART

#ifdef UsingDualUART
    #ifdef USE_485
        #define UART1_EN_PIN 3
    #endif
    #define UART1_TX_PIN 4
    #define UART1_RX_PIN 5
#endif
#endif


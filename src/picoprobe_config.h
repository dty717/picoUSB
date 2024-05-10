/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef PICOPROBE_H_
#define PICOPROBE_H_

#if false
#define picoprobe_info(format,args...) printf(format, ## args)
#else
#define picoprobe_info(format,...) ((void)0)
#endif


#if false
#define picoprobe_debug(format,args...) printf(format, ## args)
#else
#define picoprobe_debug(format,...) ((void)0)
#endif

#if false
#define picoprobe_dump(format,args...) printf(format, ## args)
#else
#define picoprobe_dump(format,...) ((void)0)
#endif


#define len0 1024
extern uint16_t num_read0;
extern uint8_t data0[len0];

#define len1 1024
extern uint16_t num_read1;
extern uint8_t data1[len1];

#define Listener

#define UsingLED

#ifdef UsingLED
    #define LED1_PIN 11
    #define LED2_PIN 12
    #define LED3_PIN 13
    #define LED4_PIN 14
#endif

#define UsingKey

#ifdef UsingKey
    #define KEY1_PIN 6     //USB0 to USB1
    #define KEY2_PIN 7     //USB0 to UART0
    #define KEY3_PIN 8     //USB1 to UART1
    #define KEY4_PIN 9     //UART0 to UART1
    // extern int key1;
    // extern int key2;
    // extern int key3;
    // extern int key4;
#endif

#define UsingUART

#ifdef UsingUART
    // #define USE_485
    #ifdef USE_485
        #define UART0_EN_PIN                    2
        #define UART1_EN_PIN                    3
    #endif
    #define UART0_TX_PIN                    0
    #define UART0_RX_PIN                    1
    #define UART1_TX_PIN                    4
    #define UART1_RX_PIN                    5
    #define BAUD_RATE                  115200
    #define DATA_BITS                       8
    #define STOP_BITS                       1
    #define PARITY           UART_PARITY_NONE
#endif
// This is an idiosyncrasy of the C# API fixing for that C# code does not set SerialPort.DtrEnable(DTR:Data Terminal Ready), with .connected not be true
// #define checkUsbConnecting

#define firstLine 0
#define secondLine 1

// LED config
#ifndef PICOPROBE_LED


#ifndef PICO_DEFAULT_LED_PIN
#error PICO_DEFAULT_LED_PIN is not defined, run PICOPROBE_LED=<led_pin> cmake
#elif PICO_DEFAULT_LED_PIN == -1
#error PICO_DEFAULT_LED_PIN is defined as -1, run PICOPROBE_LED=<led_pin> cmake
#else
#define PICOPROBE_LED PICO_DEFAULT_LED_PIN
#endif

#endif

#endif

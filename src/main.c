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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "tusb_config.h"
#include "picoprobe_config.h"
#include "cdc_uart.h"
#include "get_serial.h"
#include "led.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

uint16_t num_read0 = 0;
uint8_t data0[len0];

uint16_t num_read1 = 0;
uint8_t data1[len1];

void print_buf(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02X", buf[i]);
        if (i % 16 == 15)
            printf("\n");
        else
            printf(" ");
    }
}

#ifdef USE_485
    uint haswritten0 = 0;
    uint haswritten1 = 0;
    volatile uint32_t bit_rate = BAUD_RATE;
#endif


#ifdef UsingUART
    // uart0 RX interrupt handler
    void on_uart0_rx()
    {
        while (uart_is_readable(uart0))
        {
            uint8_t ch = uart_getc(uart0);
            tud_cdc_n_write(secondLine, &ch, 1);
        }
        tud_cdc_n_write_flush(secondLine);
    }
    // uart1 RX interrupt handler
    void on_uart1_rx() {
        while (uart_is_readable(uart1))
        {
            uint8_t ch = uart_getc(uart1);
            tud_cdc_n_write(firstLine, &ch, 1);
        }
        tud_cdc_n_write_flush(firstLine);
    } 
#endif




void core1_entry()
{
    while (true)
    {
        if (tud_cdc_n_available(secondLine))
        {
            #ifdef checkUsbConnecting
                if (tud_cdc_n_connected(secondLine))
            #endif
            {
                num_read1 = tud_cdc_n_read(secondLine, data1, len1);

                if (num_read1)
                {
                    #ifdef UsingUART
                        #ifdef USE_485
                            haswritten1 = 1;
                            gpio_put(UART1_EN_PIN, 1);
                            sleep_us(50);
                        #endif
                            uart_write_blocking(uart1, data1, num_read1);
                        #ifdef USE_485
                            sleep_us(10304 * 1000 * (num_read1 < 33 ? num_read1 + 1 : 33) / bit_rate);
                            gpio_put(UART1_EN_PIN, 0);
                        #endif
                    #else
                        #ifdef checkUsbConnecting
                            if (tud_cdc_n_connected(secondLine))
                        #endif
                        {
                            tud_cdc_n_write(secondLine, data0, num_read0);
                            tud_cdc_n_write_flush(secondLine);
                        }
                    #endif
                }
            }
        }
    }
}


int main(void) {
    testFun();
    stdio_init_all();
    board_init();
    usb_serial_init();
    init_usb_hardware();
    tusb_init();
    post_usb_init();
    led_init();

    #ifdef UsingUART
        #ifdef USE_485
            const uint uart0_EN = UART0_EN_PIN;
            const uint uart1_EN = UART1_EN_PIN;
            gpio_init(uart0_EN);
            gpio_init(uart1_EN);
            gpio_set_dir(uart0_EN, GPIO_OUT);
            gpio_set_dir(uart1_EN, GPIO_OUT);
            gpio_put(uart0_EN, 0);
            gpio_put(uart1_EN, 0);
        #endif
        uart_init(uart0, BAUD_RATE);
        uart_init(uart1, BAUD_RATE);

        // Set the TX and RX pins by using the function select on the GPIO
        // Set datasheet for more information on function select
        gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
        gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);

        gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
        gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

        // Set UART flow control CTS/RTS, we don't want these, so turn them off
        uart_set_hw_flow(uart0, false, false);
        uart_set_hw_flow(uart1, false, false);

        // Set our data format
        uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);
        uart_set_format(uart1, DATA_BITS, STOP_BITS, PARITY);

        // Turn off FIFO's - we want to do this character by character
        uart_set_fifo_enabled(uart0, false);
        uart_set_fifo_enabled(uart1, false);

        // Set up a RX interrupt
        // We need to set up the handler first
        // Select correct interrupt for the UART we are using
        // And set up and enable the interrupt handlers
        irq_set_exclusive_handler(UART0_IRQ, on_uart0_rx);
        irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
        irq_set_enabled(UART0_IRQ, true);
        irq_set_enabled(UART1_IRQ, true);

        // Now enable the UART to send interrupts - RX only
        uart_set_irq_enables(uart0, true, false);
        uart_set_irq_enables(uart1, true, false);
    #endif


    picoprobe_info("Welcome to Picoprobe!\n");
    #ifdef Listener
        multicore_launch_core1(core1_entry);
    #endif
    while (1) 
    {
        if (tud_cdc_n_available(firstLine))
        {
            #ifdef checkUsbConnecting
                if (tud_cdc_n_connected(secondLine))
            #endif
                {
                num_read0 = tud_cdc_n_read(firstLine, data0, len0);

                if (num_read0)
                {
                    #ifdef UsingUART
                        #ifdef USE_485
                            haswritten0 = 1;
                            gpio_put(UART0_EN_PIN, 1);
                            sleep_us(50);
                        #endif
                            uart_write_blocking(uart0, data0, num_read0);
                        #ifdef USE_485
                            sleep_us(10304 * 1000 * (num_read0 < 33 ? num_read0 + 1 : 33) / bit_rate);
                            gpio_put(UART0_EN_PIN, 0);
                        #endif
                    #else
                        #ifdef checkUsbConnecting
                            if (tud_cdc_n_connected(secondLine))
                        #endif
                        {
                            tud_cdc_n_write(secondLine, data0, num_read0);
                            tud_cdc_n_write_flush(secondLine);
                        }
                    #endif
                }
            }
        }
        #ifdef Listener
            led_task();
        #else
            led_task();
        #endif
    }

    return 0;
}
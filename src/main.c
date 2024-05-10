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
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"


#ifdef UsingKey
    int key1, key2, key3, key4;
    void gpio_callback(uint gpio, uint32_t events)
    {
        if (gpio == KEY1_PIN)
        {
            key1 = !gpio_get(gpio);
            #ifdef UsingLED
                gpio_put(LED1_PIN, key1);
            #endif            
            // if(events&0b100){
            //     key1 = 0;
            //     #ifdef UsingLED
            //         gpio_put(LED1_PIN, key1);
            //     #endif
            // }else if(events&0b1000){
            //     key1 = 1;
            //     #ifdef UsingLED
            //         gpio_put(LED1_PIN, key1);
            //     #endif
            // }
        }
        if (gpio == KEY2_PIN)
        {
            key2 = !gpio_get(gpio);
            #ifdef UsingLED
                gpio_put(LED2_PIN, key2);
            #endif  
            // if(events&0b100){
            //     key2 = 0;
            //     #ifdef UsingLED
            //         gpio_put(LED2_PIN, key2);
            //     #endif
            // }else if(events&0b1000){
            //     key2 = 1;
            //     #ifdef UsingLED
            //         gpio_put(LED2_PIN, key2);
            //     #endif
            // }
        }
        if (gpio == KEY3_PIN)
        {
            key3 = !gpio_get(gpio);
            #ifdef UsingLED
                gpio_put(LED3_PIN, key3);
            #endif
            // if(events&0b100){
            //     key3 = 0;
            //     #ifdef UsingLED
            //         gpio_put(LED3_PIN, key3);
            //     #endif
            // }else if(events&0b1000){
            //     key3 = 1;
            //     #ifdef UsingLED
            //         gpio_put(LED3_PIN, key3);
            //     #endif
            // }
        }
        if (gpio == KEY4_PIN)
        {
            key4 = !gpio_get(gpio);
            #ifdef UsingLED
                gpio_put(LED4_PIN, key4);
            #endif
            // if(events&0b100){
            //     key4 = 0;
            //     #ifdef UsingLED
            //         gpio_put(LED4_PIN, key4);
            //     #endif
            // }else if(events&0b1000){
            //     key4 = 1;
            //     #ifdef UsingLED
            //         gpio_put(LED4_PIN, key4);
            //     #endif
            // }
        }
    }
#endif

#ifdef UsingPIO
    #include "hardware/pio.h"
    #include "uart_rx.pio.h"
    #include "uart_tx.pio.h"

    PIO pio_rx;
    uint8_t sm_rx;
    int8_t pio_rx_irq;
    uint offset_rx;

    PIO pio_tx;
    uint8_t sm_tx;
    uint offset_tx;

    // IRQ called when the pio_rx fifo is not empty, i.e. there are some characters on the uart
    // This needs to run as quickly as possible or else you will lose characters (in particular don't printf!)
    void pio_rx_irq_func(void)
    {
        while (!pio_sm_is_rx_fifo_empty(pio_rx, sm_rx))
        {
            char ch = uart_rx_program_getc_without_fifo(pio_rx, sm_rx);
            // putchar(ch); // Display character in the console
            #ifdef UsingKey
                if(key2){
                    tud_cdc_n_write(firstLine, &ch, 1);
                }
                if(key4){
                    #ifdef USE_485
                        haswritten1 = 1;
                        gpio_put(UART1_EN_PIN, 1);
                        sleep_us(50);
                    #endif
                    uart_putc(uart1,ch);
                    #ifdef USE_485
                        sleep_us(10304 * 1000 * 2 / bit_rate);
                        gpio_put(UART1_EN_PIN, 0);
                    #endif
                }
            #else
                tud_cdc_n_write(firstLine, &ch, 1);
            #endif
        }
        #ifdef UsingKey
            if(key2){
                tud_cdc_n_write_flush(firstLine);
            }
        #else
            tud_cdc_n_write_flush(firstLine);
        #endif
    }

    // Find a free pio_rx and state machine and load the program into it.
    // Returns false if this fails
    bool init_pio(const pio_program_t *program, PIO *pio_hw, uint *offset)
    {
        // Find a free pio_rx
        *pio_hw = pio1;
        if (!pio_can_add_program(*pio_hw, program))
        {
            *pio_hw = pio0;
            if (!pio_can_add_program(*pio_hw, program))
            {
                *offset = -1;
                return false;
            }
        }
        *offset = pio_add_program(*pio_hw, program);
        return true;
    }
#endif

uint16_t num_read0 = 0;
uint8_t data0[len0];

uint16_t num_read1 = 0;
uint8_t data1[len1];

#ifdef UsingUART
    #ifdef USE_485
        uint haswritten0 = 0;
        uint haswritten1 = 0;
        volatile uint32_t bit_rate = BAUD_RATE;
    #endif
    
    // uart0 RX interrupt handler
    void on_uart0_rx()
    {
        while (uart_is_readable(uart0))
        {
            uint8_t ch = uart_getc(uart0);
            #ifdef UsingKey
                if(key2){
                    tud_cdc_n_write(firstLine, &ch, 1);
                }
                if(key4){
                    #ifdef USE_485
                        haswritten1 = 1;
                        gpio_put(UART1_EN_PIN, 1);
                        sleep_us(50);
                    #endif
                    uart_putc(uart1,ch);
                    #ifdef USE_485
                        sleep_us(10304 * 1000 * 2 / bit_rate);
                        gpio_put(UART1_EN_PIN, 0);
                    #endif
                }
            #else
                tud_cdc_n_write(firstLine, &ch, 1);
            #endif
        }
        #ifdef UsingKey
            if(key2){
                tud_cdc_n_write_flush(firstLine);
            }
        #else
            tud_cdc_n_write_flush(firstLine);
        #endif
    }

    // uart1 RX interrupt handler
    void on_uart1_rx() {
        while (uart_is_readable(uart1))
        {
            uint8_t ch = uart_getc(uart1);
            #ifdef UsingKey
                if(key3){
                    tud_cdc_n_write(secondLine, &ch, 1);
                }
                if(key4){
                    #ifdef USE_485
                        haswritten0 = 1;
                        gpio_put(UART0_EN_PIN, 1);
                        sleep_us(50);
                    #endif
                    #ifdef UsingPIO
                        uart_tx_program_putc(pio_tx, sm_tx, ch);
                    #else
                        uart_putc(uart0,ch);
                    #endif
                    #ifdef USE_485
                        sleep_us(10304 * 1000 * 2 / bit_rate);
                        gpio_put(UART0_EN_PIN, 0);
                    #endif
                }
            #else
                tud_cdc_n_write(secondLine, &ch, 1);
            #endif
        }
        #ifdef UsingKey
            if(key3){
                tud_cdc_n_write_flush(secondLine);
            }
        #else
            tud_cdc_n_write_flush(secondLine);
        #endif
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
                    #ifdef UsingKey
                        if(key1){
                            #ifdef checkUsbConnecting
                                if (tud_cdc_n_connected(firstLine))
                            #endif
                            {
                                tud_cdc_n_write(firstLine, data1, num_read1);
                                tud_cdc_n_write_flush(firstLine);
                            }
                        }
                        if(key3){
                            #ifdef USE_485
                                haswritten1 = 1;
                                gpio_put(UART1_EN_PIN, 1);
                                sleep_us(50);
                            #endif
                                uart_write_blocking(uart1, data1, num_read1);
                                // uart_write_blocking(uart0, data1, num_read1);
                            #ifdef USE_485
                                sleep_us(10304 * 1000 * (num_read1 < 33 ? num_read1 + 1 : 33) / bit_rate);
                                gpio_put(UART1_EN_PIN, 0);
                            #endif
                        }
                    #else
                        #ifdef UsingUART
                            #ifdef USE_485
                                haswritten1 = 1;
                                gpio_put(UART1_EN_PIN, 1);
                                sleep_us(50);
                            #endif
                                uart_write_blocking(uart1, data1, num_read1);
                                // uart_write_blocking(uart0, data1, num_read1);
                            #ifdef USE_485
                                sleep_us(10304 * 1000 * (num_read1 < 33 ? num_read1 + 1 : 33) / bit_rate);
                                gpio_put(UART1_EN_PIN, 0);
                            #endif
                        #else
                            #ifdef checkUsbConnecting
                                if (tud_cdc_n_connected(firstLine))
                            #endif
                            {
                                tud_cdc_n_write(firstLine, data1, num_read1);
                                tud_cdc_n_write_flush(firstLine);
                            }
                        #endif
                    #endif
                }
            }
        }
    }
}

int main(void) {
    stdio_init_all();
    board_init();
    usb_serial_init();
    init_usb_hardware();
    tusb_init();
    post_usb_init();
    led_init();

    #ifdef UsingPIO
        // Set up the state machine we're going to use to receive them.
        // In real code you need to find a free pio_rx and state machine in case pio_rx resources are used elsewhere
        if (!init_pio(&uart_rx_program, &pio_rx, &offset_rx))
        {
            panic("failed to setup pio_rx");
        }
        if (!init_pio(&uart_tx_program, &pio_tx, &offset_tx))
        {
            panic("failed to setup pio_tx");
        }
        sm_tx = pio_claim_unused_sm(pio_tx, false);
        sm_rx = pio_claim_unused_sm(pio_rx, false);
        // sm_gps_tx = pio_claim_unused_sm(pio_tx, false);
        // sm_gps_rx = pio_claim_unused_sm(pio_rx, false);
    #endif

    #ifdef UsingLED
        const uint LED1 = LED1_PIN;
        const uint LED2 = LED2_PIN;
        const uint LED3 = LED3_PIN;
        const uint LED4 = LED4_PIN;
        gpio_init(LED1);
        gpio_init(LED2);
        gpio_init(LED3);
        gpio_init(LED4);
        gpio_set_dir(LED1, GPIO_OUT);
        gpio_set_dir(LED2, GPIO_OUT);
        gpio_set_dir(LED3, GPIO_OUT);
        gpio_set_dir(LED4, GPIO_OUT);
    #endif

    #ifdef UsingKey
        const uint key1Pin = KEY1_PIN;
        const uint key2Pin = KEY2_PIN;
        const uint key3Pin = KEY3_PIN;
        const uint key4Pin = KEY4_PIN;
        
        gpio_init(key1Pin);
        gpio_init(key2Pin);
        gpio_init(key3Pin);
        gpio_init(key4Pin);

        gpio_set_dir(key1Pin, GPIO_IN);
        gpio_set_dir(key2Pin, GPIO_IN);
        gpio_set_dir(key3Pin, GPIO_IN);
        gpio_set_dir(key4Pin, GPIO_IN);
        key1 = !gpio_get(KEY1_PIN);
        key2 = !gpio_get(KEY2_PIN);
        key3 = !gpio_get(KEY3_PIN);
        key4 = !gpio_get(KEY4_PIN);
        #ifdef UsingLED
            gpio_put(LED1,key1);
            gpio_put(LED2,key2);
            gpio_put(LED3,key3);
            gpio_put(LED4,key4);
        #endif
        gpio_set_irq_enabled(key1Pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        gpio_set_irq_enabled(key2Pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        gpio_set_irq_enabled(key3Pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        gpio_set_irq_enabled(key4Pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        gpio_set_irq_callback(&gpio_callback);
        irq_set_enabled(IO_IRQ_BANK0, true);
        // gpio_set_irq_enabled_with_callback(key1Pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        // gpio_set_irq_enabled_with_callback(key2Pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        // gpio_set_irq_enabled_with_callback(key3Pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        // gpio_set_irq_enabled_with_callback(key4Pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    #endif

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
        #ifdef UsingPIO
            uart_tx_program_init(pio_tx, sm_tx, offset_tx, PIO_TX_PIN, BAUD_RATE);
            uart_rx_program_init(pio_rx, sm_rx, offset_rx, PIO_RX_PIN, BAUD_RATE);
            // uart_tx_program_init(pio_tx, sm_gps_tx, offset_tx, PIO_GPS_TX_PIN, BAUD_RATE);
            // uart_rx_program_init(pio_rx, sm_gps_rx, offset_rx, PIO_GPS_RX_PIN, BAUD_RATE);
            // Find a free irq
            pio_rx_irq = (pio_rx == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0;
            // Enable interrupt
            irq_set_exclusive_handler(pio_rx_irq, pio_rx_irq_func);
            irq_set_enabled(pio_rx_irq, true); // Enable the IRQ
            const uint irq_index = pio_rx_irq - ((pio_rx == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0); // Get index of the IRQ
            pio_set_irqn_source_enabled(pio_rx, irq_index, pis_sm0_rx_fifo_not_empty + sm_rx, true); // Set pio_rx to tell us when the FIFO is NOT empty

            // // Find a free irq
            // pio_gps_rx_irq = (pio_rx == pio0) ? PIO0_IRQ_1 : PIO1_IRQ_1;
            // // Enable interrupt
            // irq_set_exclusive_handler(pio_gps_rx_irq, pio_gps_rx_irq_func);
            // irq_set_enabled(pio_gps_rx_irq, true); // Enable the IRQ
            // const uint gps_irq_index = pio_gps_rx_irq - ((pio_rx == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0); // Get index of the IRQ
            // pio_set_irqn_source_enabled(pio_rx, gps_irq_index, pis_sm0_rx_fifo_not_empty + sm_gps_rx, true); // Set pio_rx to tell us when the FIFO is NOT empty
        #else
            uart_init(uart0, BAUD_RATE);
            gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
            gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
            uart_set_hw_flow(uart0, false, false);
            uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);
            uart_set_fifo_enabled(uart0, false);
            irq_set_exclusive_handler(UART0_IRQ, on_uart0_rx);
            irq_set_enabled(UART0_IRQ, true);
            uart_set_irq_enables(uart0, true, false);
        #endif
        uart_init(uart1, BAUD_RATE);

        // Set the TX and RX pins by using the function select on the GPIO
        // Set datasheet for more information on function select
        gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
        gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

        // Set UART flow control CTS/RTS, we don't want these, so turn them off
        uart_set_hw_flow(uart1, false, false);

        // Set our data format
        uart_set_format(uart1, DATA_BITS, STOP_BITS, PARITY);

        // Turn off FIFO's - we want to do this character by character
        uart_set_fifo_enabled(uart1, false);

        // Set up a RX interrupt
        // We need to set up the handler first
        // Select correct interrupt for the UART we are using
        // And set up and enable the interrupt handlers
        irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
        irq_set_enabled(UART1_IRQ, true);

        // Now enable the UART to send interrupts - RX only
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
                if (tud_cdc_n_connected(firstLine))
            #endif
                {
                num_read0 = tud_cdc_n_read(firstLine, data0, len0);
                if (num_read0)
                {
                    #ifdef UsingKey
                        if(key1){
                            #ifdef checkUsbConnecting
                                if (tud_cdc_n_connected(secondLine))
                            #endif
                            {
                                tud_cdc_n_write(secondLine, data0, num_read0);
                                tud_cdc_n_write_flush(secondLine);
                            }
                        }
                        if(key2){
                            #ifdef USE_485
                                haswritten0 = 1;
                                gpio_put(UART0_EN_PIN, 1);
                                sleep_us(50);
                            #endif
                                #ifdef UsingPIO
                                    uart_tx_program_write(pio_tx, sm_tx, data0, num_read0);
                                #else
                                    uart_write_blocking(uart0, data0, num_read0);
                                #endif
                                // uart_write_blocking(uart1, data0, num_read0);
                            #ifdef USE_485
                                sleep_us(10304 * 1000 * (num_read0 < 33 ? num_read0 + 1 : 33) / bit_rate);
                                gpio_put(UART0_EN_PIN, 0);
                            #endif
                        }
                    #else
                        #ifdef UsingUART
                            #ifdef USE_485
                                haswritten0 = 1;
                                gpio_put(UART0_EN_PIN, 1);
                                sleep_us(50);
                            #endif
                                #ifdef UsingPIO
                                    uart_tx_program_write(pio_tx, sm_tx, data0, num_read0);
                                #else
                                    uart_write_blocking(uart0, data0, num_read0);
                                #endif
                                // uart_write_blocking(uart1, data0, num_read0);
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
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

void core1_entry()
{
    while (true)
    {
        if(secondLineRecFlag){
            secondLineRecFlag = 0;
            num_read1 = tud_cdc_n_read(1, data1, len1);
            if (num_read1)
            {
                if (tud_cdc_n_connected(0))
                {
                    tud_cdc_n_write(0, data1, num_read1);
                    tud_cdc_n_write_flush(0);
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

    picoprobe_info("Welcome to Picoprobe!\n");
    #ifdef Listener
        multicore_launch_core1(core1_entry);
    #endif
    while (1) 
    {
        if (firstLineRecFlag)
        {
            firstLineRecFlag = 0;
            num_read0 = tud_cdc_n_read(0, data0, len0);
            if (num_read0)
            {
                if (tud_cdc_n_connected(1))
                {
                    tud_cdc_n_write(1, data0, num_read0);
                    tud_cdc_n_write_flush(1);
                }
            }
        }
        #ifdef Listener
        #else
            led_task();
        #endif
    }

    return 0;
}
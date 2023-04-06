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

#include "pico/stdlib.h"

#include "tusb.h"

#include "picoprobe_config.h"

volatile uint8_t firstLineRecFlag = 0;
volatile uint8_t secondLineRecFlag = 0;

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *line_coding)
{
    // printf("New baud rate %d\n", line_coding->bit_rate);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allows us to perform remote wakeup
// USB Specs: Within 7ms, device must draw an average current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    printf("tud_suspend_cb remote_wakeup_en%d\r\n", remote_wakeup_en);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    printf("tud_resume_cb");
}

// Invoked when cdc when line state changed e.g connected/disconnected
// Use to reset to DFU when disconnect with 1200 bps
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    // printf("tud_cdc_line_state_cb itf:%d,dtr:%d,rts:%d\r\n", itf, dtr, rts);
    (void)itf;  // interface ID, not used
}

void tud_cdc_rx_cb(uint8_t itf) {
    (void)itf;
    // Workaround for "press any key to enter REPL" response being delayed on espressif.
    // Wake main task when any key is pressed.
    switch (itf)
    {
        case firstLine:
            firstLineRecFlag = 1;
            /* code */
            break;
        case secondLine:
            secondLineRecFlag = 1;
            /* code */
            break;
        default:
            printf("tud_cdc_rx_cb:%d\r\n",itf);
            break;
    }
    // port_wake_main_task();
}
/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Federico Zuccardi Merli
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

#include <stdint.h>
#include "pico.h"
#include "pico/unique_id.h"
#include "get_serial.h"

#include "usbd.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

/* C string for iSerialNumber in USB Device Descriptor, two chars per byte + terminating NUL */
char usb_serial[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

/* Why a uint8_t[8] array inside a struct instead of an uint64_t an inquiring mind might wonder */
static pico_unique_board_id_t uID;

void usb_serial_init(void)
{
    pico_get_unique_board_id(&uID);

    for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2; i++)
    {
        /* Byte index inside the uid array */
        int bi = i / 2;
        /* Use high nibble first to keep memory order (just cosmetics) */
        uint8_t nibble = (uID.id[bi] >> 4) & 0x0F;
        uID.id[bi] <<= 4;
        /* Binary to hex digit */
        usb_serial[i] = nibble < 10 ? nibble + '0' : nibble + 'A' - 10;
    }
}

void usb_background_schedule(void)
{
    if (tusb_inited())
    {
        tud_task();
        tud_cdc_write_flush();
    }
}

void usb_irq_handler(int instance)
{
    // printf("usb_irq_handler %d\r\n",instance);
    tud_int_handler(instance);
    usb_background_schedule();
}

void init_usb_hardware(void) {
}

void _usb_irq_wrapper(void) {
    usb_irq_handler(0);
}


void post_usb_init(void) {
    irq_set_enabled(USBCTRL_IRQ, false);

    irq_handler_t usb_handler = irq_get_exclusive_handler(USBCTRL_IRQ);
    if (usb_handler) {
        irq_remove_handler(USBCTRL_IRQ, usb_handler);
    }
    irq_set_exclusive_handler(USBCTRL_IRQ, _usb_irq_wrapper);

    irq_set_enabled(USBCTRL_IRQ, true);

    // There is a small window where the USB interrupt may be handled by the
    // pico-sdk instead of CircuitPython. If that is the case, then we'll have
    // USB events to process that we didn't queue up a background task for. So,
    // queue one up here even if we might not have anything to do.
    usb_background_schedule();
}

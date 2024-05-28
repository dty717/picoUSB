/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bsp/board.h"
#include "tusb.h"
#include "tusb_config.h"
#include "config.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
void led_blinking_task(void);

size_t get_console_inputs(uint8_t* buf, size_t bufsize)
{
  size_t count = 0;
  while (count < bufsize)
  {
    int ch = board_getchar();
    if ( ch <= 0 ) break;

    buf[count] = (uint8_t) ch;
    count++;
  }

  return count;
}

void cdc_app_task(void)
{
  uint8_t buf[64+1]; // +1 for extra null character
  uint32_t const bufsize = sizeof(buf)-1;

  uint32_t count = get_console_inputs(buf, bufsize);
  buf[count] = 0;

  // loop over all mounted interfaces
  for(uint8_t idx=0; idx<CFG_TUH_CDC; idx++)
  {
    if ( tuh_cdc_mounted(idx) )
    {
      // console --> cdc interfaces
      if (count)
      {
        tuh_cdc_write(idx, buf, count);
        tuh_cdc_write_flush(idx);
      }
    }
  }
}

// Invoked when received new data
void tuh_cdc_rx_cb(uint8_t idx)
{
  uint8_t buf[64+1]; // +1 for extra null character
  uint32_t const bufsize = sizeof(buf)-1;

  // forward cdc interfaces -> console
  uint32_t count = tuh_cdc_read(idx, buf, bufsize);
  buf[count] = 0;
  uart_write_blocking(uart0, buf, count);
  #ifdef UsingDualUART
    uart_write_blocking(uart1, buf, count);
  #endif
}

// void core1_entry()
// {
//   while (1)
//   {
//     cdc_app_task();
//   }
// }

#ifdef UsingDualUART
  // uart1 RX interrupt handler
  void on_uart1_rx()
  {
    while (uart_is_readable(uart1))
    {
      uint8_t ch = uart_getc(uart1);
      if (tuh_cdc_mounted(firstLine))
      {
        tuh_cdc_write(firstLine, &ch, 1);
      }
    }
    if (tuh_cdc_mounted(firstLine))
    {
      tuh_cdc_write_flush(firstLine);
    }
  }
#endif
// uart0 RX interrupt handler
void on_uart0_rx()
{
  while (uart_is_readable(uart0))
  {
    uint8_t ch = uart_getc(uart0);
    if (tuh_cdc_mounted(firstLine))
    {
      tuh_cdc_write(firstLine, &ch, 1);
    }
  }
  if (tuh_cdc_mounted(firstLine))
  {
    tuh_cdc_write_flush(firstLine);
  }
}

/*------------- MAIN -------------*/
int main(void)
{
  board_init();

  stdio_init_all();
  // printf("TinyUSB Host CDC MSC HID Example\r\n");
  #ifdef USE_485
    const uint uart0_EN = UART0_EN_PIN;
    gpio_init(uart0_EN);
    gpio_set_dir(uart0_EN, GPIO_OUT);
    gpio_put(uart0_EN, 0);

    const uint uart1_EN = UART1_EN_PIN;
    gpio_init(uart1_EN);
    gpio_set_dir(uart1_EN, GPIO_OUT);
    gpio_put(uart1_EN, 0);
  #endif

  #ifdef UsingDualUART
    uart_init(uart1, BAUD_RATE);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(uart1, false, false);
    uart_set_format(uart1, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(uart1, false);
    irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(uart1, true, false);
  #endif
  uart_init(uart0, BAUD_RATE);

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);

  // Set UART flow control CTS/RTS, we don't want these, so turn them off
  uart_set_hw_flow(uart0, false, false);

  // Set our data format
  uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);

  // Turn off FIFO's - we want to do this character by character
  uart_set_fifo_enabled(uart0, false);

  // Set up a RX interrupt
  // We need to set up the handler first
  // Select correct interrupt for the UART we are using
  // And set up and enable the interrupt handlers
  irq_set_exclusive_handler(UART0_IRQ, on_uart0_rx);
  irq_set_enabled(UART0_IRQ, true);

  // Now enable the UART to send interrupts - RX only
  uart_set_irq_enables(uart0, true, false);

  // init host stack on configured roothub port
  tuh_init(BOARD_TUH_RHPORT);
  // multicore_launch_core1(core1_entry);

  while (1)
  {
    // tinyusb host task
    tuh_task();

    led_blinking_task();
    // cdc_app_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

void tuh_mount_cb(uint8_t dev_addr)
{
  // application set-up
  // printf("A device with address %d is mounted\r\n", dev_addr);
}

void tuh_umount_cb(uint8_t dev_addr)
{
  // application tear-down
  // printf("A device with address %d is unmounted \r\n", dev_addr);
}


//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  const uint32_t interval_ms = 1000;
  static uint32_t start_ms = 0;

  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
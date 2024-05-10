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

#include "tusb.h"
#include "get_serial.h"


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+


tusb_desc_device_t desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, //
    .bDeviceClass       = 0x00, // Each interface specifies its own
    .bDeviceSubClass    = 0x00, // Each interface specifies its own
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0x239A, // Pi
    .idProduct          = 0x80F4, // Picoprobe
    .bcdDevice          = 0x0100, // Version 01.00
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
  ITF_NUM_CDC_COM,
  ITF_NUM_CDC_DATA,
  ITF_NUM_PROBE,
  ITF_NUM_TOTAL
};

#if USB_HIGHSPEED
    #define USB_SPEED 0x00, 0x02  /*56,57  wMaxPacketSize 512*/
#else
    #define USB_SPEED 0x40, 0x00  /*56,57  wMaxPacketSize 64*/
#endif

#define usb_cdc_descriptor_template(interface, iInterface, cdc_control_in_endpoint, cdc_data_out_endpoint, cdc_data_in_endpoint) \
  /*CDC IAD Descriptor*/                                                                                                         \
  0x08,                      /* 0 bLength*/                                                                                      \
      0x0B,                  /* 1 bDescriptorType: IAD Descriptor*/                                                              \
      interface,             /* 2 bFirstInterface  [SET AT RUNTIME]*/                                                            \
      0x02,                  /* 3 bInterfaceCount: 2*/                                                                           \
      0x02,                  /* 4 bFunctionClass: COMM*/                                                                         \
      0x02,                  /* 5 bFunctionSubclass: ACM*/                                                                       \
      0x00,                  /* 6 bFunctionProtocol: NONE*/                                                                      \
      0x00, /* 7 iFunction*/ /*CDC Comm Interface Descriptor*/                                                                   \
      0x09,                  /* 8 bLength*/                                                                                      \
      0x04,                  /* 9 bDescriptorType (Interface)*/                                                                  \
      interface,             /*10 bInterfaceNumber  [SET AT RUNTIME]*/                                                           \
      0x00,                  /*11 bAlternateSetting*/                                                                            \
      0x01,                  /*12 bNumEndpoints 1*/                                                                              \
      0x02,                  /*13 bInterfaceClass: COMM*/                                                                        \
      0x02,                  /*14 bInterfaceSubClass: ACM*/                                                                      \
      0x00,                  /*15 bInterfaceProtocol: NONE*/                                                                     \
      iInterface,            /*16 iInterface (String Index)*/                                                                    \
                                                                                                                                 \
      /*CDC Header Descriptor*/                                                                                                  \
      0x05,       /*17 bLength*/                                                                                                 \
      0x24,       /*18 bDescriptorType: CLASS SPECIFIC INTERFACE*/                                                               \
      0x00,       /*19 bDescriptorSubtype: NONE*/                                                                                \
      0x10, 0x01, /*20,21 bcdCDC: 1.10*/                                                                                         \
                                                                                                                                 \
      /*CDC Call Management Descriptor*/                                                                                         \
      0x05,            /*22 bLength*/                                                                                            \
      0x24,            /*23 bDescriptorType: CLASS SPECIFIC INTERFACE*/                                                          \
      0x01,            /*24 bDescriptorSubtype: CALL MANAGEMENT*/                                                                \
      0x01,            /*25 bmCapabilities*/                                                                                     \
      (interface + 1), /*26 bDataInterface  [SET AT RUNTIME]*/                                                                   \
                                                                                                                                 \
      /*CDC Abstract Control Management Descriptor*/                                                                             \
      0x04, /*27 bLength*/                                                                                                       \
      0x24, /*28 bDescriptorType: CLASS SPECIFIC INTERFACE*/                                                                     \
      0x02, /*29 bDescriptorSubtype: ABSTRACT CONTROL MANAGEMENT*/                                                               \
      0x02, /*30 bmCapabilities*/                                                                                                \
                                                                                                                                 \
      /*CDC Union Descriptor*/                                                                                                   \
      0x05,            /*31 bLength*/                                                                                            \
      0x24,            /*32 bDescriptorType: CLASS SPECIFIC INTERFACE*/                                                          \
      0x06,            /*33 bDescriptorSubtype: CDC*/                                                                            \
      interface,       /*34 bMasterInterface  [SET AT RUNTIME]*/                                                                 \
      (interface + 1), /*35 bSlaveInterface_list (1 item)*/                                                                      \
                                                                                                                                 \
      /*CDC Control IN Endpoint Descriptor*/                                                                                     \
      0x07,                    /*36 bLength*/                                                                                    \
      0x05,                    /*37 bDescriptorType (Endpoint)*/                                                                 \
      cdc_control_in_endpoint, /*38 bEndpointAddress (IN/D2H) [SET AT RUNTIME: 0x80 | number]*/                                  \
      0x03,                    /*39 bmAttributes (Interrupt)*/                                                                   \
      0x40, 0x00,              /*40, 41 wMaxPacketSize 64*/                                                                      \
      0x10,                    /*42 bInterval 16 (unit depends on device speed)*/                                                \
                                                                                                                                 \
      /*CDC Data Interface*/                                                                                                     \
      0x09,             /*43 bLength*/                                                                                           \
      0x04,             /*44 bDescriptorType (Interface)*/                                                                       \
      (interface + 1),  /*45 bInterfaceNumber  [SET AT RUNTIME]*/                                                                \
      0x00,             /*46 bAlternateSetting*/                                                                                 \
      0x02,             /*47 bNumEndpoints 2*/                                                                                   \
      0x0A,             /*48 bInterfaceClass: DATA*/                                                                             \
      0x00,             /*49 bInterfaceSubClass: NONE*/                                                                          \
      0x00,             /*50 bInterfaceProtocol*/                                                                                \
      (iInterface + 1), /*51 iInterface (String Index)*/                                                                         \
                                                                                                                                 \
      /*CDC Data OUT Endpoint Descriptor*/                                                                                       \
      0x07,                    /*52 bLength*/                                                                                    \
      0x05,                    /*53 bDescriptorType (Endpoint)*/                                                                 \
      (cdc_data_out_endpoint), /*54 bEndpointAddress (OUT/H2D) [SET AT RUNTIME]*/                                                \
      0x02,                    /*55 bmAttributes (Bulk)*/                                                                        \
      USB_SPEED,               /*56,57  wMaxPacketSize 64 or 512*/                                                               \
      0x00,                    /*58 bInterval 0 (unit depends on device speed)*/                                                 \
                                                                                                                                 \
      /*CDC Data IN Endpoint Descriptor*/                                                                                        \
      0x07,                 /*59 bLength*/                                                                                       \
      0x05,                 /*60 bDescriptorType (Endpoint)*/                                                                    \
      cdc_data_in_endpoint, /*61 bEndpointAddress (IN/D2H) [SET AT RUNTIME: 0x80 | number]*/                                     \
      0x02,                 /*62 bmAttributes (Bulk)*/                                                                           \
      USB_SPEED,            /*63,64  wMaxPacketSize 64 or 512*/                                                                  \
      0x00                  /*65 bInterval 0 (unit depends on device speed)*/

#define interface 0
#define iInterface 4

#define configuration_descriptor_template(totalLength, interfacesNum)                               \
  0x09,                                              /*0 bLength*/                                  \
      0x02,                                          /*1 bDescriptorType (Configuration)*/          \
      totalLength & 0xFF, (totalLength >> 8) & 0xFF, /*2,3 wTotalLength  [SET AT RUNTIME: lo, hi]*/ \
      interfacesNum,                                 /*4 bNumInterfaces  [SET AT RUNTIME]*/         \
      0x01,                                          /*5 bConfigurationValue*/                      \
      0x00,                                          /*6 iConfiguration (String Index)*/            \
      0x80 | TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,     /*7 bmAttributes*/                             \
      0x32                                           /*8 bMaxPower 100mA*/



uint8_t const desc_configuration[] =
    {
        configuration_descriptor_template(0x8D, 4),
        usb_cdc_descriptor_template(interface, iInterface, 0x80 | (interface + 1), (interface + 2), 0x80 | (interface + 2)),
        usb_cdc_descriptor_template(interface + 2, iInterface + 2, 0x80 | (interface + 3), (interface + 4), 0x80 | (interface + 4))};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
  (void)index; // for multiple configurations
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const* string_desc_arr [] =
{
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "Raspberry Pi", // 1: Manufacturer
  "Picoprobe",    // 2: Product
  usb_serial,     // 3: Serial, uses flash unique ID
};

typedef union {
    const char *char_str;
    const uint16_t *descriptor;
} interface_string_t;


#define MAX_INTERFACE_STRINGS 16
interface_string_t collected_interface_strings[MAX_INTERFACE_STRINGS];
uint16_t lang_descriptor[] = {0x0304, 0x0409};
// Raspberry Pi
uint16_t descArr1[] = {0x031A, 0x52, 0X61, 0X73, 0X70, 0X62, 0X65, 0X72, 0X72, 0x79, 0X20, 0X50, 0X69};
// Pico
uint16_t descArr2[] = {0x030A, 0x50, 0X69, 0X63, 0X6F};
// E6611C08CB58B524
uint16_t descArr3[] = {0x0322, 0x45, 0X36, 0X36, 0X31, 0X31, 0X43, 0X30, 0X38, 0x43, 0X42, 0X35, 0X38, 0X42, 0X35, 0X32, 0X34};
// CircuitPython CDC control
uint16_t descArr4[] = {0x0334, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20, 0x43, 0x44, 0x43, 0x20, 0x63, 0x6F, 0x6E, 0x74, 0x72, 0x6F, 0x6C};
// CircuitPython CDC data
uint16_t descArr5[] = {0x032E, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20, 0x43, 0x44, 0x43, 0x20, 0x64, 0x61, 0x74, 0x61};
// CircuitPython CDC2 control
uint16_t descArr6[] = {0x0336, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20, 0x43, 0x44, 0x43, 0x32, 0x20, 0x63, 0x6F, 0x6E, 0x74, 0x72, 0x6F, 0x6C};
// CircuitPython CDC2 data
uint16_t descArr7[] = {0x0330, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20, 0x43, 0x44, 0x43, 0x32, 0x20, 0x64, 0x61, 0x74, 0x61};
//
uint16_t descArr8[] = {0x0324, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20};
uint16_t descArr9[] = {0x26, 0x03, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20};
uint16_t descArr10[] = {0x28, 0x03, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20};
uint16_t descArr11[] = {0x40, 0x03, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20};
uint16_t descArr12[] = {0x40, 0x03, 0x43, 0x69, 0x72, 0x63, 0x75, 0x69, 0x74, 0x50, 0x79, 0x74, 0x68, 0x6F, 0x6E, 0x20};
uint16_t descArr13[] = {0x1F, 0x04, 0x20, 0xEB, 0x35, 0x31, 0x4D, 0x75, 0x01, 0x03, 0x7A, 0xC4, 0x1D, 0x23};
uint16_t descArr14[] = {0x1F, 0x04, 0x20, 0xEB, 0x35, 0x31, 0x4D, 0x75, 0x01, 0x03, 0x7A, 0xC4, 0x1D, 0x23};
uint16_t descArr15[] = {0x1F, 0x04, 0x20, 0xEB, 0x35, 0x31, 0x4D, 0x75, 0x01, 0x03, 0x7A, 0xC4, 0x1D, 0x23};

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  // (void) langid;
  switch (index)
  {
    case 0:
      return lang_descriptor;
    case 1:
      return descArr1;
    case 2:
      return descArr2;
    case 3:
      return descArr3;
    case 4:
      return descArr4;
    case 5:
      return descArr5;
    case 6:
      return descArr6;
    case 7:
      return descArr7;
    // case 8:
    //   return descArr8;
    // case 9:
    //   return descArr9;
    // case 10:
    //   return descArr10;
    // case 11:
    //   return descArr11;
    // case 12:
    //   return descArr12;
    // case 13:
    //   return descArr13;
    // case 14:
    //   return descArr14;
    // case 15:
    //   return descArr15;
    default:
      return NULL;
  }

}

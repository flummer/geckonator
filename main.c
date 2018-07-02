/*
 * This file is part of geckonator.
 * Copyright 2017 Emil Renner Berthing <esmil@esmil.dk>
 *
 * geckonator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * geckonator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with geckonator. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "geckonator/clock.h"
#include "geckonator/gpio.h"
#include "geckonator/leuart0.h"
#include "geckonator/usb.h"

//#define LEUART_DEBUG
#define ACM_DEBUG
#ifdef NDEBUG
#undef LEUART_DEBUG
#undef ACM_DEBUG
#define debug(...)
#else
#define debug(...) printf(__VA_ARGS__)
#endif

#define LED_RED   GPIO_PA8
#define LED_GREEN GPIO_PA9
#define LED_BLUE  GPIO_PA10

#define USB_WORD(x) ((x) & 0xFF),((x) >> 8)
#define USB_TRIPLE(x) ((x) & 0xFF),(((x) >> 8) & 0xFF),((x) >> 16)
#define USB_QUAD(x) ((x) & 0xFF),(((x) >> 8) & 0xFF),(((x) >> 16) & 0xFF),((x) >> 24)

#define USB_FIFO_RXSIZE  256
#define USB_FIFO_TX0SIZE 128
#define USB_FIFO_TX1SIZE 128
#define USB_FIFO_TX2SIZE 64
#define USB_FIFO_TX3SIZE 0

#define DFU_INTERFACE 0

#define CDC_INTERFACE 1
#define CDC_PACKETSIZE 64
#define CDC_ENDPOINT 2

#define ACM_INTERFACE 2
#define ACM_ENDPOINT 1
#define ACM_PACKETSIZE 64

struct usb_packet_setup {
	union {
		struct {
			uint8_t bmRequestType;
			uint8_t bRequest;
		};
		uint16_t request;
	};
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};

struct usb_descriptor_device {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
};

struct usb_descriptor_configuration {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
	uint8_t rest[];
};

struct usb_descriptor_string {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wCodepoint[];
};

static const __align(4) struct usb_descriptor_device usb_descriptor_device = {
	.bLength            = 18,
	.bDescriptorType    = 0x01, /* Device */
	.bcdUSB             = 0x0200,
	.bDeviceClass       = 0x00, /* 0x00 = per interface */
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,
	.bMaxPacketSize0    = 64,
	.idVendor           = 0x0483,
	.idProduct          = 0x5740,
	.bcdDevice          = 0x0200,
	.iManufacturer      = 1,
	.iProduct           = 2,
	.iSerialNumber      = 3,
	.bNumConfigurations = 1,
};

static const __align(4) struct usb_descriptor_configuration usb_descriptor_configuration1 = {
	.bLength              = 9,
	.bDescriptorType      = 0x02, /* Configuration */
#ifdef ACM_DEBUG
	.wTotalLength         = 9 + 18 + 58,
	.bNumInterfaces       = 1 + 2,
#else
	.wTotalLength         = 9 + 18,
	.bNumInterfaces       = 1,
#endif
	.bConfigurationValue  = 1,
	.iConfiguration       = 0,
	.bmAttributes         = 0x80,
	.bMaxPower            = 250,
	.rest = {
	/* Interface */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x04, /* Interface */
	/* .bInterfaceNumber       */ DFU_INTERFACE,
	/* .bAlternateSetting      */ 0,
	/* .bNumEndpoints          */ 0,
	/* .bInterfaceClass        */ 0xfe, /* 0xfe = Application Specific */
	/* .bInterfaceSubClass     */ 0x01, /* 0x01 = Device Firmware Upgrade */
	/* .bInterfaceProtocol     */ 0x01, /* 0x01 = Runtime Protocol */
	/* .iInterface             */ 4,
	/* DFU Interface */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x21, /* DFU Interface */
	/* .bmAttributes           */ 0x08, /* will detach usb on DFU_DETACH */
	/* .wDetachTimeOut         */ USB_WORD(50), /* 50ms */
	/* .wTransferSize          */ USB_WORD(FLASH_PAGE_SIZE),
	/* .bcdDFUVersion          */ USB_WORD(0x0101), /* DFU v1.1 */
#ifdef ACM_DEBUG
	/* Interface */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x04, /* Interface */
	/* .bInterfaceNumber       */ CDC_INTERFACE,
	/* .bAlternateSetting      */ 0,
	/* .bNumEndpoints          */ 1,
	/* .bInterfaceClass        */ 0x02, /* 0x02 = CDC */
	/* .bInterfaceSubClass     */ 0x02, /* 0x02 = ACM */
	/* .bInterfaceProtocol     */ 0x01, /* 0x00 = no protocol required, 0x01 = AT commands V.250 etc */
	/* .iInterface             */ 0,
	/* CDC Header */
	/* .bLength                */ 5,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x00, /* Header */
	/* .bcdCDC                 */ USB_WORD(0x0120),
	/* CDC Call Management */
	/* .bLength                */ 5,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x01, /* Call Management */
	/* .bmCapabilities         */ 0x00, /* Does not handle call management */
	/* .bDataInterface         */ ACM_INTERFACE,
	/* CDC ACM */
	/* .bLength                */ 4,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x02, /* ACM */
	/* .bmCapabilities         */ 0x00, /* 0x00 = supports nothing */
	/* CDC Union */
	/* .bLength                */ 5,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x06, /* Union */
	/* .bControlInterface      */ CDC_INTERFACE,
	/* .bSubordinateInterface0 */ ACM_INTERFACE,
	/* Endpoint */
	/* .bLength                */ 7,
	/* .bDescriptorType        */ 0x05, /* Endpoint */
	/* .bEndpointAddress       */ 0x80 | CDC_ENDPOINT, /* in */
	/* .bmAttributes           */ 0x03, /* interrupt */
	/* .wMaxPacketSize         */ USB_WORD(CDC_PACKETSIZE),
	/* .bInterval              */ 255,  /* poll every 255ms */
	/* Interface */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x04, /* Interface */
	/* .bInterfaceNumber       */ ACM_INTERFACE,
	/* .bAlternateSetting      */ 0,
	/* .bNumEndpoints          */ 2,
	/* .bInterfaceClass        */ 0x0A, /* 0x0A = CDC Data */
	/* .bInterfaceSubClass     */ 0x00,
	/* .bInterfaceProtocol     */ 0x00,
	/* .iInterface             */ 0,
	/* Endpoint */
	/* .bLength                */ 7,
	/* .bDescriptorType        */ 0x05, /* Endpoint */
	/* .bEndpointAddress       */ 0x80 | ACM_ENDPOINT, /* in */
	/* .bmAttributes           */ 0x02, /* bulk */
	/* .wMaxPacketSize         */ USB_WORD(ACM_PACKETSIZE),
	/* .bInterval              */ 0,    /* unused */
	/* Endpoint */
	/* .bLength                */ 7,
	/* .bDescriptorType        */ 0x05, /* Endpoint */
	/* .bEndpointAddress       */ ACM_ENDPOINT, /* out */
	/* .bmAttributes           */ 0x02, /* bulk */
	/* .wMaxPacketSize         */ USB_WORD(ACM_PACKETSIZE),
	/* .bInterval              */ 0,    /* unused */
#endif
	}
};

static const struct usb_descriptor_configuration *const usb_descriptor_configuration[] = {
	&usb_descriptor_configuration1,
};

static const __align(4) struct usb_descriptor_string usb_descriptor_string0 = {
	.bLength         = 4,
	.bDescriptorType = 0x03, /* String */
	.wCodepoint = {
		0x0409, /* English (US) */
	},
};

static const __align(4) struct usb_descriptor_string usb_descriptor_vendor = {
	.bLength         = 16,
	.bDescriptorType = 0x03, /* String */
	.wCodepoint = {
		'L','a','b','i','t','a','t',
	},
};

static const __align(4) struct usb_descriptor_string usb_descriptor_product = {
	.bLength         = 22,
	.bDescriptorType = 0x03, /* String */
	.wCodepoint = {
		'G','e','c','k','o','n','a','t','o','r',
	},
};

/* must be at least 12 characters long and consist of only '0'-'9','A'-'B'
 * at least according to the mass-storage bulk-only document */
static const __align(4) struct usb_descriptor_string usb_descriptor_serial = {
	.bLength         = 26,
	.bDescriptorType = 0x03, /* String */
	.wCodepoint = {
		'0','0','0','0','0','0','0','0','0','0','0','1',
	},
};

static const __align(4) struct usb_descriptor_string usb_descriptor_dfu = {
	.bLength         = 20,
	.bDescriptorType = 0x03, /* String */
	.wCodepoint = {
		'G','e','c','k','o','B','o','o','t',
	},
};

static const struct usb_descriptor_string *const usb_descriptor_string[] = {
	&usb_descriptor_string0,
	&usb_descriptor_vendor,
	&usb_descriptor_product,
	&usb_descriptor_serial,
	&usb_descriptor_dfu,
};

static struct {
	uint32_t bytes;
	uint32_t packetsize;
} usb_state;

static __uninitialized uint32_t usb_outbuf[
	(4*sizeof(struct usb_packet_setup) + 64) / sizeof(uint32_t)
];
static __uninitialized union {
	int8_t    i8[4];
	uint8_t   u8[4];
	int16_t  i16[2];
	uint16_t u16[2];
	int32_t  i32[1];
	uint32_t u32[1];
} usb_inbuf;

#ifdef ACM_DEBUG
static __uninitialized union {
	uint32_t word[ACM_PACKETSIZE/sizeof(uint32_t)];
	uint8_t byte[ACM_PACKETSIZE];
} acm_outbuf;

static struct {
	volatile uint32_t first;
	volatile uint32_t last;
	uint8_t buf[2048];
} acm_inbuf;

static volatile bool acm_idle;

static void
acm_shipit(void)
{
	uint32_t first = acm_inbuf.first;
	uint32_t last = acm_inbuf.last;
	uint32_t end;

	if (first == last) {
		acm_inbuf.first = acm_inbuf.last = 0;
		acm_idle = true;
		return;
	}

	end = first + ACM_PACKETSIZE;
	if (last > first && end > last) {
		end = last;
		if (last & 3) {
			last = (last & ~3) + 4;
			last %= ARRAY_SIZE(acm_inbuf.buf);
		}
		acm_inbuf.first = acm_inbuf.last = last;
	} else if (end >= ARRAY_SIZE(acm_inbuf.buf)) {
		end = ARRAY_SIZE(acm_inbuf.buf);
		acm_inbuf.first = 0;
	} else
		acm_inbuf.first = end;

	usb_ep_in_dma_address_set(ACM_ENDPOINT, &acm_inbuf.buf[first]);
	usb_ep_in_transfer_size(ACM_ENDPOINT, 1, end - first);
	usb_ep_in_enable(ACM_ENDPOINT);

	acm_idle = false;
}

static void
acm_write(const uint8_t *ptr, size_t len)
{
	uint32_t last = acm_inbuf.last;

	for (; len > 0; len--) {
		acm_inbuf.buf[last++] = *ptr++;
		last %= ARRAY_SIZE(acm_inbuf.buf);
	}
	acm_inbuf.last = last;

	if (acm_idle)
		acm_shipit();
}

static void
acm_writeln(const uint8_t *ptr, size_t len)
{
	static const uint8_t hex[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	uint32_t last = acm_inbuf.last;

	for (; len > 0; len--) {
		uint8_t c = *ptr++;

		if (c > 32 && c < 127) {
			acm_inbuf.buf[last++] = c;
			last %= ARRAY_SIZE(acm_inbuf.buf);
		} else if (c == '\r') {
			acm_inbuf.buf[last++] = '\r';
			last %= ARRAY_SIZE(acm_inbuf.buf);
			acm_inbuf.buf[last++] = '\n';
			last %= ARRAY_SIZE(acm_inbuf.buf);
		} else if (c == 127) {
			acm_inbuf.buf[last++] = '\b';
			last %= ARRAY_SIZE(acm_inbuf.buf);
		} else {
			acm_inbuf.buf[last++] = '\\';
			last %= ARRAY_SIZE(acm_inbuf.buf);
			acm_inbuf.buf[last++] = 'x';
			last %= ARRAY_SIZE(acm_inbuf.buf);
			acm_inbuf.buf[last++] = hex[c >> 4];
			last %= ARRAY_SIZE(acm_inbuf.buf);
			acm_inbuf.buf[last++] = hex[c & 0xF];
			last %= ARRAY_SIZE(acm_inbuf.buf);
		}
	}
	acm_inbuf.last = last;

	if (acm_idle)
		acm_shipit();
}
#endif

#ifdef LEUART_DEBUG
static struct {
	volatile uint32_t first;
	volatile uint32_t last;
	uint8_t buf[4096];
} leuart0_output;

static void
leuart0_write(const uint8_t *ptr, size_t len)
{
	uint32_t last = leuart0_output.last;

	for (; len > 0; len--) {
		leuart0_output.buf[last++] = *ptr++;
		last %= ARRAY_SIZE(leuart0_output.buf);
	}
	leuart0_output.last = last;
	leuart0_flag_tx_buffer_level_enable();
}

void
LEUART0_IRQHandler(void)
{
	uint32_t first = leuart0_output.first;
	uint32_t last = leuart0_output.last;

	if (first == last) {
		leuart0_flag_tx_buffer_level_disable();
		return;
	}

	leuart0_txdata(leuart0_output.buf[first++]);
	leuart0_output.first = first % ARRAY_SIZE(leuart0_output.buf);
}
#endif

#ifndef NDEBUG
ssize_t __used
_write(int fd, const uint8_t *ptr, size_t len)
{
#ifdef LEUART_DEBUG
	leuart0_write(ptr, len);
#endif
#ifdef ACM_DEBUG
	acm_write(ptr, len);
#endif
	return len;
}
#endif

static void
dumpsetup(const struct usb_packet_setup *p)
{
	debug("{\r\n"
	      "  bmRequestType 0x%02x\r\n"
	      "  bRequest      0x%02x\r\n"
	      "  wValue        0x%04x\r\n"
	      "  wIndex        0x%04x\r\n"
	      "  wLength       0x%04x\r\n"
	      "}\r\n",
		p->bmRequestType,
		p->bRequest,
		p->wValue,
		p->wIndex,
		p->wLength);
}

static void
usb_ep0out_prepare_setup(void *buf)
{
	usb_ep0out_dma_address_set(buf);
	usb_ep0out_transfer_size(0, 0);
	usb_ep0out_enable_setup();
}

static void
usb_suspend(void)
{
	usb_phy_stop();
	usb_flags_enable(USB_GINTMSK_WKUPINTMSK
			| USB_GINTMSK_RESETDETMSK);
}

static void
usb_unsuspend(void)
{
	usb_phy_start();
	usb_flags_enable(USB_GINTMSK_ENUMDONEMSK
			| USB_GINTMSK_USBRSTMSK
			| USB_GINTMSK_USBSUSPMSK
			| USB_GINTMSK_OEPINTMSK
			| USB_GINTMSK_IEPINTMSK);
}

static void
usb_ep_reset(void)
{
	unsigned int i;

	usb_ep0out_config_64byte_stall();
	for (i = 1; i < 4; i++)
		usb_ep_out_config_disabled_nak(i);

	usb_ep0in_config_64byte_stall();
	for (i = 1; i < 4; i++)
		usb_ep_in_config_disabled_nak(i);
}

static void
usb_reset(void)
{
	usb_ep_reset();

	usb_state.bytes = 0;

	/* flush fifos */
	usb_fifo_flush();
	while (usb_fifo_flushing())
		/* wait */;
}

static void
usb_enumdone(void)
{
	switch (usb_enumerated_speed()) {
	case 0:
	case 1:
		debug("\r\nERROR!\r\n");
		return;
	case 2:
		debug("low speed.. ");
		/* use 8 byte packages */
		usb_ep0out_config_8byte();
		usb_ep0in_config_8byte();
		usb_state.packetsize = 8;
		break;
	case 3:
		debug("full speed.. ");
		/* we already set 64 byte packages at reset */
		usb_state.packetsize = 64;
		break;
	}

	/* prepare to receive setup package */
	usb_ep0out_prepare_setup(&usb_outbuf);

	/* enable interrupts for endpoint 0 only */
	USB->DAINTMSK = USB_DAINTMSK_INEPMSK0 | USB_DAINTMSK_OUTEPMSK0;
	USB->DOEPMSK = USB_DOEPMSK_SETUPMSK
	             | USB_DOEPMSK_EPDISBLDMSK
	             | USB_DOEPMSK_XFERCOMPLMSK;
	USB->DIEPMSK = USB_DIEPMSK_TIMEOUTMSK
	             | USB_DIEPMSK_EPDISBLDMSK
	             | USB_DIEPMSK_XFERCOMPLMSK;
}

static int
usb_handle_get_status_device(const struct usb_packet_setup *p, const void **data)
{
	debug("GET_STATUS device\r\n");
	usb_inbuf.u16[0] = 0;
	*data = &usb_inbuf;
	return 2;
}

static int
usb_handle_set_address(const struct usb_packet_setup *p, const void **data)
{
	debug("SET_ADDRESS: wValue = %hu\r\n", p->wValue);
	usb_set_address(p->wValue);
	return 0;
}

static int
usb_handle_get_descriptor_device(const void **data, uint8_t index)
{
	debug("GET_DESCRIPTOR: device\r\n");
	if (index != 0) {
		debug("GET_DESCRIPTOR: type = 0x01, but index = 0x%02x\r\n", index);
		return -1;
	}
	*data = &usb_descriptor_device;
	return sizeof(usb_descriptor_device);
}

static int
usb_handle_get_descriptor_configuration(const void **data, uint8_t index)
{
	const struct usb_descriptor_configuration *desc;

	debug("GET_DESCRIPTOR: configuration %hu\r\n", index);
	if (index >= ARRAY_SIZE(usb_descriptor_configuration)) {
		debug("GET_DESCRIPTOR: unknown configuration %hu\r\n", index);
		return -1;
	}
	desc = usb_descriptor_configuration[index];
	*data = desc;
	return desc->wTotalLength;
}

static int
usb_handle_get_descriptor_string(const void **data, uint8_t index)
{
	const struct usb_descriptor_string *desc;

	debug("GET_DESCRIPTOR: string %hu\r\n", index);
	if (index >= ARRAY_SIZE(usb_descriptor_string)) {
		debug("GET_DESCRIPTOR: unknown string %hu\r\n", index);
		return -1;
	}
	desc = usb_descriptor_string[index];
	*data = desc;
	return desc->bLength;
}

static int
usb_handle_get_descriptor(const struct usb_packet_setup *p, const void **data)
{
	uint8_t type = p->wValue >> 8;
	uint8_t index = p->wValue & 0xFFU;

	switch (type) {
	case 0x01:
		return usb_handle_get_descriptor_device(data, index);
	case 0x02:
		return usb_handle_get_descriptor_configuration(data, index);
	case 0x03:
		return usb_handle_get_descriptor_string(data, index);
	case 0x06: /* DEVICE QUALIFIER (for high-speed) */
		debug("DEVICE_QUALIFIER\r\n");
		break;
	default:
		debug("GET_DESCRIPTOR: unknown type 0x%02x\r\n", type);
		dumpsetup(p);
		break;
	}
	return -1;
}

static int
usb_handle_get_configuration(const struct usb_packet_setup *p, const void **data)
{
	debug("GET_CONFIGURATION\r\n");
	usb_inbuf.u8[0] = usb_descriptor_configuration[0]->bConfigurationValue;
	*data = &usb_inbuf;
	return 1;
}

static int
usb_handle_set_configuration(const struct usb_packet_setup *p, const void **data)
{
	debug("SET_CONFIGURATION: wIndex = %hu, wValue = %hu\r\n",
			p->wIndex, p->wValue);

	if (p->wIndex == 0 && p->wValue == usb_descriptor_configuration[0]->bConfigurationValue) {
#ifdef ACM_DEBUG
		/* configure CDC endpoint */
		usb_ep_in_config_interrupt(CDC_ENDPOINT, CDC_PACKETSIZE);
		usb_ep_flag_in_enable(CDC_ENDPOINT);

		/* configure ACM endpoints */
		usb_ep_in_config_bulk(ACM_ENDPOINT, ACM_PACKETSIZE);

		usb_ep_out_dma_address_set(ACM_ENDPOINT, &acm_outbuf);
		usb_ep_out_transfer_size(ACM_ENDPOINT, 1, ACM_PACKETSIZE);
		usb_ep_out_config_bulk_enabled(ACM_ENDPOINT, ACM_PACKETSIZE);
		usb_ep_flag_inout_enable(ACM_ENDPOINT);
#endif
		return 0;
	}

	return -1;
}

static int
usb_handle_set_interface(const struct usb_packet_setup *p, const void **data)
{
	debug("SET_INTERFACE: wIndex = %hu, wValue = %hu\r\n", p->wIndex, p->wValue);

	if (p->wIndex == DFU_INTERFACE && p->wValue == 0)
		return 0;
#ifdef ACM_DEBUG
	if (p->wIndex == CDC_INTERFACE && p->wValue == 0)
		return 0;
	if (p->wIndex == ACM_INTERFACE && p->wValue == 0)
		return 0;
#endif

	return -1;
}

static int
usb_handle_clear_feature_endpoint(const struct usb_packet_setup *p, const void **data)
{
	debug("CLEAR_FEATURE endpoint %hu\r\n", p->wIndex);

#ifdef ACM_DEBUG
	if (p->wIndex == CDC_ENDPOINT)
		return 0;
	if (p->wIndex == ACM_ENDPOINT)
		return 0;
#endif

	return -1;
}

static bool dfu_reset;

static int
usb_handle_dfu_detach(const struct usb_packet_setup *p, const void **data)
{
	debug("DFU_DETACH: wValue = %hu, wIndex = %hu\r\n",
			p->wValue, p->wIndex);

	if (p->wIndex != DFU_INTERFACE)
		return -1;

	dfu_reset = true;
	return 0;
}

#ifdef ACM_DEBUG
static int
usb_handle_set_control_line_state(const struct usb_packet_setup *p, const void **data)
{
	debug("SET_CONTROL_LINE_STATE: wIndex = %hu wValue = 0x%04hx\r\n",
			p->wIndex, p->wValue);

	if (p->wIndex == CDC_INTERFACE) {
		debug("  RTS %u DTR %u\r\n", !!(p->wValue & 0x2), !!(p->wValue & 0x1));
		acm_shipit();
		return 0;
	}

	return -1;
}

static int
usb_handle_set_line_coding(const struct usb_packet_setup *p, const void **data)
{
	if (p->wIndex == CDC_INTERFACE) {
		const struct {
			uint32_t dwDTERate;
			uint8_t bCharFormat; /* 0 = 1 stop bit, 1 = 1.5, 2 = 2 */
			uint8_t bParityType; /* 0=none, 1=odd, 2=even, 3=mark, 4=space */
			uint8_t bDataBits;
		} *lc = *data;

		debug("SET_LINE_CODING: {\r\n"
		      "  dwDTERate   = %lu\r\n"
		      "  bCharFormat = 0x%02x\r\n"
		      "  bParityType = 0x%02x\r\n"
		      "  bDataBits   = 0x%02x\r\n"
		      "}\r\n",
				lc->dwDTERate,
				lc->bCharFormat,
				lc->bParityType,
				lc->bDataBits);
		return 0;
	}

	return -1;
}
#endif

struct usb_setup_handler {
	uint16_t req;
	int16_t len;
	int (*fn)(const struct usb_packet_setup *p, const void **data);
};

static const struct usb_setup_handler usb_setup_handlers[] = {
	{ .req = 0x0080, .len = -1, .fn = usb_handle_get_status_device },
	{ .req = 0x0500, .len =  0, .fn = usb_handle_set_address },
	{ .req = 0x0680, .len = -1, .fn = usb_handle_get_descriptor },
	{ .req = 0x0880, .len = -1, .fn = usb_handle_get_configuration },
	{ .req = 0x0900, .len =  0, .fn = usb_handle_set_configuration },
	{ .req = 0x0b01, .len =  0, .fn = usb_handle_set_interface },
	{ .req = 0x0102, .len =  0, .fn = usb_handle_clear_feature_endpoint },
	{ .req = 0x0021, .len =  0, .fn = usb_handle_dfu_detach },
#ifdef ACM_DEBUG
	{ .req = 0x2221, .len =  0, .fn = usb_handle_set_control_line_state },
	{ .req = 0x2021, .len =  7, .fn = usb_handle_set_line_coding },
#endif
};

static int
usb_setup_handler_run(const struct usb_packet_setup *p, const void **data)
{
	const struct usb_setup_handler *h = usb_setup_handlers;
	const struct usb_setup_handler *end = h + ARRAY_SIZE(usb_setup_handlers);

	for (; h < end; h++) {
		if (h->req == p->request) {
			if (h->len >= 0 && (uint16_t)h->len != p->wLength)
				break;
			return h->fn(p, data);
		}
	}

	debug("unknown request 0x%04x\r\n", p->request);
	dumpsetup(p);
	return -1;
}

static void
usb_handle_setup(void)
{
	const struct usb_packet_setup *p =
		usb_ep0out_dma_address() - sizeof(struct usb_packet_setup);

	usb_state.bytes = 0;

	if (p->bmRequestType & 0x80U) {
		const void *data;
		int ret = usb_setup_handler_run(p, &data);

		if (ret >= 0) {
			/* send IN data */
			if (ret > p->wLength)
				ret = p->wLength;
			usb_state.bytes = ret;
			if ((uint32_t)ret > usb_state.packetsize)
				ret = usb_state.packetsize;
			usb_ep0in_dma_address_set(data);
			usb_ep0in_transfer_size(1, ret);
			usb_ep0in_enable();
			/* prepare for IN ack */
			usb_ep0out_dma_address_set(&usb_outbuf);
			usb_ep0out_transfer_size(1, usb_state.packetsize);
			usb_ep0out_enable();
			return;
		}
	} else if (p->wLength == 0) {
		if (!usb_setup_handler_run(p, NULL)) {
			/* send empty ack package */
			usb_ep0in_transfer_size(1, 0);
			usb_ep0in_enable();
			/* prepare for next SETUP package */
			usb_ep0out_prepare_setup(&usb_outbuf);
			return;
		}
	} else if (p->wLength <= sizeof(usb_outbuf) - 4*sizeof(struct usb_packet_setup)) {
		uint32_t *rp = (uint32_t *)p;

		if (rp != &usb_outbuf[0]) {
			usb_outbuf[0] = rp[0];
			usb_outbuf[1] = rp[1];
		}

		/* receive OUT data */
		usb_ep0out_dma_address_set(&usb_outbuf[2]);
		usb_ep0out_transfer_size(1, usb_state.packetsize);
		usb_ep0out_enable();
		usb_state.bytes = p->wLength;
		return;
	}

	/* stall IN endpoint */
	usb_ep0in_stall();
	/* prepare for next SETUP package */
	usb_ep0out_prepare_setup(&usb_outbuf);
}

static void
usb_handle_ep0(void)
{
	uint32_t oflags = usb_ep0out_flags();
	uint32_t iflags = usb_ep0in_flags();
	uint32_t bytes;

	usb_ep0out_flags_clear(oflags);
	usb_ep0in_flags_clear(iflags);

	debug("EP0 %04lx %04lx %lu\r\n", oflags, iflags, usb_state.bytes);

	if (usb_ep0out_flag_setup(oflags)) {
		usb_handle_setup();
		return;
	}

	bytes = usb_state.bytes;
	if (bytes == 0) {
		if (dfu_reset) {
			if (usb_ep0in_flag_complete(iflags)) {
				NVIC_SystemReset();
				__builtin_unreachable();
			} else
				dfu_reset = false;
		}
		return;
	}

	if (usb_ep0in_flag_complete(iflags)) {
		/* data IN */
		if (bytes > usb_state.packetsize) {
			/* send next package */
			bytes -= usb_state.packetsize;
			usb_state.bytes = bytes;
			if (bytes > usb_state.packetsize)
				bytes = usb_state.packetsize;
			usb_ep0in_transfer_size(1, bytes);
			usb_ep0in_enable();
		} else
			usb_state.bytes = 0;
	} else if (usb_ep0out_flag_complete(oflags)) {
		/* data OUT */
		bytes = usb_state.packetsize - usb_ep0out_bytes_left();
		usb_state.bytes -= bytes;
		if (usb_state.bytes > 0) {
			/* prepare for more OUT data */
			usb_ep0out_transfer_size(1, usb_state.packetsize);
			usb_ep0out_enable();
		} else {
			const void *data = &usb_outbuf[2];

			if (!usb_setup_handler_run((const void *)&usb_outbuf, &data)) {
				/* send empty ack package */
				usb_ep0in_transfer_size(1, 0);
				usb_ep0in_enable();
			} else
				usb_ep0in_stall();
			usb_ep0out_prepare_setup(&usb_outbuf);
		}
	}
}

#ifdef ACM_DEBUG
static void
usb_handle_acm_in(void)
{
	uint32_t flags = usb_ep_in_flags(ACM_ENDPOINT);

	usb_ep_in_flags_clear(ACM_ENDPOINT, flags);

	if (usb_ep_in_flag_timeout(flags)) {
		debug("ACM: timeout!!\r\n");
		usb_ep_in_enable(ACM_ENDPOINT);
	}
	if (usb_ep_in_flag_complete(flags))
		acm_shipit();
}

static void
usb_handle_acm_out(void)
{
	uint32_t flags = usb_ep_out_flags(ACM_ENDPOINT);
	uint32_t bytes;

	usb_ep_out_flags_clear(ACM_ENDPOINT, flags);

	if (!usb_ep_out_flag_complete(flags))
		return;

	bytes = ACM_PACKETSIZE - usb_ep_out_bytes_left(ACM_ENDPOINT);

	acm_writeln(acm_outbuf.byte, bytes);

	usb_ep_out_dma_address_set(ACM_ENDPOINT, &acm_outbuf);
	usb_ep_out_transfer_size(ACM_ENDPOINT, 1, ACM_PACKETSIZE);
	usb_ep_out_enable(ACM_ENDPOINT);
}
#endif

static void
usb_handle_endpoints(void)
{
	uint32_t flags = usb_ep_flags();

	if (usb_ep_flag_in_or_out(0, flags))
		usb_handle_ep0();
#ifdef ACM_DEBUG
	if (usb_ep_flag_in(ACM_ENDPOINT, flags))
		usb_handle_acm_in();
	if (usb_ep_flag_out(ACM_ENDPOINT, flags))
		usb_handle_acm_out();
#endif
}

void
USB_IRQHandler(void)
{
	uint32_t flags = usb_flags();

	usb_flags_clear(flags);

	gpio_clear(LED_GREEN);

	/* we ought to check the endpoint flag
	 * but most likely we're interrupted because
	 * of an endpoint, so just check endpoint
	 * status every time
	if (usb_flag_ep(flags))
	*/
	usb_handle_endpoints();

	if (flags & (USB_GINTSTS_RESETDET | USB_GINTSTS_WKUPINT)) {
		debug("WAKEUP.. ");
		usb_unsuspend();
		debug("done\r\n");
		goto out;
	}
	if (usb_flag_suspend(flags)) {
		debug("SUSPEND.. ");
		usb_suspend();
		debug("done\r\n");
		goto out;
	}
	if (usb_flag_reset(flags)) {
		debug("RESET.. ");
		usb_reset();
		debug("done\r\n");
	}
	if (usb_flag_enumdone(flags)) {
		debug("ENUMDONE.. ");
		usb_enumdone();
		debug("done\r\n");
	}
out:
	gpio_set(LED_GREEN);
}

static void
usb_init(void)
{
	/* enable USB clock */
	clock_usb_enable();

	/* gate usbc clock when bus is idle,
	   signal full speed device */
	usb_mode(USB_CTRL_LEMIDLEEN
			| USB_CTRL_LEMOSCCTRL_GATE
			| USB_CTRL_DMPUAP_LOW);

	/* enable USB PHY pins */
	usb_pins_enable();
	/* enable USB and USB core clock */
	clock_usbc_enable();
	/* make sure oscillator is ready and selected */
	clock_usbc_select_ushfrco();
	while (!clock_usbc_ushfrco_selected())
		/* wait */;
	/* enable clock recovery */
	clock_ushfrco_recovery_enable();
	/* wait for core to come out of reset */
	while (!usb_ahb_idle())
		/* wait */;

	/* initialise USB core */
	usb_ahb_config(USB_AHB_CONFIG_DMA_ENABLE
			| USB_AHB_CONFIG_BURST_INCR
			| USB_AHB_CONFIG_INTERRUPTS_ENABLE);
	usb_flags_clear(~0U);
	usb_flags_enable(USB_GINTMSK_ENUMDONEMSK
			| USB_GINTMSK_USBRSTMSK
			| USB_GINTMSK_USBSUSPMSK
			| USB_GINTMSK_OEPINTMSK
			| USB_GINTMSK_IEPINTMSK);

	/* initialize device */
	USB->DCFG = USB_DCFG_RESVALID_DEFAULT
	          | USB_DCFG_PERFRINT_80PCNT
	          /* | USB_DCFG_ENA32KHZSUSP */
	          | USB_DCFG_NZSTSOUTHSHK
	          | USB_DCFG_DEVSPD_FS;

	/* ignore frame numbers on iso transfers */
	USB->DCTL = USB_DCTL_IGNRFRMNUM
	          | USB_DCTL_PWRONPRGDONE
	          | USB_DCTL_CGOUTNAK
	          | USB_DCTL_SFTDISCON;

	/* setup fifo allocation */
	usb_allocate_buffers(USB_FIFO_RXSIZE,
			USB_FIFO_TX0SIZE,
			USB_FIFO_TX1SIZE,
			USB_FIFO_TX2SIZE,
			USB_FIFO_TX3SIZE);

	usb_ep_reset();

	/* enable interrupt */
	NVIC_SetPriority(USB_IRQn, 1);
	NVIC_EnableIRQ(USB_IRQn);

	usb_connect();
}

static inline uint8_t
system_getprodrev(void)
{
	return (DEVINFO->PART & _DEVINFO_PART_PROD_REV_MASK)
		>> _DEVINFO_PART_PROD_REV_SHIFT;
}

void __noreturn
main(void)
{
#if 0
	if (system_getprodrev() <= 129) {
		/* This fixes a mistaken internal connection between PC0 and PC4 */
		/* This disables an internal pulldown on PC4 */
		*(volatile uint32_t*)(0x400C6018) = (1 << 26) | (5 << 0);
		/* This disables an internal LDO test signal driving PC4 */
		*(volatile uint32_t*)(0x400C80E4) &= ~(1 << 24);
	}
#endif

	/* switch to 48MHz / 2 ushfrco as core clock */
	clock_ushfrco_48mhz_div2();
	clock_ushfrco_enable();
	while (!clock_ushfrco_ready())
		/* wait */;
	clock_hfclk_select_ushfrco();
	while (!clock_ushfrco_selected())
		/* wait */;
	clock_lfrco_enable();
	clock_hfrco_disable();
	clock_auxhfrco_disable();

	clock_le_enable();
#ifdef LEUART_DEBUG
	/* route 24MHz core clock / 2 / 8 to LEUART0 */
	/* clock_le_div2(); bootup default */
	clock_lf_config(CLOCK_LFA_DISABLED | CLOCK_LFB_CORECLK | CLOCK_LFC_LFRCO);
	clock_leuart0_div8();
	clock_leuart0_enable();
#else
	clock_lf_config(CLOCK_LFA_DISABLED | CLOCK_LFB_DISABLED | CLOCK_LFC_LFRCO);
#endif
	clock_usble_enable();
	while (clock_lf_syncbusy())
		/* wait */;

	/* enable and configure GPIOs */
	clock_gpio_enable();
	gpio_set(LED_RED);
	gpio_set(LED_GREEN);
	gpio_set(LED_BLUE);
	gpio_mode(LED_RED,   GPIO_MODE_WIREDAND);
	gpio_mode(LED_GREEN, GPIO_MODE_WIREDAND);
	gpio_mode(LED_BLUE,  GPIO_MODE_WIREDAND);
#ifdef LEUART_DEBUG
	gpio_mode(GPIO_PD4, GPIO_MODE_PUSHPULL); /* LEUART0 TX */
	gpio_mode(GPIO_PD5, GPIO_MODE_INPUT);    /* LEUART0 RX */

	leuart0_freeze();
	leuart0_config(LEUART_CONFIG_8N1);
	leuart0_clock_div(3077); /* 256*(f/115200 - 1), f = 24MHz / 2 / 8 */
	leuart0_pins(LEUART_PINS_RXTX0); /* RX -> PD5, TX -> PD4 */
	leuart0_rxtx_enable();
	leuart0_update();
	while (leuart0_syncbusy())
		/* wait */;

	/* enable interrupt */
	NVIC_SetPriority(LEUART0_IRQn, 2);
	NVIC_EnableIRQ(LEUART0_IRQn);
#endif

	gpio_clear(LED_GREEN);
	usb_init();
	gpio_set(LED_GREEN);

	/* sleep when not interrupted */
	while (1) {
		__WFI();
	}
}

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

#include "lib/clock.c"
#include "lib/gpio.c"
#include "lib/leuart0.c"
#include "lib/usart1.c"
#include "lib/timer1.c"
#include "lib/timer2.c"
#include "lib/usb.c"
#include "lib/dma.c"
#include "lib/prs.c"

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

#define USB_FIFO_RXSIZE  512
#define USB_FIFO_TX0SIZE 128
#define USB_FIFO_TX1SIZE 64
#define USB_FIFO_TX2SIZE 64
#define USB_FIFO_TX3SIZE 128

#define AC_INTERFACE 0
#define AC_ENDPOINT 2
#define AC_PACKETSIZE 2

#define AS_INTERFACE 1
#define AS_ENDPOINT 1
#define AS_PACKETSIZE 196
#define AS_DMA 0

#define DFU_INTERFACE 2

#define LEUART_DMA 1

#ifdef ACM_DEBUG
#define CDC_INTERFACE 3
#define CDC_ENDPOINT 4 /* only used in descriptor */
#define CDC_PACKETSIZE 64
#define ACM_INTERFACE 4
#define ACM_ENDPOINT 3
#define ACM_PACKETSIZE 64
#endif

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
	.wTotalLength         = 9 + 122 + 18 + 58,
	.bNumInterfaces       = 2 + 1 + 2,
#else
	.wTotalLength         = 9 + 122 + 18,
	.bNumInterfaces       = 2 + 1,
#endif
	.bConfigurationValue  = 1,
	.iConfiguration       = 0,
	.bmAttributes         = 0x80,
	.bMaxPower            = 250,
	.rest = {
	/* Interface */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x04, /* Interface */
	/* .bInterfaceNumber       */ AC_INTERFACE,
	/* .bAlternateSetting      */ 0,
	/* .bNumEndpoints          */ 1,
	/* .bInterfaceClass        */ 0x01, /* 0x01 = Audio */
	/* .bInterfaceSubClass     */ 0x01, /* 0x01 = Audio Control */
	/* .bInterfaceProtocol     */ 0x00, /* 0x00 = Interface Protocol Version 1.0 */
	/* .iInterface             */ 0,
	/* AudioControl Interface: Header */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x01, /* 0x01 = Header */
	/* .bcdADC                 */ USB_WORD(0x0100),
	/* .wTotalLength           */ USB_WORD(43),
	/* .bInCollection          */ 1,
	/* .baInterfaceNr(1)       */ AS_INTERFACE,
	/* AudioControl Interface: Input Terminal */
	/* .bLength                */ 12,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x02, /* Input Terminal */
	/* .bTerminalID            */ 1,
	/* .wTerminalType          */ USB_WORD(0x0101), /* 0x0101 = USB Streaming */
	/* .bAssocTerminal         */ 0, /* none */
	/* .bNrChannels            */ 2,
	/* .wChannelConfig         */ USB_WORD(0x0003),
	/* .iChannelNames          */ 0,
	/* .iTerminal              */ 5,
	/* AudioControl Interface: Feature Unit */
	/* .bLength                */ 13, /* 7 + n * (1 + channels) */
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x06, /* Feature Unit */
	/* .bUnitID                */ 2,
	/* .bSourceID              */ 1, /* id of input terminal above */
	/* .bControlSize           */ 2,
	/* .bmaControls(0) master  */ USB_WORD(0x0003), /* Mute + Volume Control */
	/* .bmaControls(1) left    */ USB_WORD(0x0000),
	/* .bmaControls(2) right   */ USB_WORD(0x0000),
	/* .iFeature               */ 0,
	/* AudioControl Interface: Output Terminal */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x03, /* Output Terminal */
	/* .bTerminalID            */ 3,
#if 0
	/* .wTerminalType          */ USB_WORD(0x0305), /* 0x0305 = Room Speaker */
#elif 0
	/* .wTerminalType          */ USB_WORD(0x0302), /* 0x0302 = Headphones */
#else
	/* .wTerminalType          */ USB_WORD(0x0603), /* 0x0603 = Line Connector */
#endif
	/* .bAssocTerminal         */ 0, /* none */
	/* .bSourceID              */ 2, /* id of feature unit above */
	/* .iTerminal              */ 0,
	/* Endpoint */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x05, /* Endpoint */
	/* .bEndpointAddress       */ 0x80 | AC_ENDPOINT, /* in */
	/* .bmAttributes           */ 0x03, /* interrupt */
	/* .wMaxPacketSize         */ USB_WORD(AC_PACKETSIZE),
	/* .bInterval              */ 100, /* check every 100ms */
	/* .bRefresh               */ 0,
	/* .bSynchAddress          */ 0x00,
	/* Interface */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x04, /* Interface */
	/* .bInterfaceNumber       */ AS_INTERFACE,
	/* .bAlternateSetting      */ 0,
	/* .bNumEndpoints          */ 0,
	/* .bInterfaceClass        */ 0x01, /* 0x01 = Audio */
	/* .bInterfaceSubClass     */ 0x02, /* 0x02 = Audio Streaming */
	/* .bInterfaceProtocol     */ 0x00, /* 0x00 = Interface Protocol Version 1.0 */
	/* .iInterface             */ 0,
	/* Interface */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x04, /* Interface */
	/* .bInterfaceNumber       */ AS_INTERFACE,
	/* .bAlternateSetting      */ 1,
	/* .bNumEndpoints          */ 2,
	/* .bInterfaceClass        */ 0x01, /* 0x01 = Audio */
	/* .bInterfaceSubClass     */ 0x02, /* 0x02 = Audio Streaming */
	/* .bInterfaceProtocol     */ 0x00, /* 0x00 = Interface Protocol Version 1.0 */
	/* .iInterface             */ 0,
	/* AudioStreaming Interface: General */
	/* .bLength                */ 7,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x01, /* General */
	/* .bTerminalLink          */ 1, /* id of input terminal above */
	/* .bDelay                 */ 1,
	/* .wFormatTag             */ USB_WORD(0x0001), /* PCM format */
	/* AudioStreaming Interface: Format Type 1 */
	/* .bLength                */ 11,
	/* .bDescriptorType        */ 0x24, /* CS_INTERFACE */
	/* .bDescriptorSubtype     */ 0x02, /* Format Type */
	/* .bFormatType            */ 0x01, /* 0x01 = Type 1 */
	/* .bNrChannels            */ 2,
	/* .bSubFrameSize          */ 2,    /* 2 bytes */
	/* .bBitResolution         */ 16,   /* 16 bit resolution */
	/* .bSamFreqType           */ 1,    /* one sampling frequency */
	/* .tSamFreq               */ USB_TRIPLE(48000),
	/* Endpoint */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x05, /* Endpoint */
	/* .bEndpointAddress       */ AS_ENDPOINT, /* out */
	/* .bmAttributes           */ 0x05, /* Isochronous, async */
	/* .wMaxPacketSize         */ USB_WORD(AS_PACKETSIZE),
	/* .bInterval              */ 1,
	/* .bRefresh               */ 0,
	/* .bSynchAddress          */ 0x80 | AS_ENDPOINT,
	/* AudioStreaming Endpoint: General */
	/* .bLength                */ 7,
	/* .bDescriptorType        */ 0x25, /* CS_ENDPOINT */
	/* .bDescriptorSubtype     */ 0x01, /* General */
	/* .bmAttributes           */ 0x00, /* No freq control, no pitch control */
	/* .bLockDelayUnits        */ 0,
	/* .wLockDelay             */ USB_WORD(0),
	/* Endpoint */
	/* .bLength                */ 9,
	/* .bDescriptorType        */ 0x05, /* Endpoint */
	/* .bEndpointAddress       */ 0x80 | AS_ENDPOINT, /* in */
	/* .bmAttributes           */ 0x11, /* Isochronous, feedback */
	/* .wMaxPacketSize         */ USB_WORD(3),
	/* .bInterval              */ 1,
	/* .bRefresh               */ 2, /* 1 is valid, but doesn't work with windows */
	/* .bSynchAddress          */ 0x00,
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
	.bLength         = 26,
	.bDescriptorType = 0x03, /* String */
	.wCodepoint = {
		'G','e','c','k','o','B','l','a','s','t','e','r'
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

static const __align(4) struct usb_descriptor_string usb_descriptor_test = {
	.bLength         = 26,
	.bDescriptorType = 0x03, /* String */
	.wCodepoint = {
		'C','r','a','z','y',' ','S','o','u','n','d','s',
	},
};

static const struct usb_descriptor_string *const usb_descriptor_string[] = {
	&usb_descriptor_string0,
	&usb_descriptor_vendor,
	&usb_descriptor_product,
	&usb_descriptor_serial,
	&usb_descriptor_dfu,
	&usb_descriptor_test,
};

static DMA_DESCRIPTORS(dma);

static struct {
	uint32_t bytes;
	uint32_t packetsize;
	uint8_t out_disabling;
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

static void
usb_ep_out_disable_start(unsigned int nr)
{
	usb_state.out_disabling++;
	usb_global_out_nak_set();
	usb_ep_out_disable(nr);
}

static void
usb_ep_out_disable_done(unsigned int nr)
{
	if (--usb_state.out_disabling == 0)
		usb_global_out_nak_clear();
	usb_ep_out_config_disabled(nr);
	usb_ep_flag_out_disable(nr);
}

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

#ifdef LEUART_DMA
	if (dma_channel_enabled(LEUART_DMA))
		return;

	if (last < first) {
		leuart0_output.first = 0;
		last = ARRAY_SIZE(leuart0_output.buf);
	} else
		leuart0_output.first = last;

	dma_channel_alternate_disable(LEUART_DMA);
	dma.primary[LEUART_DMA].src_end = &leuart0_output.buf[last-1];
	dma.primary[LEUART_DMA].control =
		  (3 << 30)  /* dst_inc, no increment */
		| (0 << 28)  /* dst_size, 1 byte */
		| (0 << 26)  /* src_inc, 1 byte */
		| (0 << 24)  /* src_size, 1 byte */
		| (0 << 21)  /* dst_prot_ctrl, non-privileged */
		| (0 << 18)  /* src_prot_ctrl, non-privileged */
		| (0 << 14)  /* R_power, arbitrate everytime */
		| (((last - first) - 1) << 4) /* n_minus_1 */
		| (0 <<  3)  /* next_useburst, for scatter-gather */
		| (1 <<  0); /* cycle_ctrl, basic */
	dma_channel_enable(LEUART_DMA);
#else
	leuart0_txdata(leuart0_output.buf[first++]);
	leuart0_output.first = first % ARRAY_SIZE(leuart0_output.buf);
#endif
}
#endif

#ifdef ACM_DEBUG
static __uninitialized union {
	uint32_t word[ACM_PACKETSIZE/sizeof(uint32_t)];
	uint8_t byte[ACM_PACKETSIZE];
} acm_outbuf;

static struct {
	volatile uint32_t first;
	volatile uint32_t last;
	uint8_t buf[1024];
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
#endif

#ifndef NDEBUG
ssize_t
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

static const uint16_t pwm_steps[] = {
	0, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610,
	987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368
};

static uint8_t usbac_mute;
static int16_t usbac_volume;
#define USBAC_RES 256
#define USBAC_MIN (-(int16_t)(ARRAY_SIZE(pwm_steps) - 1) * 256)
#define USBAC_MAX (0 * 256)

static __align(4) struct {
	uint8_t bStatusType;
	uint8_t bOriginator;
} usbac_inbuf;

static volatile uint32_t usbas_feedback;

struct usbas_buf {
	uint32_t samples;
	union {
		uint8_t byte[AS_PACKETSIZE];
		uint32_t word[AS_PACKETSIZE/sizeof(uint32_t)];
	};
};

static struct usbas_buf usbas_buf[3];
static struct usbas_buf *volatile usbas_next;
static struct usbas_buf *volatile dma_next;

static struct usbas_buf *
usbas_nextbuf(struct usbas_buf *p)
{
	p++;
	if (p > &usbas_buf[ARRAY_SIZE(usbas_buf) - 1])
		p = &usbas_buf[0];
	return p;
}

static void
pwm_init(void)
{
	clock_timer2_enable();
	timer2_config(TIMER_CONFIG_UP);
	timer2_pins(TIMER_PINS_LOCATION0
	          /*| TIMER_PINS_CC2_ENABLE */
	          /*| TIMER_PINS_CC1_ENABLE */
	          | TIMER_PINS_CC0_ENABLE);
	timer2_top_set(pwm_steps[ARRAY_SIZE(pwm_steps)-1]);
	timer2_cc_config(0, TIMER_CC_CONFIG_PWM | TIMER_CC_CONFIG_INVERT);
	/*
	timer2_cc_config(1, TIMER_CC_CONFIG_PWM | TIMER_CC_CONFIG_INVERT);
	timer2_cc_config(2, TIMER_CC_CONFIG_PWM | TIMER_CC_CONFIG_INVERT);
	*/
}

static inline void
pwm_on(void)
{
	timer2_start();
}

static inline void
pwm_off(void)
{
	timer2_stop();
}

static void
pwm_set(void)
{
	unsigned int i;

	if (!usbac_mute) {
		i = (int)usbac_volume - USBAC_MIN;
		i *= ARRAY_SIZE(pwm_steps) - 1;
		i += (USBAC_MAX - USBAC_MIN)/2;
		i /= USBAC_MAX - USBAC_MIN;
	} else
		i = 0;

	timer2_cc_buffer_set(0, pwm_steps[i]);
}

static void
dma_zerosamples(struct dma_descriptor *d, unsigned int samples)
{
	static const int16_t zerosample;

	d->src_end = (void*)&zerosample;
	d->control =
		  (3 << 30)  /* dst_inc, no increment */
		| (1 << 28)  /* dst_size, 2 bytes */
		| (3 << 26)  /* src_inc, no increment */
		| (1 << 24)  /* src_size, 2 bytes */
		| (0 << 21)  /* dst_prot_ctrl, non-privileged */
		| (0 << 18)  /* src_prot_ctrl, non-privileged */
		| (0 << 14)  /* R_power, arbitrate everytime */
		| ((2*samples - 1) << 4) /* n_minus_1 */
		| (0 <<  3)  /* next_useburst, for scatter-gather */
		| (3 <<  0); /* cycle_ctrl, ping-pong */
	d->user = samples;
}

static void
usbac_notify_host(void)
{
	usbac_inbuf.bStatusType = 0x00; /* 0x00 = Audio Control interface */
	usbac_inbuf.bOriginator = 2;
	usb_ep_in_dma_address_set(AC_ENDPOINT, &usbac_inbuf);
	usb_ep_in_transfer_size(AC_ENDPOINT, 1, 2);
	usb_ep_in_enable(AC_ENDPOINT);
}

static void
usbac_mute_set(uint8_t v)
{
	usbac_mute = v;
	debug("  mute = %hu\r\n", usbac_mute);
	pwm_set();
}

static void
usbac_volume_set(int16_t v)
{
	usbac_volume = v;
	debug("  volume = %hd\r\n", usbac_volume);
	pwm_set();
}

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
dumpreq(const struct usb_packet_setup *p)
{
#ifndef NDEBUG
	uint8_t iface = p->wIndex & 0xFF;
	uint8_t unit = p->wIndex >> 8;
	uint8_t channel = p->wValue & 0xFF;
	uint8_t control = p->wValue >> 8;
	const char *typ;
	const char *req;

	if (p->bRequest & 0x80)
		typ = "GET";
	else
		typ = "SET";

	switch (p->bRequest & 0x7F) {
	case 1: req = "CUR"; break;
	case 2: req = "MIN"; break;
	case 3: req = "MAX"; break;
	case 4: req = "RES"; break;
	case 5: req = "MEM"; break;
	case 0x7f: req = "STAT"; break;
	default: req = "UNKNOWN"; break;
	}

	debug("%s_%s: iface = %hu, unit = %hu, "
	      "control = %hu, channel = %hu, wLength = %hu\r\n",
			typ, req, iface, unit, control, channel, p->wLength);
#endif
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
usb_handle_get_status_endpoint(const struct usb_packet_setup *p, const void **data)
{
	debug("GET_STATUS endpoint\r\n");

	if (p->wIndex == AS_ENDPOINT) {
		usb_inbuf.u16[0] = 0;
		*data = &usb_inbuf;
		return 2;
	}

#ifdef ACM_DEBUG
	if (p->wIndex == ACM_ENDPOINT) {
		usb_inbuf.u16[0] = 0;
		*data = &usb_inbuf;
		return 2;
	}
#endif

	return -1;
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
		/* configure audio control interrupt endpoint */
		usb_ep_in_config_interrupt(AC_ENDPOINT, AC_PACKETSIZE);
		usb_ep_flag_in_enable(AC_ENDPOINT);
#ifdef ACM_DEBUG
		/* configure CDC endpoint */
		/* the chip doesn't support ep 4, but we never
		 * use the  cdc interrupt anyway
		usb_ep_in_config_interrupt(CDC_ENDPOINT, CDC_PACKETSIZE);
		usb_ep_flag_in_enable(CDC_ENDPOINT);
		*/

		/* configure ACM endpoints */
		usb_ep_in_config_bulk(ACM_ENDPOINT, ACM_PACKETSIZE);

		usb_ep_out_dma_address_set(ACM_ENDPOINT, &acm_outbuf);
		usb_ep_out_transfer_size(ACM_ENDPOINT, 1, ACM_PACKETSIZE);
		usb_ep_out_config_bulk_enabled(ACM_ENDPOINT, ACM_PACKETSIZE);

		usb_ep_flag_inout_enable(ACM_ENDPOINT);

		acm_idle = true;
#endif
		return 0;
	}

	return -1;
}

static int
usb_handle_set_interface(const struct usb_packet_setup *p, const void **data)
{
	debug("SET_INTERFACE: wIndex = %hu, wValue = %hu\r\n", p->wIndex, p->wValue);

	if (p->wIndex == AC_INTERFACE && p->wValue == 0)
		return 0;

	if (p->wIndex == AS_INTERFACE) {
		switch (p->wValue) {
		case 0:
			usb_low_energy_enable();
			if (usb_ep_in_active(AS_ENDPOINT))
				usb_ep_in_disable(AS_ENDPOINT);
			if (usb_ep_out_active(AS_ENDPOINT))
				usb_ep_out_disable_start(AS_ENDPOINT);
			usart1_trigger_config(0);
			dma_next = NULL;
			return 0;
		case 1:
			usb_low_energy_disable();
			usbas_feedback = 48 << 14;
			usb_ep_in_dma_address_set(AS_ENDPOINT, (void *)&usbas_feedback);
			usb_ep_in_transfer_size(AS_ENDPOINT, 1, 3);
			usb_ep_in_config_iso_enabled(AS_ENDPOINT, 3);

			usbas_next = dma_next = &usbas_buf[0];
			usb_ep_out_dma_address_set(AS_ENDPOINT, usbas_buf[0].word);
			usb_ep_out_transfer_size(AS_ENDPOINT, 1, AS_PACKETSIZE);
			usb_ep_out_config_iso_enabled(AS_ENDPOINT, AS_PACKETSIZE);

			usb_ep_flag_inout_enable(AS_ENDPOINT);

			dma_channel_disable(AS_DMA);
			dma_channel_alternate_disable(AS_DMA);
			usart1_tx_disable();
			usart1_tx_clear();
			/* there is a bug in the EFM32 usart,
			 * so we need to disable i2s mode to
			 * reset its internal state properly */
			usart1_i2s_disable();
			dma_zerosamples(&dma.primary[AS_DMA],   48);
			dma_zerosamples(&dma.alternate[AS_DMA], 48);
			usart1_i2s_16bit_stereo();
			dma_channel_enable(AS_DMA);
			/* enable tx at next usb sof */
			usart1_trigger_config(USART_TRIGCTRL_TXTEN | USART_TRIGCTRL_TSEL_PRSCH0);
			return 0;
		}
	}

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

	if (p->wIndex == AS_ENDPOINT)
		return 0;
#ifdef ACM_DEBUG
	if (p->wIndex == CDC_ENDPOINT)
		return 0;
	if (p->wIndex == ACM_ENDPOINT)
		return 0;
#endif

	return -1;
}

static int
usb_handle_set_cur(const struct usb_packet_setup *p, const void **data)
{
	dumpreq(p);

	if (p->wIndex == (0x0200 | AC_INTERFACE) && p->wValue == 0x0100) {
		if (p->wLength != 1)
			return -1;

		const uint8_t *v = *data;
		usbac_mute_set(*v);
		return 0;
	}

	if (p->wIndex == (0x0200 | AC_INTERFACE) && p->wValue == 0x0200) {
		if (p->wLength != 2)
			return -1;

		const int16_t *v = *data;
		usbac_volume_set(*v);
		return 0;
	}

	return -1;
}

static int
usb_handle_set_res(const struct usb_packet_setup *p, const void **data)
{
	dumpreq(p);

	if (p->wIndex == (0x0200 | AC_INTERFACE) && p->wValue == 0x0200) {
		if (p->wLength != 2)
			return -1;
#ifndef NDEBUG
		const int16_t *v = *data;
		debug("  res = %hd\r\n", *v);
#endif
	}

	return -1;
}


static int
usb_handle_get_cur(const struct usb_packet_setup *p, const void **data)
{
	dumpreq(p);

	if (p->wIndex == (0x0200 | AC_INTERFACE) && p->wValue == 0x0100) {
		usb_inbuf.u8[0] = usbac_mute;
		*data = &usb_inbuf;
		return 1;
	}

	if (p->wIndex == (0x0200 | AC_INTERFACE) && p->wValue == 0x0200) {
		usb_inbuf.u16[0] = usbac_volume;
		*data = &usb_inbuf;
		return 2;
	}

	return -1;
}

static int
usb_handle_get_min(const struct usb_packet_setup *p, const void **data)
{
	dumpreq(p);

	if (p->wIndex == (0x0200 | AC_INTERFACE) && p->wValue == 0x0200) {
		usb_inbuf.i16[0] = USBAC_MIN;
		*data = &usb_inbuf;
		debug("  returning %hd\r\n", USBAC_MIN);
		return 2;
	}

	return -1;
}

static int
usb_handle_get_max(const struct usb_packet_setup *p, const void **data)
{
	dumpreq(p);

	if (p->wIndex == (0x0200 | AC_INTERFACE) && p->wValue == 0x0200) {
		usb_inbuf.i16[0] = USBAC_MAX;
		*data = &usb_inbuf;
		debug("  returning %hd\r\n", USBAC_MAX);
		return 2;
	}

	return -1;
}

static int
usb_handle_get_res(const struct usb_packet_setup *p, const void **data)
{
	dumpreq(p);

	if (p->wIndex == (0x0200 | AC_INTERFACE) && p->wValue == 0x0200) {
		usb_inbuf.i16[0] = USBAC_RES;
		*data = &usb_inbuf;
		debug("  returning %hd\r\n", USBAC_RES);
		return 2;
	}

	return -1;
}

static int
usb_handle_get_stat(const struct usb_packet_setup *p, const void **data)
{
	dumpreq(p);

	if ((p->wIndex & 0xff) == AC_INTERFACE && p->wValue == 0) {
		*data = &usb_inbuf;
		return 0;
	}

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
		return 0;
	}

	return -1;
}

static int
usb_handle_set_line_coding(const struct usb_packet_setup *p, const void **data)
{
	debug("SET_LINE_CODING: wIndex = %hu wValue = 0x%04hx\r\n",
			p->wIndex, p->wValue);

	if (p->wIndex == CDC_INTERFACE) {
		const struct {
			uint32_t dwDTERate;
			uint8_t bCharFormat;
			uint8_t bParityType;
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
	{ .req = 0x0082, .len = -1, .fn = usb_handle_get_status_endpoint },
	{ .req = 0x0500, .len =  0, .fn = usb_handle_set_address },
	{ .req = 0x0680, .len = -1, .fn = usb_handle_get_descriptor },
	{ .req = 0x0880, .len = -1, .fn = usb_handle_get_configuration },
	{ .req = 0x0900, .len =  0, .fn = usb_handle_set_configuration },
	{ .req = 0x0b01, .len =  0, .fn = usb_handle_set_interface },
	{ .req = 0x0102, .len =  0, .fn = usb_handle_clear_feature_endpoint },
	{ .req = 0x0121, .len = -1, .fn = usb_handle_set_cur },
	{ .req = 0x0421, .len = -1, .fn = usb_handle_set_res },
	{ .req = 0x81a1, .len = -1, .fn = usb_handle_get_cur },
	{ .req = 0x82a1, .len = -1, .fn = usb_handle_get_min },
	{ .req = 0x83a1, .len = -1, .fn = usb_handle_get_max },
	{ .req = 0x84a1, .len = -1, .fn = usb_handle_get_res },
	{ .req = 0xffa1, .len = -1, .fn = usb_handle_get_stat },
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

	//debug("EP0 %04lx %04lx %lu\r\n", oflags, iflags, usb_state.bytes);

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

static void
usb_handle_audio_streaming_feedback(void)
{
	uint32_t flags = usb_ep_in_flags(AS_ENDPOINT);

	usb_ep_in_flags_clear(AS_ENDPOINT, flags);

	if (usb_ep_in_flag_disabled(flags)) {
		usb_ep_flag_in_disable(AS_ENDPOINT);
		usb_fifo_tx_flush(AS_ENDPOINT);
		usb_ep_in_config_disabled(AS_ENDPOINT);
		return;
	}

	if (usb_ep_in_flag_complete(flags)) {
		usb_ep_in_dma_address_set(AS_ENDPOINT, (void *)&usbas_feedback);
		usb_ep_in_transfer_size(AS_ENDPOINT, 1, 3);
		usb_ep_in_enable(AS_ENDPOINT);
	}
}

static void
usb_handle_audio_streaming_out(void)
{
	uint32_t flags = usb_ep_out_flags(AS_ENDPOINT);

	usb_ep_out_flags_clear(AS_ENDPOINT, flags);

	if (usb_ep_out_flag_disabled(flags)) {
		usb_ep_out_disable_done(AS_ENDPOINT);
		return;
	}

	if (usb_ep_out_flag_complete(flags)) {
		uint32_t bytes = AS_PACKETSIZE - usb_ep_out_bytes_left(AS_ENDPOINT);
		struct usbas_buf *next = usbas_next;

		if ((bytes & 3) || bytes < 47*4) {
			debug("wat? received %lu bytes\r\n", bytes);
		} else if (next == dma_next) {
			next->samples = bytes/4;
			next = usbas_nextbuf(next);
			usbas_next = next;
		}
		usb_ep_out_dma_address_set(AS_ENDPOINT, next->word);
		usb_ep_out_transfer_size(AS_ENDPOINT, 1, AS_PACKETSIZE);
		usb_ep_out_enable(AS_ENDPOINT);
	}
}

static void
usb_handle_audio_control_in(void)
{
	uint32_t flags = usb_ep_in_flags(AC_ENDPOINT);

	usb_ep_in_flags_clear(AC_ENDPOINT, flags);

	debug("AC IN\r\n");
}

#ifdef ACM_DEBUG
static void
usb_handle_acm_in(void)
{
	uint32_t flags = usb_ep_in_flags(ACM_ENDPOINT);

	usb_ep_in_flags_clear(ACM_ENDPOINT, flags);

	if (usb_ep_in_flag_timeout(flags)) {
		debug("timeout!!\r\n");
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
	static bool twog;

	usb_ep_out_flags_clear(ACM_ENDPOINT, flags);

	if (!usb_ep_out_flag_complete(flags))
		return;

	bytes = ACM_PACKETSIZE - usb_ep_out_bytes_left(ACM_ENDPOINT);
	debug("ACM: received %lu bytes\r\n", bytes);

	//acm_write(acm_outbuf.byte, bytes);

	for (uint32_t i = 0; i < bytes; i++) {
		switch (acm_outbuf.byte[i]) {
		case 'p':
			debug("first %lu last %lu idle %u\r\n",
					acm_inbuf.first,
					acm_inbuf.last,
					acm_idle);
			break;
		case 's':
			twog = false;
			debug("mute %hu volume %hd\r\n",
					usbac_mute, usbac_volume);
			break;
		case 'm':
			usbac_mute_set(!usbac_mute);
			usbac_notify_host();
			break;
		case 'u':
			if (usbac_volume < USBAC_MAX) {
				usbac_volume_set(usbac_volume + USBAC_RES);
				usbac_notify_host();
			}
			break;
		case 'd':
			if (usbac_volume > USBAC_MIN) {
				usbac_volume_set(usbac_volume - USBAC_RES);
				usbac_notify_host();
			}
			break;
		case 'g':
			if (!twog) {
				twog = true;
				continue;
			}
			if (timer1_flag_cc_enabled(0)) {
				timer1_flag_cc_disable(0);
			} else {
				timer1_flag_cc_enable(0);
			}
			break;
		case 'c':
			debug("ep1in:    0x%08lx\r\n", USB->DIEP[0].CTL);
			debug("ep1in:    0x%08lx\r\n", USB->DIEP[0].INT);
			debug("ep1out:   0x%08lx\r\n", USB->DOEP[0].CTL);
			debug("ep1out:   0x%08lx\r\n", USB->DOEP[0].INT);
			debug("daintmsk: 0x%08lx\r\n", USB->DAINTMSK);
			break;
		}
		twog = false;
	}

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
	if (usb_ep_flag_in(AS_ENDPOINT, flags))
		usb_handle_audio_streaming_feedback();
	if (usb_ep_flag_out(AS_ENDPOINT, flags))
		usb_handle_audio_streaming_out();
	if (usb_ep_flag_in(AC_ENDPOINT, flags))
		usb_handle_audio_control_in();
#ifdef ACM_DEBUG
	if (usb_ep_flag_out(ACM_ENDPOINT, flags))
		usb_handle_acm_out();
	if (usb_ep_flag_in(ACM_ENDPOINT, flags))
		usb_handle_acm_in();
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

static uint32_t
usbas_calculate_feedback(uint32_t p, int32_t ts, unsigned int samples)
{
#ifndef NDEBUG
	static unsigned int count;
#endif
	static int32_t last_ts;
	uint32_t t = p + ts - last_ts;
	uint32_t r = samples * p;

	r -= 46*t;
	r <<= 15;
	r += t;
	r /= t;
	r >>= 1;
	r += 46 << 14;

	if (ts > 750 || ts < -750)
		r -= ts/8;

#ifndef NDEBUG
	count++;
	if (count == 100) {
		count = 0;
		debug("DMA: %lu %ld %u\r\n", r, ts, samples);
	}
#endif

	last_ts = ts;
	return r;
}

void
DMA_IRQHandler(void)
{
	int32_t ts = timer1_counter();
	uint32_t flags = dma_flags();
	struct usbas_buf *next;
	struct dma_descriptor *d;
	uint32_t p;

	dma_flags_clear(flags);
	gpio_clear(LED_BLUE);

	next = dma_next;
	if (!next)
		goto out;

	p = timer1_cc_value(0) + 1;
	if (ts > 12000)
		ts -= p;

	if (dma_channel_alternate(AS_DMA))
		d = &dma.primary[AS_DMA];
	else
		d = &dma.alternate[AS_DMA];

	usbas_feedback = usbas_calculate_feedback(p, ts, d->user);

	if (next == usbas_next) {
		unsigned int samples;

		if (ts > 500)
			samples = 47;
		else if (ts < -500)
			samples = 49;
		else
			samples = 48;
		dma_zerosamples(d, samples);
		goto out;
	}
	if (ts > 4000 && next->samples >= 48) /* drop last sample */
		next->samples--;
	if (ts < -4000 && next->samples <= 48) { /* duplicate last sample */
		uint32_t tmp = next->word[next->samples-1];

		next->word[next->samples] = tmp;
		next->samples++;
	}
	d->src_end = &next->byte[4*next->samples - 2];
	d->control =
		  (3 << 30)  /* dst_inc, no increment */
		| (1 << 28)  /* dst_size, 2 bytes */
		| (1 << 26)  /* src_inc, 2 bytes */
		| (1 << 24)  /* src_size, 2 bytes */
		| (0 << 21)  /* dst_prot_ctrl, non-privileged */
		| (0 << 18)  /* src_prot_ctrl, non-privileged */
		| (0 << 14)  /* R_power, arbitrate everytime */
		| ((2*next->samples - 1) << 4) /* n_minus_1 */
		| (0 <<  3)  /* next_useburst, for scatter-gather */
		| (3 <<  0); /* cycle_ctrl, ping-pong */
	d->user = next->samples;

	dma_next = usbas_nextbuf(next);
out:
	gpio_set(LED_BLUE);
}

void
TIMER1_IRQHandler(void)
{
	uint32_t flags = timer1_flags();
	static unsigned int count;
	static uint32_t sum;

	timer1_flags_clear(flags);

	sum += timer1_cc_value(0) + 1;
	count += 1;
	if (count == 1000) {
		debug("T%lu\r\n", sum);
		sum = 0;
		count = 0;
	}
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
	clock_auxhfrco_enable();

	clock_dma_enable();
	clock_peripheral_div1();
	clock_usart1_enable();

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

	clock_prs_enable();
	prs_channel_config(0, PRS_SOURCE_USB_SOF | PRS_EDGE_BOTH);

	clock_timer1_enable();
	timer1_config(TIMER_CONFIG_UP
			| TIMER_CTRL_RISEA_RELOADSTART);
	timer1_cc_config(0, TIMER_CC_CONFIG_CAPTURE
			| TIMER_CC_CTRL_ICEDGE_RISING
			| TIMER_CC_CTRL_INSEL_PRS
			| TIMER_CC_CTRL_PRSSEL_PRSCH0);
	NVIC_SetPriority(TIMER1_IRQn, 3);
	NVIC_EnableIRQ(TIMER1_IRQn);

	dma_base_set(&dma);
	dma_enable();

	dma.primary[AS_DMA].dst_end = &USART1->TXDOUBLE;
	dma.alternate[AS_DMA].dst_end = &USART1->TXDOUBLE;
	dma_channel_config(AS_DMA, DMA_CH_CTRL_SOURCESEL_USART1
			| DMA_CH_CTRL_SIGSEL_USART1TXBL);
	//dma_channel_priority_low(AS_DMA);
	//dma_channel_alternate_disable(AS_DMA);
	//dma_channel_mask_disable(AS_DMA);
#ifdef LEUART_DMA
	dma.primary[LEUART_DMA].dst_end = &LEUART0->TXDATA;
	dma_channel_config(LEUART_DMA, DMA_CH_CTRL_SOURCESEL_LEUART0
			| DMA_CH_CTRL_SIGSEL_LEUART0TXBL);
#endif

	dma_flag_error_enable();
	dma_flag_done_enable(AS_DMA);
	NVIC_SetPriority(DMA_IRQn, 0);
	NVIC_EnableIRQ(DMA_IRQn);

	/*
	 * use location 5:           J4
	 * US1_CLK -> PC3      GND-| 1  2 |-3.3v
	 * US1_CS  -> PC0      PA1-| 3  4 |-PA2
	 * US1_RX  -> PC2      PC0-| 5  6 |-PC1
	 * US1_TX  -> PC1      PC2-| 7  8 |-PC3
	usart1_pins(..);
	 */
	gpio_drive_strength_high(GPIO_PC3);
	gpio_mode(GPIO_PC3, GPIO_MODE_PUSHPULLDRIVE);
	gpio_mode(GPIO_PC0, GPIO_MODE_PUSHPULLDRIVE);
	gpio_mode(GPIO_PC1, GPIO_MODE_PUSHPULLDRIVE);
	usart1_pins(USART_ROUTE_LOCATION_LOC5
			| USART_ROUTE_CLKPEN
			| USART_ROUTE_CSPEN
			| USART_ROUTE_TXPEN);
	usart1_config(USART_CTRL_MSBF | USART_CTRL_SYNC);
	/* bitrate = peripheral / (2 * (1 + usart_clkdiv / 256))
	 * 2 * (1 + usart_clkdiv/256) = peripheral / bitrate
	 * 1 + usart_clkdiv/256 = peripheral / (2*bitrate)
	 * usart_clkdiv/256 = (peripheral / (2*bitrate)) - 1
	 * usart_clkdiv = 256 * ((peripheral / (2*bitrate)) - 1)
	 * usart_clkdiv = 256 * ((24MHz / 2*48kHz*2*16) - 1) = 1744
	 */
	usart1_clock_div(1744);
	usart1_frame_16bit();
	usart1_master_enable();

	pwm_init();
	pwm_set();
	pwm_on();

	/* not needed when usbas_buf is in .bss
	for (unsigned int i = 0; i < ARRAY_SIZE(usbas_buf); i++)
		usbas_buf[i].samples = 0;
	*/

	gpio_clear(LED_GREEN);
	usb_init();
	gpio_set(LED_GREEN);

	/* sleep when not interrupted */
	while (1) {
		__WFI();
	}
}

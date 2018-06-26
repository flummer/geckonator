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

#include "geckonator/clock.h"
#include "geckonator/gpio.h"
#include "geckonator/usart1.h"

#include "swd.h"

static inline void __attribute__((always_inline))
swd__wait_complete(void)
{
	while (!usart1_tx_complete())
		/* wait */;
}

static inline void
swd__wait_txdata(void)
{
	while (!usart1_tx_buffer_level())
		/* wait */;
}

static inline void
swd__wait_rxdata(void)
{
	while (!usart1_rx_valid())
		/* wait */;
}

static int
swd__tail(int ret)
{
	swd__wait_complete();
	usart1_frame_bits(8);
	usart1_rx_disable_and_clear();
#ifdef SWD_PP
	usart1_tx_tristate_disable();
#endif
	return ret;
}

void
swd_init(void)
{
	clock_usart1_enable();
	/*
	 * use location 5:          J4
	 * US1_CLK --> PC3   GND-| 1  2 |-3.3v
	 * US1_CS ---> PC0   PA1-| 3  4 |-PA2
	 * US1_RX \ -> PC2   PC0-| 5  6 |-PC1
	 * US1_TX ---> PC1   PC2-| 7  8 |-PC3
	 */
	gpio_mode(GPIO_PC3, GPIO_MODE_INPUT);
	gpio_mode(GPIO_PC1, GPIO_MODE_INPUT);
	usart1_pins(USART_ROUTE_LOCATION_LOC5);
	usart1_config(USART_CTRL_CLKPOL | USART_CTRL_CLKPHA
			| USART_CTRL_LOOPBK | USART_CTRL_SYNC);
	/* bitrate = peripheral / (2 * (1 + usart_clkdiv / 256))
	 * 2 * (1 + usart_clkdiv/256) = peripheral / bitrate
	 * 1 + usart_clkdiv/256 = peripheral / (2*bitrate)
	 * usart_clkdiv/256 = (peripheral / (2*bitrate)) - 1
	 * usart_clkdiv = 256 * ((peripheral / (2*bitrate)) - 1)
	 * //usart_clkdiv = 256 * ((24MHz / 2*125kHz) - 1) = 24320
	 * usart_clkdiv = 256 * ((24MHz / 2*500kHz) - 1) = 5888
	 */
	//usart1_clock_div(24320);
	usart1_clock_div(5888);
	usart1_master_enable();
	usart1_frame_bits(8);
	usart1_rx_disable_and_clear();
	usart1_tx_enable();
}

void
swd_engage(void)
{
	gpio_mode(GPIO_PC3, GPIO_MODE_PUSHPULL);
#ifdef SWD_PP
	gpio_mode(GPIO_PC1, GPIO_MODE_PUSHPULL);
#else
	gpio_mode(GPIO_PC1, GPIO_MODE_WIREDANDPULLUPFILTER);
#endif
	usart1_pins(USART_ROUTE_LOCATION_LOC5
			| USART_ROUTE_CLKPEN
			| USART_ROUTE_TXPEN);
}

void
swd_disengage(void)
{
	usart1_pins(USART_ROUTE_LOCATION_LOC5);
	gpio_mode(GPIO_PC3, GPIO_MODE_INPUT);
	gpio_mode(GPIO_PC1, GPIO_MODE_INPUT);
}

int
swd_reset(uint32_t *dpid)
{
	unsigned int i;

	/* send 152 one bits */
	for (i = 19; i > 0; i--) {
		swd__wait_txdata();
		usart1_txdata(~0U);
	}

	/* send jtag to swd code */
	swd__wait_txdata();
	usart1_txdata(0x9EU);
	swd__wait_txdata();
	usart1_txdata(0xE7U);

	/* send 150 one bits */
	for (i = 18; i > 0; i--) {
		swd__wait_txdata();
		usart1_txdata(~0U);
	}
	/* ..and two zero bits */
	swd__wait_txdata();
	usart1_txdata(0x3FU);

	return swd_read(SWD_DP_DPIDR, dpid);
}

int
swd_read(unsigned int addr, uint32_t *v)
{
	int ret;
	uint32_t val;
	uint32_t tmp;

	/* send 8bit request and enable rx afterwards */
	swd__wait_txdata();
	usart1_txdatax(addr ^ 0x80a5);

	/* read turnaround bit, 3bit ack and one more bit
	 * which is either first response bit or turnaround
	 * in case of error */
	swd__wait_complete();
	usart1_frame_bits(5);
	usart1_txdata(~0U);
	swd__wait_rxdata();
	val = usart1_rxdata() >> 1;
	ret = val & 0x7U;
	if (ret != SWD_OK)
		return swd__tail(ret);

	val >>= 3;
	/* read 31bit of value, parity bit and turnaround bit */
	swd__wait_complete();
	usart1_frame_bits(11);
	usart1_txdouble(~0U);
	swd__wait_txdata();
	usart1_txdouble(~0U);
	swd__wait_rxdata();
	val |= usart1_rxdouble() << 1;
	swd__wait_txdata();
	usart1_txdouble(~0U);
	swd__wait_rxdata();
	val |= usart1_rxdouble() << 12;
	swd__wait_rxdata();
	tmp = usart1_rxdouble();
	val |= tmp << 23;

	if (swd_parity(val) == ((tmp >> 9) & 0x1U))
		*v = val;
	else
		ret = SWD_PARITY;

	return swd__tail(ret);
}

int
swd_write(unsigned int addr, uint32_t v)
{
	int ret;
	uint32_t tmp;

	/* send 8bit request and enable rx afterwards */
	swd__wait_txdata();
	usart1_txdatax(addr ^ 0x8081);

	/* read turnaround, 3bit ack and turnaround */
	swd__wait_complete();
	usart1_frame_bits(5);
	usart1_txdata(~0U);
	swd__wait_rxdata();
	ret = (usart1_rxdata() >> 1) & 0x7U;
	if (ret != SWD_OK)
		return swd__tail(ret);

	/* write 32bit value and 1 bit parity */
	swd__wait_complete();
	usart1_frame_bits(11);
	usart1_rx_disable_and_clear();
#ifdef SWD_PP
	usart1_tx_tristate_disable();
#endif
	usart1_txdouble(v);
	tmp = (swd_parity(v) << 10) | (v >> 22);
	v >>= 11;
	swd__wait_txdata();
	usart1_txdouble(v);
	swd__wait_txdata();
	usart1_txdouble(tmp);

	return swd__tail(ret);
}

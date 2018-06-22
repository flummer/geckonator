/*
 * This file is part of geckonator.
 * Copyright 2018 Emil Renner Berthing <esmil@esmil.dk>
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

#include "geckonator/gpio.h"
#include "geckonator/emu.h"
#include "geckonator/reset.h"

void
gpio_mode(gpio_pin_t pin, uint32_t mode)
{
	volatile uint32_t *reg;
	uint32_t pinshift;
	uint32_t mask;
	unsigned int port = gpio_port(pin);

	if (gpio_nr(pin) & 0x8U)
		reg = &GPIO->P[port].MODEH;
	else
		reg = &GPIO->P[port].MODEL;

	pinshift = 4*(gpio_nr(pin) & 0x7U);
	mask = 0xFU << pinshift;
	*reg = (*reg & ~mask) | (mode << pinshift);
}

void
gpio_flag_select(gpio_pin_t pin)
{
	volatile uint32_t *reg;
	uint32_t pinshift;
	uint32_t mask;
	unsigned int port = gpio_port(pin);

	if (gpio_nr(pin) & 0x8U)
		reg = &GPIO->EXTIPSELH;
	else
		reg = &GPIO->EXTIPSELL;

	pinshift = 4*(gpio_nr(pin) & 0x7U);
	mask = 0xFU << pinshift;
	*reg = (*reg & ~mask) | (port << pinshift);
}

void __noreturn
emu_em4_enter(void)
{
	unsigned int i;

	/* do the EM4 handshake */
	EMU->CTRL = 0;
	for (i = 0; i < 4; i++) {
		EMU->CTRL = 0x2U << 2;
		EMU->CTRL = 0x3U << 2;
	}
	EMU->CTRL = 0x2U << 2;

	/* enter EM4 */
	__WFI();
	__builtin_unreachable();
}

void
emu_reset_cause_clear(void)
{
	EMU->AUXCTRL = 1;
	EMU->AUXCTRL = 0;
}

void
reset_cause_clear_all(void)
{
	reset_cause_clear();
	EMU->AUXCTRL = EMU_AUXCTRL_HRCCLR;
	EMU->AUXCTRL = 0;
}

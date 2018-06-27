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

#include <stdint.h>
#include <stdio.h>

#include "swd.h"

#ifdef NDEBUG
#define debug(...)
#else
#define debug(...) printf(__VA_ARGS__)
#endif

#define POWER_BASE     0x40000000UL
#define POWER_RESET    (POWER_BASE + 0x544UL)

#define POWER_RESET_RESET (0x1UL << 0)

#define NVMC_BASE      0x4001E000UL
#define NVMC_READY     (NVMC_BASE + 0x400UL)
#define NVMC_CONFIG    (NVMC_BASE + 0x504UL)
#define NVMC_ERASEPAGE (NVMC_BASE + 0x508UL)
#define NVMC_ERASEPCR1 (NVMC_BASE + 0x508UL) /* yes, same as above */
#define NVMC_ERASEALL  (NVMC_BASE + 0x50CUL)
#define NVMC_ERASEPCR0 (NVMC_BASE + 0x510UL)
#define NVMC_ERASEUICR (NVMC_BASE + 0x514UL)

#define NVMC_READY_READY (1UL << 0)

#define NVMC_CONFIG_WEN_Pos 0
#define NVMC_CONFIG_WEN_Msk (0x3UL << NVMC_CONFIG_WEN_Pos)
#define NVMC_CONFIG_WEN_Ren (0x0UL << NVMC_CONFIG_WEN_Pos)
#define NVMC_CONFIG_WEN_Wen (0x1UL << NVMC_CONFIG_WEN_Pos)
#define NVMC_CONFIG_WEN_Een (0x2UL << NVMC_CONFIG_WEN_Pos)

#define NVMC_ERASEALL_ERASEALL 1UL

#define NVMC_ERASEUICR_ERASEUICR 1UL

int
nRF51_erase(void)
{
	int ret;
	uint32_t ready;

	ret = swd_mem_write(NVMC_CONFIG, NVMC_CONFIG_WEN_Een);
	if (ret != SWD_OK)
		return ret;

	ret = swd_mem_write(NVMC_ERASEALL, NVMC_ERASEALL_ERASEALL);
	if (ret != SWD_OK)
		return ret;

	do {
		ret = swd_mem_read(NVMC_READY, &ready);
		if (ret != SWD_OK)
			return ret;
	} while (!(ready & NVMC_READY_READY));

	return swd_mem_write(NVMC_CONFIG, NVMC_CONFIG_WEN_Wen);
}

int
nRF51_write(uint32_t addr, uint32_t v)
{
	int ret;
	uint32_t ready;

	do {
		ret = swd_mem_read(NVMC_READY, &ready);
		if (ret != SWD_OK)
			return ret;
	} while (!(ready & NVMC_READY_READY));

	return swd_mem_write(addr, v);
}

int
nRF51_reboot_run(void)
{
	return swd_mem_write(POWER_RESET, POWER_RESET_RESET);
}

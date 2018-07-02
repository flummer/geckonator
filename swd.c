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
#include <stdio.h>

#include "swd.h"

#define NDEBUG

#ifdef NDEBUG
#define debug(...)
#else
#define debug(...) printf(__VA_ARGS__)
#endif

uint32_t
swd_parity(uint32_t v)
{
	uint32_t parity = 0;

	while (v) {
		parity = !parity;
		v &= v - 1;
	}
	return parity;
}

#if 0
int
swd_read_10(unsigned int addr, uint32_t *v)
{
	int ret;
	unsigned int tries = 10;

	while (1) {
		ret = swd_read(addr, v);
		if (ret != SWD_WAIT)
			break;

		tries--;
		if (tries == 0)
			return SWD_TIMEOUT;
	}
	return ret;
}

int
swd_write_10(unsigned int addr, uint32_t v)
{
	uint32_t ret;
	unsigned int tries = 10;

	while (1) {
		ret = swd_write(addr, v);
		if (ret != SWD_WAIT)
			break;

		tries--;
		if (tries == 0)
			return SWD_TIMEOUT;
	}
	return ret;
}
#endif

int
swd_dp_init(uint32_t *id)
{
	int ret;
	uint32_t v;
	uint32_t stat;

	/* reset swd line and read DPIDR register */
	ret = swd_reset(&v);
	debug("(%d) dbidr  == 0x%08lx\r\n", ret, v);
	if (ret != SWD_OK)
		return ret;

	/* set APSEL = 0, APBANKSEL = 0xF, DPBANKSEL = 0 */
	ret = swd_write(SWD_DP_SELECT, 0xFU << SWD_DP_SELECT_APBANKSEL_Pos);
	debug("(%d) select <- 0x%08lx\r\n", ret, (uint32_t)(0xFU << SWD_DP_SELECT_APBANKSEL_Pos));
	if (ret != SWD_OK)
		return ret;

	/* make sure debug port is powered on, and
	 * TRNCNT, MASKLANE, TRNMODE and ORUNDETECT are cleared */
	while (1) {
		ret = swd_read(SWD_DP_STAT, &stat);
		debug("(%d) stat   == 0x%08lx\r\n", ret, stat);
		if (ret != SWD_OK)
			return ret;

		if ((stat & (SWD_DP_STAT_CDBGPWRUPACK
					| SWD_DP_STAT_TRNCNT_Msk
					| SWD_DP_STAT_MASKLANE_Msk
					| SWD_DP_STAT_TRNMODE_Msk
					| SWD_DP_STAT_ORUNDETECT))
				== SWD_DP_STAT_CDBGPWRUPACK)
			break;

		v = (stat & ~(SWD_DP_STAT_WRITEZERO_Msk
					| SWD_DP_STAT_TRNCNT_Msk
					| SWD_DP_STAT_MASKLANE_Msk
					| SWD_DP_STAT_TRNMODE_Msk
					| SWD_DP_STAT_ORUNDETECT))
			| SWD_DP_STAT_CDBGPWRUPREQ;

		ret = swd_write(SWD_DP_STAT, v);
		debug("(%d) stat   <- 0x%08lx\r\n", ret, v);
		if (ret != SWD_OK)
			return ret;
	}

	/* read IDR register */
	ret = swd_read(SWD_AP_IDR, &v);
	debug("(%d) tmp    == 0x%08lx\r\n", ret, v);
	if (ret != SWD_OK)
		return ret;
	ret = swd_read(SWD_DP_RDBUFF, id);
	debug("(%d) id     == 0x%08lx\r\n", ret, *id);
	if (ret != SWD_OK)
		return ret;

	/* switch back to APBANKSEL = 0 */
	ret = swd_write(SWD_DP_SELECT, 0);
	debug("(%d) select <- 0x%08lx\r\n", ret, 0UL);
	if (ret != SWD_OK)
		return ret;

	/* set CSW to 32bit read/write sizes */
	while (1) {
		ret = swd_read(SWD_AP_CSW, &v);
		debug("(%d) tmp    == 0x%08lx\r\n", ret, v);
		if (ret != SWD_OK)
			return ret;
		ret = swd_read(SWD_DP_RDBUFF, &v);
		debug("(%d) csw    == 0x%08lx\r\n", ret, v);
		if (ret != SWD_OK)
			return ret;

		if ((v & (SWD_AP_CSW_ADDRINC_Msk | SWD_AP_CSW_SIZE_Msk)) == SWD_AP_CSW_SIZE_32BIT)
			break;

		v = (v & ~(SWD_AP_CSW_WRITEZERO_Msk
					| SWD_AP_CSW_ADDRINC_Msk
					| SWD_AP_CSW_SIZE_Msk))
			| SWD_AP_CSW_SIZE_32BIT;
		ret = swd_write(SWD_AP_CSW, v);
		debug("(%d) swd    <- 0x%08lx\r\n", ret, v);
		if (ret != SWD_OK)
			return ret;
	}

	/* make sure system is powered on */
	while (!(stat & SWD_DP_STAT_CSYSPWRUPACK)) {
		v = (stat & ~SWD_DP_STAT_WRITEZERO_Msk) | SWD_DP_STAT_CSYSPWRUPREQ;
		ret = swd_write(SWD_DP_STAT, v);
		debug("(%d) stat   <- 0x%08lx\r\n", ret, v);
		if (ret != SWD_OK)
			return ret;

		ret = swd_read(SWD_DP_STAT, &stat);
		debug("(%d) stat   == 0x%08lx\r\n", ret, stat);
		if (ret != SWD_OK)
			return ret;
	}

	return ret;
}

int
swd_mem_read(uint32_t addr, uint32_t *v)
{
	int ret;
	uint32_t tmp;

	do {
		ret = swd_write(SWD_AP_TAR, addr);
	} while (ret == SWD_WAIT);
	debug("(%d) tar    <- 0x%08lx\r\n", ret, addr);
	if (ret != SWD_OK)
		return ret;

	ret = swd_read(SWD_AP_DRW, &tmp);
	debug("(%d) tar    == 0x%08lx\r\n", ret, tmp);
	if (ret != SWD_OK)
		return ret;

	ret = swd_read(SWD_DP_RDBUFF, v);
	debug("(%d) drw    == 0x%08lx\r\n", ret, *v);
	return ret;
}

int
swd_mem_write(uint32_t addr, uint32_t v)
{
	int ret;

	do {
		ret = swd_write(SWD_AP_TAR, addr);
	} while (ret == SWD_WAIT);
	debug("(%d) tar    <- 0x%08lx\r\n", ret, addr);
	if (ret != SWD_OK)
		return ret;

	ret = swd_write(SWD_AP_DRW, v);
	debug("(%d) drw    <- 0x%08lx\r\n", ret, v);
	return ret;
}

int
swd_mem_rmu(uint32_t addr, uint32_t mask, uint32_t enable)
{
	int ret;
	uint32_t v;

	do {
		ret = swd_write(SWD_AP_TAR, addr);
	} while (ret == SWD_WAIT);
	debug("(%d) tar    <- 0x%08lx\r\n", ret, addr);
	if (ret != SWD_OK)
		return ret;

	ret = swd_read(SWD_AP_DRW, &v);
	debug("(%d) tmp    == 0x%08lx\r\n", ret, v);
	if (ret != SWD_OK)
		return ret;

	ret = swd_read(SWD_DP_RDBUFF, &v);
	debug("(%d) drw    == 0x%08lx\r\n", ret, v);
	if (ret != SWD_OK)
		return ret;

	v = (v & ~mask) | enable;
	ret = swd_write(SWD_AP_DRW, v);
	debug("(%d) drw    <- 0x%08lx\r\n", ret, v);
	return ret;
}

int
swd_mem_rmu_read(uint32_t addr, uint32_t mask, uint32_t enable, uint32_t *v)
{
	int ret;

	ret = swd_mem_rmu(addr, mask, enable);
	if (ret != SWD_OK)
		return ret;

	ret = swd_read(SWD_AP_DRW, v);
	debug("(%d) tmp    == 0x%08lx\r\n", ret, *v);
	if (ret != SWD_OK)
		return ret;

	ret = swd_read(SWD_DP_RDBUFF, v);
	debug("(%d) drw    == 0x%08lx\r\n", ret, *v);
	return ret;
}

#define ARMV6M_AIRCR 0xE000ED0CUL

#define ARMV6M_AIRCR_VECTKEY_Pos   16
#define ARMV6M_AIRCR_VECTKEY_Msk   (0xFFFFUL << ARMV6M_AIRCR_VECTKEY_Pos)
#define ARMV6M_AIRCR_VECTKEY       (0x05FAUL << ARMV6M_AIRCR_VECTKEY_Pos)
#define ARMV6M_AIRCR_ENDIANNESS    (0x1UL << 15)
#define ARMV6M_AIRCR_SYSRESETREQ   (0x1UL << 2)
#define ARMV6M_AIRCR_VECTCLRACTIVE (0x1UL << 1)

#define ARMV6M_DFSR  0xE000ED30UL

#define ARMV6M_DHCSR 0xE000EDF0UL
#define ARMV6M_DCRSR 0xE000EDF4UL
#define ARMV6M_DCRDR 0xE000EDF8UL
#define ARMV6M_DEMCR 0xE000EDFFUL

#define ARMV6M_DHCSR_DBGKEY_Pos   16
#define ARMV6M_DHCSR_DBGKEY_Msk   (0xFFFFUL << ARMV6M_DHCSR_DBGKEY_Pos)
#define ARMV6M_DHCSR_DBGKEY       (0xA05FUL << ARMV6M_DHCSR_DBGKEY_Pos)
#define ARMV6M_DHCSR_S_RESET_ST   (0x1UL << 25)
#define ARMV6M_DHCSR_S_RETIRE_ST  (0x1UL << 24)
#define ARMV6M_DHCSR_S_LOCKUP     (0x1UL << 19)
#define ARMV6M_DHCSR_S_SLEEP      (0x1UL << 18)
#define ARMV6M_DHCSR_S_HALT       (0x1UL << 17)
#define ARMV6M_DHCSR_S_REGRDY     (0x1UL << 16)
#define ARMV6M_DHCSR_C_MASKINTS   (0x1UL << 3)
#define ARMV6M_DHCSR_C_STEP       (0x1UL << 2)
#define ARMV6M_DHCSR_C_HALT       (0x1UL << 1)
#define ARMV6M_DHCSR_C_DEBUGEN    (0x1UL << 0)

#define ARMV6M_DEMCR_DWTENA       (0x1UL << 24)
#define ARMV6M_DEMCR_VC_HARDERR   (0x1UL << 10)
#define ARMV6M_DEMCR_VC_CORERESET (0x1UL << 0)

int
swd_armv6m_reset_debug(void)
{
	int ret;
	uint32_t v;

	/* halt core and enter debug mode */
	ret = swd_mem_rmu_read(ARMV6M_DHCSR, ARMV6M_DHCSR_DBGKEY_Msk,
			ARMV6M_DHCSR_DBGKEY
			| ARMV6M_DHCSR_C_HALT
			| ARMV6M_DHCSR_C_DEBUGEN, &v);
	debug("(%d) DHCSR  == 0x%08lx\r\n", ret, v);
	if (ret != SWD_OK)
		return ret;

	/* enter debug mode directly after next reset */
	ret = swd_mem_rmu_read(ARMV6M_DEMCR, 0, ARMV6M_DEMCR_VC_CORERESET, &v);
	debug("(%d) DEMCR  == 0x%08lx\r\n", ret, v);
	if (ret != SWD_OK)
		return ret;

	/* reset core */
	ret = swd_mem_rmu(ARMV6M_AIRCR, ARMV6M_AIRCR_VECTKEY_Msk,
			ARMV6M_AIRCR_VECTKEY | ARMV6M_AIRCR_SYSRESETREQ);
	if (ret != SWD_OK)
		return ret;

	do {
		ret = swd_mem_read(ARMV6M_AIRCR, &v);
		debug("(%d) AIRCR  == 0x%08lx\r\n", ret, v);
		if (ret != SWD_OK)
			return ret;
	} while (v & ARMV6M_AIRCR_SYSRESETREQ);

	ret = swd_mem_read(ARMV6M_DHCSR, &v);
	debug("(%d) DHCSR  == 0x%08lx\r\n", ret, v);
	return ret;
}

int
swd_armv6m_reset_run(void)
{
	int ret;
	uint32_t v;

	/* make sure core runs normally after next reset */
	ret = swd_mem_rmu_read(ARMV6M_DEMCR, ARMV6M_DEMCR_VC_CORERESET, 0, &v);
	debug("(%d) DEMCR  == 0x%08lx\r\n", ret, v);
	if (ret != SWD_OK)
		return ret;

	/* reset core */
	ret = swd_mem_rmu(ARMV6M_AIRCR, ARMV6M_AIRCR_VECTKEY_Msk,
			ARMV6M_AIRCR_VECTKEY | ARMV6M_AIRCR_SYSRESETREQ);
	if (ret != SWD_OK)
		return ret;

	do {
		ret = swd_mem_read(ARMV6M_AIRCR, &v);
		debug("(%d) AIRCR  == 0x%08lx\r\n", ret, v);
		if (ret != SWD_OK)
			return ret;
	} while (v & ARMV6M_AIRCR_SYSRESETREQ);

	ret = swd_mem_read(ARMV6M_DHCSR, &v);
	debug("(%d) DHCSR  == 0x%08lx\r\n", ret, v);
	return ret;
}

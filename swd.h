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

#ifndef _SWD_H
#define _SWD_H

enum swd_return {
	SWD_OK      = 1,
	SWD_WAIT    = 2,
	/* SWD_BROKENACK = 3, */
	SWD_FAULT   = 4,
	/* SWD_BROKENACK = 5, */
	/* SWD_BROKENACK = 6, */
	SWD_NOACK   = 7, /* device doesn't answer, line stays high */
	SWD_PARITY  = 8,
	SWD_TIMEOUT = 9,
};

enum swd_registers {
	SWD_DP_0      = 0x00U, /* 0b00000000 */
	SWD_DP_4      = 0x28U, /* 0b00101000 */
	SWD_DP_8      = 0x30U, /* 0b00110000 */
	SWD_DP_C      = 0x18U, /* 0b00011000 */

	SWD_AP_0      = 0x22U, /* 0b00100010 */
	SWD_AP_4      = 0x0AU, /* 0b00001010 */
	SWD_AP_8      = 0x12U, /* 0b00010010 */
	SWD_AP_C      = 0x3AU, /* 0b00111010 */

	SWD_DP_DPIDR  = SWD_DP_0,
	SWD_DP_ABORT  = SWD_DP_0,
	SWD_DP_STAT   = SWD_DP_4,
	SWD_DP_RESEND = SWD_DP_8,
	SWD_DP_SELECT = SWD_DP_8,
	SWD_DP_RDBUFF = SWD_DP_C,

	/* APBANKSEL = 0x0 */
	SWD_AP_CSW    = SWD_AP_0,
	SWD_AP_TAR    = SWD_AP_4,
	SWD_AP_DRW    = SWD_AP_C,

	/* APBANKSEL = 0xF */
	SWD_AP_CFG    = SWD_AP_4,
	SWD_AP_BASE   = SWD_AP_8,
	SWD_AP_IDR    = SWD_AP_C,
};

#define SWD_DP_DPIDR_REVISION_Pos 28
#define SWD_DP_DPIDR_REVISION_Msk (0xFU << SWD_DP_DPIDR_REVISION_Pos)
#define SWD_DP_DPIDR_PARTNO_Pos   20
#define SWD_DP_DPIDR_PARTNO_Msk   (0xFFU << SWD_DP_DPIDR_PARTNO_Pos)
#define SWD_DP_DPIDR_MIN          (1U << 16)
#define SWD_DP_DPIDR_VERSION_Pos  12
#define SWD_DP_DPIDR_VERSION_Msk  (0xFU << SWD_DP_DPIDR_VERSION_Pos)
#define SWD_DP_DPIDR_VERSION_DPv1 (0x1U << SWD_DP_DPIDR_VERSION_Pos)
#define SWD_DP_DPIDR_VERSION_DPv2 (0x2U << SWD_DP_DPIDR_VERSION_Pos)
#define SWD_DP_DPIDR_DESIGNER_Pos 0
#define SWD_DP_DPIDR_DESIGNER_Msk (0xFFFU << SWD_DP_DPIDR_DESIGNER_Pos)

#define SWD_DP_ABORT_ORUNERRCLR (1U << 4)
#define SWD_DP_ABORT_WDERRCLR   (1U << 3)
#define SWD_DP_ABORT_STKERRCLR  (1U << 2)
#define SWD_DP_ABORT_STKCMPCLR  (1U << 1)
#define SWD_DP_ABORT_DAPABORT   (1U << 0)

#define SWD_DP_STAT_CSYSPWRUPACK (1U << 31)
#define SWD_DP_STAT_CSYSPWRUPREQ (1U << 30)
#define SWD_DP_STAT_CDBGPWRUPACK (1U << 29)
#define SWD_DP_STAT_CDBGPWRUPREQ (1U << 28)
#define SWD_DP_STAT_CDBGRSTACK   (1U << 27)
#define SWD_DP_STAT_CDBGRSTREQ   (1U << 26)
#define SWD_DP_STAT_TRNCNT_Pos   12
#define SWD_DP_STAT_TRNCNT_Msk   (0xFFFU << SWD_DP_STAT_TRNCNT_Pos)
#define SWD_DP_STAT_MASKLANE_Pos 8
#define SWD_DP_STAT_MASKLANE_Msk (0xFU << SWD_DP_STAT_MASKLANE_Pos)
#define SWD_DP_STAT_WDATAERR     (1U << 7)
#define SWD_DP_STAT_READOK       (1U << 6)
#define SWD_DP_STAT_STICKYERR    (1U << 5)
#define SWD_DP_STAT_STICKYCMP    (1U << 4)
#define SWD_DP_STAT_TRNMODE_Pos  2
#define SWD_DP_STAT_TRNMODE_Msk  (0x3U << SWD_DP_STAT_TRNMODE_Pos)
#define SWD_DP_STAT_STICKYORUN   (1U << 1)
#define SWD_DP_STAT_ORUNDETECT   (1U << 0)
#define SWD_DP_STAT_WRITEZERO_Msk \
	( SWD_DP_STAT_WDATAERR \
	| SWD_DP_STAT_READOK \
	| SWD_DP_STAT_STICKYERR \
	| SWD_DP_STAT_STICKYCMP \
	| SWD_DP_STAT_STICKYORUN )

#define SWD_DP_SELECT_APSEL_Pos     24
#define SWD_DP_SELECT_APSEL_Msk     (0xFFU << SWD_DP_SELECT_APSEL_Pos)
#define SWD_DP_SELECT_APBANKSEL_Pos 4
#define SWD_DP_SELECT_APBANKSEL_Msk (0xFU << SWD_DP_SELECT_APBANKSEL_Pos)
#define SWD_DP_SELECT_DPBANKSEL_Pos 0
#define SWD_DP_SELECT_DPBANKSEL_Msk (0xFU << SWD_DP_SELECT_DPBANKSEL_Pos)

#define SWD_AP_CSW_DBGSWENABLE      (1U << 31)
#define SWD_AP_CSW_PROT_Pos         24
#define SWD_AP_CSW_PROT_Msk         (0x7FU << SWD_AP_CSW_PROT_Pos)
#define SWD_AP_CSW_SPIDEN           (1U << 23)
#define SWD_AP_CSW_TYPE_Pos         12
#define SWD_AP_CSW_TYPE_Msk         (0xFU << SWD_AP_CSW_TYPE_Pos)
#define SWD_AP_CSW_MODE_Pos         8
#define SWD_AP_CSW_MODE_Msk         (0xFU << SWD_AP_CSW_MODE_Pos)
#define SWD_AP_CSW_TRINPROG         (1U << 7)
#define SWD_AP_CSW_DEVICEEN         (1U << 6)
#define SWD_AP_CSW_ADDRINC_Pos      4
#define SWD_AP_CSW_ADDRINC_Msk      (0x3U << SWD_AP_CSW_ADDRINC_Pos)
#define SWD_AP_CSW_ADDRINC_OFF      (0x0U << SWD_AP_CSW_ADDRINC_Pos)
#define SWD_AP_CSW_ADDRINC_SINGLE   (0x1U << SWD_AP_CSW_ADDRINC_Pos)
#define SWD_AP_CSW_ADDRINC_PACKED   (0x2U << SWD_AP_CSW_ADDRINC_Pos)
#define SWD_AP_CSW_SIZE_Pos         0
#define SWD_AP_CSW_SIZE_Msk         (0x3U << SWD_AP_CSW_SIZE_Pos)
#define SWD_AP_CSW_SIZE_8BIT        (0x0U << SWD_AP_CSW_SIZE_Pos)
#define SWD_AP_CSW_SIZE_16BIT       (0x1U << SWD_AP_CSW_SIZE_Pos)
#define SWD_AP_CSW_SIZE_32BIT       (0x2U << SWD_AP_CSW_SIZE_Pos)
#define SWD_AP_CSW_WRITEZERO_Msk \
	( SWD_AP_CSW_TRINPROG \
	| SWD_AP_CSW_DEVICEEN )


extern uint32_t swd_parity(uint32_t v);

/* low-level functions */
extern void swd_init(void);
extern void swd_engage(void);
extern void swd_disengage(void);
extern int swd_reset(uint32_t *dpid);
extern int swd_read(unsigned int addr, uint32_t *v);
extern int swd_write(unsigned int addr, uint32_t v);

/* higher-level functions using the above */
extern int swd_dp_init(uint32_t *idr);
extern int swd_mem_read(uint32_t addr, uint32_t *v);
extern int swd_mem_write(uint32_t addr, uint32_t v);
extern int swd_mem_rmu(uint32_t addr, uint32_t mask, uint32_t enable);
extern int swd_mem_rmu_read(uint32_t addr, uint32_t mask, uint32_t enable, uint32_t *v);

extern int swd_armv6m_reset_debug(void);
extern int swd_armv6m_reset_run(void);

#endif

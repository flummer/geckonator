#ifndef _GECKONATOR_EMU_H
#define _GECKONATOR_EMU_H

#include "common.h"

/* EMU_CTRL */
static inline void
emu_em2_block(void)               { EMU->CTRL |= EMU_CTRL_EM2BLOCK; }
static inline void
emu_em2_unblock(void)             { EMU->CTRL &= ~EMU_CTRL_EM2BLOCK; }
static inline uint32_t
emu_em2_blocked(void)             { return EMU->CTRL & EMU_CTRL_EM2BLOCK; }
static inline void
emu_vreg_reduced(void)            { EMU->CTRL |= EMU_CTRL_EMVREG; }
static inline void
emu_vreg_full(void)               { EMU->CTRL &= ~EMU_CTRL_EMVREG; }

/* EMU_LOCK */
static inline void
emu_lock(void)                    { EMU->LOCK = 0; }
static inline void
emu_unlock(void)                  { EMU->LOCK = 0xADE8U; }
static inline uint32_t
emu_locked(void)                  { return EMU->LOCK; }

/* EMU_AUXCTRL */
extern void emu_reset_cause_clear(void);

extern void __noreturn emu_em4_enter(void);

#endif

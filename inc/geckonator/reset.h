#ifndef _GECKONATOR_RESET_H
#define _GECKONATOR_RESET_H

#include "common.h"

/* RMU_CTRL */
static inline void
reset_lockup_disable(void) { RMU->CTRL = 0; }
static inline void
reset_lockup_enable(void)  { RMU->CTRL = RMU_CTRL_LOCKUPRDIS; }


/* RMU_RSTCAUSE */
static inline uint32_t
reset_cause(void)          { return RMU->RSTCAUSE; }

/* RMU_CMD */
static inline void
reset_cause_clear(void)    { RMU->CMD = RMU_CMD_RCCLR; }

extern void reset_cause_clear_all(void);

#endif

#ifndef _GECKONATOR_PRS_H
#define _GECKONATOR_PRS_H

#include "common.h"

/* PRS_SWPULSE */
static inline void
prs_channel_pulse(unsigned int i)
{
	PRS->SWPULSE = 1 << i;
}

/* PRS_SWLEVEL */
static inline uint32_t
prs_channel_level(unsigned int i)
{
	return PRS->SWLEVEL & (1 << i);
}
static inline void
prs_channel_level_high(unsigned int i)
{
	PRS->SWLEVEL |= 1 << i;
}
static inline void
prs_channel_level_low(unsigned int i)
{
	PRS->SWLEVEL &= ~(1 << i);
}

/* PRS_ROUTE */
static inline void
prs_pins(uint32_t v)
{
	PRS->ROUTE = v;
}

/* PRS_CHx_CTRL */
enum prs_channel_config {
	PRS_ASYNC         = PRS_CH_CTRL_ASYNC,

	PRS_EDGE_OFF      = PRS_CH_CTRL_EDSEL_OFF,
	PRS_EDGE_POSITIVE = PRS_CH_CTRL_EDSEL_POSEDGE,
	PRS_EDGE_NEGATIVE = PRS_CH_CTRL_EDSEL_NEGEDGE,
	PRS_EDGE_BOTH     = PRS_CH_CTRL_EDSEL_BOTHEDGES,

	PRS_SOURCE_NONE   = PRS_CH_CTRL_SOURCESEL_NONE,

	/* vcmp */
	PRS_SOURCE_VCMP_OUT  = PRS_CH_CTRL_SOURCESEL_VCMP
		| PRS_CH_CTRL_SIGSEL_VCMPOUT,
	/* timer1 */
	PRS_SOURCE_TIMER1_UF  = PRS_CH_CTRL_SOURCESEL_TIMER1
		| PRS_CH_CTRL_SIGSEL_TIMER1UF,
	PRS_SOURCE_TIMER1_OF  = PRS_CH_CTRL_SOURCESEL_TIMER1
		| PRS_CH_CTRL_SIGSEL_TIMER1OF,
	PRS_SOURCE_TIMER1_CC0 = PRS_CH_CTRL_SOURCESEL_TIMER1
		| PRS_CH_CTRL_SIGSEL_TIMER1CC0,
	PRS_SOURCE_TIMER1_CC1 = PRS_CH_CTRL_SOURCESEL_TIMER1
		| PRS_CH_CTRL_SIGSEL_TIMER1CC1,
	PRS_SOURCE_TIMER1_CC2 = PRS_CH_CTRL_SOURCESEL_TIMER1
		| PRS_CH_CTRL_SIGSEL_TIMER1CC2,
	/* usb */
	PRS_SOURCE_USB_SOF   = PRS_CH_CTRL_SOURCESEL_USB
		| PRS_CH_CTRL_SIGSEL_USBSOF,
	PRS_SOURCE_USB_SOFSR = PRS_CH_CTRL_SOURCESEL_USB
		| PRS_CH_CTRL_SIGSEL_USBSOFSR,
};
static inline void
prs_channel_config(unsigned int i, uint32_t v)
{
	PRS->CH[i].CTRL = v;
}

/* PRS_TRACECTRL */

#endif

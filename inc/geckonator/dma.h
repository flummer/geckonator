#ifndef _GECKONATOR_DMA_H
#define _GECKONATOR_DMA_H

#include "common.h"

struct dma_descriptor {
	volatile void *volatile src_end;
	volatile void *volatile dst_end;
	volatile uint32_t control;
	volatile uint32_t user;
};

struct dma_channel_control {
	struct dma_descriptor primary[DMA_CHAN_COUNT];
#if 4*DMA_CHAN_COUNT < 32
	uint32_t unused[32 - 4*DMA_CHAN_COUNT];
#endif
	struct dma_descriptor alternate[DMA_CHAN_COUNT];
};

#define DMA_DESCRIPTORS(name) struct dma_channel_control name __attribute__((section(".dma")))

/* DMA_STATUS */
static inline uint32_t
dma_channels(void)
{
	return  (DMA->STATUS >> _DMA_STATUS_CHNUM_SHIFT) + 1;
}
static inline unsigned int
dma_state(void)
{
	return (DMA->STATUS & _DMA_STATUS_STATE_MASK) >> _DMA_STATUS_STATE_SHIFT;
}
static inline uint32_t
dma_enabled(void)
{
	return DMA->STATUS & DMA_STATUS_EN;
}

/* DMA_CONFIG */
static inline void
dma_disable(void)
{
	DMA->CONFIG = 0;
}
static inline void
dma_enable(void)
{
	DMA->CONFIG = DMA_CONFIG_EN;
}
static inline void
dma_enable_privileged(void)
{
	DMA->CONFIG = DMA_CONFIG_CHPROT | DMA_CONFIG_EN;
}

/* DMA_CTRLBASE */
static inline struct dma_descriptor *
dma_base(void)
{
	return (struct dma_descriptor *)DMA->CTRLBASE;
}
static inline void
dma_base_set(struct dma_channel_control *addr)
{
	DMA->CTRLBASE = (uint32_t)addr;
}

/* DMA_ALTCTRLBASE */
static inline struct dma_descriptor *
dma_altbase(void)
{
	return (struct dma_descriptor *)DMA->ALTCTRLBASE;
}

/* DMA_CHWAITSTATUS */
static inline uint32_t
dma_waitiing(void)
{
	return DMA->CHWAITSTATUS;
}
static inline uint32_t
dma_channel_waiting(unsigned int i)
{
	return DMA->CHWAITSTATUS & (1 << i);
}

/* DMA_CHSWREQ */
static inline void
dma_channel_request(unsigned int i)
{
	DMA->CHSWREQ = 1 << i;
}

/* DMA_CHUSEBURSTS */
static inline uint32_t
dma_channel_useburst(unsigned int i)
{
	return DMA->CHUSEBURSTS & (1 << i);
}
static inline void
dma_channel_useburst_enable(unsigned int i)
{
	DMA->CHUSEBURSTS = (1 << i);
}


/* DMA_CHUSEBURSTC */
static inline void
dma_channel_useburst_disable(unsigned int i)
{
	DMA->CHUSEBURSTC = (1 << i);
}

/* DMA_CHREQMASKS */
static inline uint32_t
dma_channel_masked(unsigned int i)
{
	return DMA->CHREQMASKS & (1 << i);
}
static inline void
dma_channel_mask_enable(unsigned int i)
{
	DMA->CHREQMASKS = (1 << i);
}

/* DMA_CHREQMASKC */
static inline void
dma_channel_mask_disable_all(void)
{
	DMA->CHREQMASKC = (1 << DMA_CHAN_COUNT) - 1;
}
static inline void
dma_channel_mask_disable(unsigned int i)
{
	DMA->CHREQMASKC = (1 << i);
}

/* DMA_CHENS */
static inline uint32_t
dma_channel_enabled(unsigned int i)
{
	return DMA->CHENS & (1 << i);
}
static inline void
dma_channel_enable(unsigned int i)
{
	DMA->CHENS = (1 << i);
}

/* DMA_CHENC */
static inline void
dma_channel_disable_all(void)
{
	DMA->CHENC = (1 << DMA_CHAN_COUNT) - 1;
}
static inline void
dma_channel_disable(unsigned int i)
{
	DMA->CHENC = (1 << i);
}

/* DMA_CHALTS */
static inline uint32_t
dma_channel_alternate(unsigned int i)
{
	return DMA->CHALTS & (1 << i);
}
static inline void
dma_channel_alternate_enable(unsigned int i)
{
	DMA->CHALTS = (1 << i);
}

/* DMA_CHALTC */
static inline void
dma_channel_alternate_disable(unsigned int i)
{
	DMA->CHALTC = (1 << i);
}

/* DMA_CHPRIS */
static inline uint32_t
dma_channel_prioritized(unsigned int i)
{
	return DMA->CHPRIS & (1 << i);
}
static inline void
dma_channel_priority_high(unsigned int i)
{
	DMA->CHPRIS = (1 << i);
}

/* DMA_CHPRIC */
static inline void
dma_channel_priority_low(unsigned int i)
{
	DMA->CHPRIC = (1 << i);
}

/* DMA_ERRORC */
static inline uint32_t
dma_bus_error(void)
{
	return DMA->ERRORC;
}
static inline void
dma_bus_error_clear(void)
{
	DMA->ERRORC = 0;
}

/* DMA_CHREQSTATUS */
static inline uint32_t
dma_channel_requested(unsigned int i)
{
	return DMA->CHREQSTATUS & (1 << i);
}

/* DMA_CHSREQSTATUS */
static inline uint32_t
dma_channel_requested_single(unsigned int i)
{
	return DMA->CHSREQSTATUS & (1 << i);
}

/* DMA_IF */
static inline uint32_t
dma_flags(void)                              { return DMA->IF; }
static inline uint32_t
dma_flag_error(uint32_t v)                   { return (int32_t)v < 0; }
static inline uint32_t
dma_flag_done(unsigned int i, uint32_t v)    { return v & (1 << i); }

/* DMA_IFS */
static inline void
dma_flag_error_set(void)                     { DMA->IFS = DMA_IFS_ERR; }
static inline void
dma_flag_done_set(unsigned int i)            { DMA->IFS = 1 << i; }

/* DMA_IFC */
static inline void
dma_flags_clear(uint32_t v)                  { DMA->IFC = v; }
static inline void
dma_flag_error_clear(void)                   { DMA->IFC = DMA_IFC_ERR; }
static inline void
dma_flag_done_clear(unsigned int i)          { DMA->IFC = 1 << i; }

/* DMA_IEN */
static inline void
dma_flag_error_disable(void)                 { DMA->IEN &= ~DMA_IEN_ERR; }
static inline void
dma_flag_error_enable(void)                  { DMA->IEN |= DMA_IEN_ERR; }
static inline void
dma_flag_done_disable(unsigned int i)        { DMA->IEN &= ~(1 << i); }
static inline void
dma_flag_done_enable(unsigned int i)         { DMA->IEN |= 1 << i; }

/* DMA_CHx_CTRL */
static inline void
dma_channel_config(unsigned int i, uint32_t v)
{
	DMA->CH[i].CTRL = v;
}

#endif

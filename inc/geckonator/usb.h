#ifndef _GECKONATOR_USB_H
#define _GECKONATOR_USB_H

#include "common.h"

/* USB_CTRL */
static inline void
usb_mode(uint32_t v)                     { USB->CTRL = v; }
static inline void
usb_low_energy_disable(void)             { USB->CTRL &= ~USB_CTRL_LEMIDLEEN; }
static inline void
usb_low_energy_enable(void)              { USB->CTRL |= USB_CTRL_LEMIDLEEN; }

/* USB_STATUS */
static inline uint32_t
usb_status(void)                         { return USB->STATUS; }
static inline uint32_t
usb_low_energy_active(void)              { return USB->STATUS & USB_STATUS_LEMACTIVE; }
static inline uint32_t
usb_vreg_sense(void)                     { return USB->STATUS & USB_STATUS_VREGOS; }

/* USB_IF */
static inline uint32_t
usb_vreg_flags(void)                     { return USB->IF; }
static inline uint32_t
usb_vreg_flag_low(uint32_t v)            { return v & USB_IF_VREGOSL; }
static inline uint32_t
usb_vreg_flag_high(uint32_t v)           { return v & USB_IF_VREGOSH; }

/* USB_IFS */
static inline void
usb_vreg_flags_set(uint32_t v)           { USB->IFS = v; }
static inline void
usb_vreg_flag_low_set(void)              { USB->IFS = USB_IFS_VREGOSL; }
static inline void
usb_vreg_flag_high_set(void)             { USB->IFS = USB_IFS_VREGOSL; }

/* USB_IFC */
static inline void
usb_vreg_flags_clear(uint32_t v)         { USB->IFC = v; }

/* USB_IEN */
static inline void
usb_vreg_flags_disable(void)             { USB->IEN = 0UL; }
static inline void
usb_vreg_flags_enable(void)              { USB->IEN = USB_IEN_VREGOSL | USB_IEN_VREGOSH; }
static inline void
usb_vreg_flag_low_disable(void)          { USB->IEN &= ~USB_IEN_VREGOSL; }
static inline void
usb_vreg_flag_low_enable(void)           { USB->IEN |= USB_IEN_VREGOSL; }
static inline void
usb_vreg_flag_high_disable(void)         { USB->IEN &= ~USB_IEN_VREGOSL; }
static inline void
usb_vreg_flag_high_enable(void)          { USB->IEN |= USB_IEN_VREGOSL; }

/* USB_ROUTE */
static inline void
usb_pins_disable(void)                   { USB->ROUTE = 0UL; }
static inline void
usb_pins_enable(void)                    { USB->ROUTE = USB_ROUTE_PHYPEN; }
static inline void
usb_pins_enable_pullup(void)             { USB->ROUTE = USB_ROUTE_DMPUPEN | USB_ROUTE_PHYPEN; }

/* USB_GAHBCFG */
static inline void
usb_ahb_config(uint32_t v)               { USB->GAHBCFG = v; }

/* USB_GUSBCFG */
static inline void
usb_global_config(uint32_t v)            { USB->GUSBCFG = v; }

/* USB_GRSTCTL */
static inline uint32_t
usb_ahb_idle(void)                       { return USB->GRSTCTL & USB_GRSTCTL_AHBIDLE; }
static inline void
usb_fifo_flush(void)
{
	USB->GRSTCTL = USB_GRSTCTL_TXFNUM_FALL
		| USB_GRSTCTL_TXFFLSH
		| USB_GRSTCTL_RXFFLSH;
}
static inline uint32_t
usb_fifo_flushing(void)
{
	return USB->GRSTCTL & (USB_GRSTCTL_TXFFLSH | USB_GRSTCTL_RXFFLSH);
}
static inline void
usb_fifo_tx_flush(unsigned int i)
{
	USB->GRSTCTL = (i << _USB_GRSTCTL_TXFNUM_SHIFT) | USB_GRSTCTL_TXFFLSH;
}
static inline uint32_t
usb_fifo_tx_flushing(void)               { return USB->GRSTCTL & USB_GRSTCTL_TXFFLSH; }
static inline void
usb_fifo_rx_flush(void)                  { USB->GRSTCTL = USB_GRSTCTL_RXFFLSH; }
static inline uint32_t
usb_fifo_rx_flushing(void)               { return USB->GRSTCTL & USB_GRSTCTL_RXFFLSH; }
static inline void
usb_core_reset(void)                     { USB->GRSTCTL = USB_GRSTCTL_CSFTRST; }
static inline uint32_t
usb_core_resetting(void)                 { return USB->GRSTCTL & USB_GRSTCTL_CSFTRST; }

/* USB_GINTSTS */
static inline uint32_t
usb_flags(void)                          { return USB->GINTSTS; }
static inline void
usb_flags_clear(uint32_t v)
{
	USB->GINTSTS = v &
		( USB_GINTSTS_WKUPINT
		| USB_GINTSTS_RESETDET
		| USB_GINTSTS_FETSUSP
		| USB_GINTSTS_INCOMPLP
		| USB_GINTSTS_INCOMPISOIN
		| USB_GINTSTS_ISOOUTDROP
		| USB_GINTSTS_ENUMDONE
		| USB_GINTSTS_USBRST
		| USB_GINTSTS_USBSUSP
		| USB_GINTSTS_ERLYSUSP
		| USB_GINTSTS_SOF);
}
static inline uint32_t
usb_flag_wakeup(uint32_t v)              { return v & USB_GINTSTS_WKUPINT; }
static inline uint32_t
usb_flag_reset_detected(uint32_t v)      { return v & USB_GINTSTS_RESETDET; }
static inline uint32_t
usb_flag_fetch_suspended(uint32_t v)     { return v & USB_GINTSTS_FETSUSP; }
static inline uint32_t
usb_flag_per_incomplete(uint32_t v)      { return v & USB_GINTSTS_INCOMPLP; }
static inline uint32_t
usb_flag_iso_incomplete(uint32_t v)      { return v & USB_GINTSTS_INCOMPISOIN; }
static inline uint32_t
usb_flag_ep_out(uint32_t v)              { return v & USB_GINTSTS_OEPINT; }
static inline uint32_t
usb_flag_ep_in(uint32_t v)               { return v & USB_GINTSTS_IEPINT; }
static inline uint32_t
usb_flag_ep(uint32_t v)                  { return v & (USB_GINTSTS_OEPINT | USB_GINTSTS_IEPINT); }
static inline uint32_t
usb_flag_iso_dropped(uint32_t v)         { return v & USB_GINTSTS_ISOOUTDROP; }
static inline uint32_t
usb_flag_enumdone(uint32_t v)            { return v & USB_GINTSTS_ENUMDONE; }
static inline uint32_t
usb_flag_reset(uint32_t v)               { return v & USB_GINTSTS_USBRST; }
static inline uint32_t
usb_flag_early_suspend(uint32_t v)       { return v & USB_GINTSTS_ERLYSUSP; }
static inline uint32_t
usb_flag_suspend(uint32_t v)             { return v & USB_GINTSTS_USBSUSP; }
static inline uint32_t
usb_flag_nak_out_effective(uint32_t v)   { return v & USB_GINTSTS_GOUTNAKEFF; }
static inline uint32_t
usb_flag_nak_in_effective(uint32_t v)    { return v & USB_GINTSTS_GINNAKEFF; }
static inline uint32_t
usb_flag_rxdata(uint32_t v)              { return v & USB_GINTSTS_RXFLVL; }
static inline uint32_t
usb_flag_sof(uint32_t v)                 { return v & USB_GINTSTS_SOF; }

/* USB_GINTMSK */
static inline void
usb_flags_enable(uint32_t v)             { USB->GINTMSK = v; }
static inline void
usb_flag_sof_disable(void)               { USB->GINTMSK &= ~USB_GINTMSK_SOFMSK; }
static inline void
usb_flag_sof_enable(void)                { USB->GINTMSK |= USB_GINTMSK_SOFMSK; }

/* USB_GRXSTSR */

/* USB_GRXSTSP */

/* USB_GRXFSIZ */

/* USB_GDFIFOCFG */

/* USB_GNPTXFSIZ */
/* USB_DIEPTXF1 */
/* USB_DIEPTXF2 */
/* USB_DIEPTXF3 */
static inline void
usb_allocate_buffers(uint32_t rx,
		uint32_t tx0,
		uint32_t tx1,
		uint32_t tx2,
		uint32_t tx3)
{
	/* round up to number of 32bit words */
	rx  = (rx  + 3) >> 2;
	tx0 = (tx0 + 3) >> 2;
	tx1 = (tx1 + 3) >> 2;
	tx2 = (tx2 + 3) >> 2;
	tx3 = (tx3 + 3) >> 2;
	USB->GRXFSIZ   = rx;
	USB->GNPTXFSIZ = (tx0 << 16) | (rx);
	USB->DIEPTXF1  = (tx1 << 16) | (rx + tx0);
	USB->DIEPTXF2  = (tx2 << 16) | (rx + tx0 + tx1);
	USB->DIEPTXF3  = (tx3 << 16) | (rx + tx0 + tx1 + tx2);
}

/* USB_DCFG */
static inline void
usb_device_config(uint32_t v)
{
	USB->DCFG = v;
}
static inline void
usb_set_address(uint32_t addr)
{
	USB->DCFG = (USB->DCFG & ~_USB_DCFG_DEVADDR_MASK)
	          | (addr << _USB_DCFG_DEVADDR_SHIFT);
}

/* USB_DCTL */
static inline void
usb_device_control(uint32_t v)           { USB->DCTL = v; }
static inline void
usb_global_out_nak_clear(void)           { USB->DCTL |= USB_DCTL_CGOUTNAK; }
static inline void
usb_global_out_nak_set(void)             { USB->DCTL |= USB_DCTL_SGOUTNAK; }
static inline void
usb_global_in_nak_clear(void)            { USB->DCTL |= USB_DCTL_CGNPINNAK; }
static inline void
usb_global_in_nak_set(void)              { USB->DCTL |= USB_DCTL_SGNPINNAK; }

#define USB_DCTL_WO_BITMASK (USB_DCTL_CGOUTNAK | USB_DCTL_SGOUTNAK | USB_DCTL_CGNPINNAK | USB_DCTL_SGNPINNAK)
static inline void
usb_connect(void)
{
	USB->DCTL &= ~(USB_DCTL_WO_BITMASK | USB_DCTL_SFTDISCON);
}
static inline void
usb_disconnect(void)
{
	USB->DCTL = (USB->DCTL & ~USB_DCTL_WO_BITMASK) | USB_DCTL_SFTDISCON;
}
static inline void
usb_remote_wakeup_clear(void)
{
	USB->DCTL &= ~(USB_DCTL_WO_BITMASK | USB_DCTL_RMTWKUPSIG);
}
static inline void
usb_remote_wakeup_set(void)
{
	USB->DCTL = (USB->DCTL & ~(USB_DCTL_WO_BITMASK | USB_DCTL_SFTDISCON))
	          | USB_DCTL_RMTWKUPSIG;
}

/* USB_DSTS */
static inline uint32_t
usb_device_status(void)
{
	return USB->DSTS;
}
static inline uint32_t
usb_enumerated_speed(void)
{
	return (USB->DSTS & _USB_DSTS_ENUMSPD_MASK) >> _USB_DSTS_ENUMSPD_SHIFT;
}
static inline bool
usb_speed_low(void)
{
	return (USB->DSTS & _USB_DSTS_ENUMSPD_MASK) == (2UL << _USB_DSTS_ENUMSPD_SHIFT);
}
static inline bool
usb_speed_full(void)
{
	return (USB->DSTS & _USB_DSTS_ENUMSPD_MASK) == (3UL << _USB_DSTS_ENUMSPD_SHIFT);
}
static inline uint32_t
usb_frame_odd(uint32_t v)
{
	return v & 0x100UL;
}
static inline uint32_t
usb_frame_number(uint32_t v)
{
	return (v >> 8) & 0x3FFFUL;
}

/* USB_DIEPMSK */
static inline void
usb_ep_in_flags_enable(uint32_t v)       { USB->DIEPMSK = v; }
static inline void
usb_ep_in_flag_nak_disable(void)         { USB->DIEPMSK &= ~USB_DIEPMSK_NAKMSK; }
static inline void
usb_ep_in_flag_nak_enable(void)          { USB->DIEPMSK |= USB_DIEPMSK_NAKMSK; }
static inline void
usb_ep_in_flag_disabled_disable(void)    { USB->DIEPMSK &= ~USB_DIEPMSK_EPDISBLDMSK; }
static inline void
usb_ep_in_flag_disabled_enable(void)     { USB->DIEPMSK |= USB_DIEPMSK_EPDISBLDMSK; }

/* USB_DOEPMSK */
static inline void
usb_ep_out_flags_enable(uint32_t v)      { USB->DOEPMSK = v; }
static inline void
usb_ep_out_flag_nak_disable(void)        { USB->DOEPMSK &= ~USB_DOEPMSK_NAKMSK; }
static inline void
usb_ep_out_flag_nak_enable(void)         { USB->DOEPMSK |= USB_DOEPMSK_NAKMSK; }
static inline void
usb_ep_out_flag_disabled_disable(void)   { USB->DOEPMSK &= ~USB_DOEPMSK_EPDISBLDMSK; }
static inline void
usb_ep_out_flag_disabled_enable(void)    { USB->DOEPMSK |= USB_DOEPMSK_EPDISBLDMSK; }

/* USB_DAINT */
static inline uint32_t
usb_ep_flags(void)                       { return USB->DAINT; }
static inline uint32_t
usb_ep_flag_in(unsigned int i, uint32_t v)
{
	return v & (0x00000001UL << i);
}
static inline uint32_t
usb_ep_flag_out(unsigned int i, uint32_t v)
{
	return v & (0x00010000UL << i);
}
static inline uint32_t
usb_ep_flag_in_or_out(unsigned int i, uint32_t v)
{
	return v & (0x00010001UL << i);
}

/* USB_DAINTMSX */
static inline void
usb_ep_flags_enable(uint32_t v)
{
	USB->DAINTMSK = v;
}
static inline void
usb_ep_flag_out_disable(unsigned int i)
{
	USB->DAINTMSK &= ~(USB_DAINTMSK_OUTEPMSK0 << i);
}
static inline void
usb_ep_flag_out_enable(unsigned int i)
{
	USB->DAINTMSK |= USB_DAINTMSK_OUTEPMSK0 << i;
}
static inline void
usb_ep_flag_in_disable(unsigned int i)
{
	USB->DAINTMSK &= ~(USB_DAINTMSK_INEPMSK0 << i);
}
static inline void
usb_ep_flag_in_enable(unsigned int i)
{
	USB->DAINTMSK |= USB_DAINTMSK_INEPMSK0 << i;
}
static inline void
usb_ep_flag_inout_disable(unsigned int i)
{
	USB->DAINTMSK &= ~((USB_DAINTMSK_OUTEPMSK0 | USB_DAINTMSK_INEPMSK0) << i);
}
static inline void
usb_ep_flag_inout_enable(unsigned int i)
{
	USB->DAINTMSK |= (USB_DAINTMSK_OUTEPMSK0 | USB_DAINTMSK_INEPMSK0) << i;
}

/* USB_DIEPEMPMSK */

/* USB_DIEP0CTL */
static inline void
usb_ep0in_config_64byte(void)
{
	USB->DIEP0CTL = (0 << _USB_DIEP0CTL_TXFNUM_SHIFT)
		| USB_DIEP0CTL_MPS_64B;
}
static inline void
usb_ep0in_config_64byte_stall(void)
{
	USB->DIEP0CTL = USB_DIEP0CTL_STALL
		| (0 << _USB_DIEP0CTL_TXFNUM_SHIFT)
		| USB_DIEP0CTL_MPS_64B;
}
static inline void
usb_ep0in_config_8byte(void)
{
	USB->DIEP0CTL = (0 << _USB_DIEP0CTL_TXFNUM_SHIFT)
		| USB_DIEP0CTL_MPS_8B;
}
static inline void
usb_ep0in_enable(void)
{
	USB->DIEP0CTL |= USB_DIEP0CTL_EPENA | USB_DIEP0CTL_CNAK;
}
static inline void
usb_ep0in_stall(void)
{
	USB->DIEP0CTL |= USB_DIEP0CTL_STALL;
}

/* USB_DIEP0INT */
static inline uint32_t
usb_ep0in_flags(void)                    { return USB->DIEP0INT; }
static inline void
usb_ep0in_flags_clear(uint32_t v)
{
	USB->DIEP0INT = v &
		( USB_DIEP0INT_NAKINTRPT
		| USB_DIEP0INT_BBLEERR
		| USB_DIEP0INT_PKTDRPSTS
		| USB_DIEP0INT_INEPNAKEFF
		| USB_DIEP0INT_INTKNTXFEMP
		| USB_DIEP0INT_TIMEOUT
		| USB_DIEP0INT_AHBERR
		| USB_DIEP0INT_EPDISBLD
		| USB_DIEP0INT_XFERCOMPL);
}
static inline uint32_t
usb_ep0in_flag_nak(uint32_t v)           { return v & USB_DIEP0INT_NAKINTRPT; }
static inline uint32_t
usb_ep0in_flag_babble(uint32_t v)        { return v & USB_DIEP0INT_BBLEERR; }
static inline uint32_t
usb_ep0in_flag_packet_drop(uint32_t v)   { return v & USB_DIEP0INT_PKTDRPSTS; }
static inline uint32_t
usb_ep0in_flag_nak_effective(uint32_t v) { return v & USB_DIEP0INT_INEPNAKEFF; }
static inline uint32_t
usb_ep0in_flag_in_empty(uint32_t v)      { return v & USB_DIEP0INT_INTKNTXFEMP; }
static inline uint32_t
usb_ep0in_flag_timeout(uint32_t v)       { return v & USB_DIEP0INT_TIMEOUT; }
static inline uint32_t
usb_ep0in_flag_ahb_error(uint32_t v)     { return v & USB_DIEP0INT_AHBERR; }
static inline uint32_t
usb_ep0in_flag_disabled(uint32_t v)      { return v & USB_DIEP0INT_EPDISBLD; }
static inline uint32_t
usb_ep0in_flag_complete(uint32_t v)      { return v & USB_DIEP0INT_XFERCOMPL; }

/* USB_DIEP0TSIZ */
static inline void
usb_ep0in_transfer_size(uint32_t packets, uint32_t size)
{
	USB->DIEP0TSIZ = (packets << _USB_DIEP0TSIZ_PKTCNT_SHIFT) | size;
}
static inline uint32_t
usb_ep0in_bytes_left(void)
{
	return USB->DIEP0TSIZ & _USB_DIEP0TSIZ_XFERSIZE_MASK;
}

/* USB_DIEP0DMAADDR */
static inline const void *
usb_ep0in_dma_address(void)
{
	return (const void *)USB->DIEP0DMAADDR;
}
static inline void
usb_ep0in_dma_address_set(const void *addr)
{
	USB->DIEP0DMAADDR = (uint32_t)addr;
}

/* USB_DIEP0TXFSTS */

/* USB_DIEPx_CTL */
static inline uint32_t
usb_ep_in_enabled(unsigned int i)
{
	return USB->DIEP[i-1].CTL & USB_DIEP_CTL_EPENA;
}
static inline uint32_t
usb_ep_in_active(unsigned int i)
{
	return USB->DIEP[i-1].CTL & USB_DIEP_CTL_USBACTEP;
}
static inline void
usb_ep_in_config_disabled(unsigned int i)
{
	USB->DIEP[i-1].CTL = 0;
}
static inline void
usb_ep_in_config_disabled_nak(unsigned int i)
{
	USB->DIEP[i-1].CTL = USB_DIEP_CTL_SNAK;
}
static inline void
usb_ep_in_config_control(unsigned int i, uint32_t mps)
{
	USB->DIEP[i-1].CTL = USB_DIEP_CTL_SNAK
		| (i << _USB_DIEP_CTL_TXFNUM_SHIFT)
		| USB_DIEP_CTL_EPTYPE_CONTROL
		| USB_DIEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_in_config_iso(unsigned int i, uint32_t mps)
{
	USB->DIEP[i-1].CTL = USB_DIEP_CTL_SNAK
		| (i << _USB_DIEP_CTL_TXFNUM_SHIFT)
		| USB_DIEP_CTL_EPTYPE_ISO
		| USB_DIEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_in_config_iso_enabled(unsigned int i, uint32_t mps)
{
	USB->DIEP[i-1].CTL = USB_DIEP_CTL_EPENA
		| USB_DIEP_CTL_CNAK
		| (i << _USB_DIEP_CTL_TXFNUM_SHIFT)
		| USB_DIEP_CTL_EPTYPE_ISO
		| USB_DIEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_in_config_bulk(unsigned int i, uint32_t mps)
{
	USB->DIEP[i-1].CTL = USB_DIEP_CTL_SNAK
		| (i << _USB_DIEP_CTL_TXFNUM_SHIFT)
		| USB_DIEP_CTL_EPTYPE_BULK
		| USB_DIEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_in_config_interrupt(unsigned int i, uint32_t mps)
{
	USB->DIEP[i-1].CTL = USB_DIEP_CTL_SNAK
		| (i << _USB_DIEP_CTL_TXFNUM_SHIFT)
		| USB_DIEP_CTL_EPTYPE_INT
		| USB_DIEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_in_disable(unsigned int i)
{
	USB->DIEP[i-1].CTL |= USB_DIEP_CTL_EPDIS | USB_DIEP_CTL_SNAK;
}
static inline void
usb_ep_in_enable(unsigned int i)
{
	USB->DIEP[i-1].CTL |= USB_DIEP_CTL_EPENA | USB_DIEP_CTL_CNAK;
}
static inline void
usb_ep_in_enable_odd(unsigned int i)
{
	USB->DIEP[i-1].CTL |= USB_DIEP_CTL_EPENA | USB_DIEP_CTL_SETD1PIDOF | USB_DIEP_CTL_CNAK;
}
static inline void
usb_ep_in_enable_even(unsigned int i)
{
	USB->DIEP[i-1].CTL |= USB_DIEP_CTL_EPENA | USB_DIEP_CTL_SETD0PIDEF | USB_DIEP_CTL_CNAK;
}
static inline void
usb_ep_in_stall(unsigned int i)
{
	USB->DIEP[i-1].CTL |= USB_DIEP_CTL_STALL;
}

/* USB_DIEPx_INT */
static inline uint32_t
usb_ep_in_flags(unsigned int i)
{
	return USB->DIEP[i-1].INT;
}
static inline void
usb_ep_in_flags_clear(unsigned int i, uint32_t v)
{
	USB->DIEP[i-1].INT = v &
		( USB_DIEP_INT_NAKINTRPT
		| USB_DIEP_INT_BBLEERR
		| USB_DIEP_INT_PKTDRPSTS
		| USB_DIEP_INT_INEPNAKEFF
		| USB_DIEP_INT_INTKNTXFEMP
		| USB_DIEP_INT_TIMEOUT
		| USB_DIEP_INT_AHBERR
		| USB_DIEP_INT_EPDISBLD
		| USB_DIEP_INT_XFERCOMPL);
}
static inline uint32_t
usb_ep_in_flag_nak(uint32_t v)           { return v & USB_DIEP_INT_NAKINTRPT; }
static inline uint32_t
usb_ep_in_flag_babble(uint32_t v)        { return v & USB_DIEP_INT_BBLEERR; }
static inline uint32_t
usb_ep_in_flag_packet_drop(uint32_t v)   { return v & USB_DIEP_INT_PKTDRPSTS; }
static inline uint32_t
usb_ep_in_flag_nak_effective(uint32_t v) { return v & USB_DIEP_INT_INEPNAKEFF; }
static inline uint32_t
usb_ep_in_flag_in_empty(uint32_t v)      { return v & USB_DIEP_INT_INTKNTXFEMP; }
static inline uint32_t
usb_ep_in_flag_timeout(uint32_t v)       { return v & USB_DIEP_INT_TIMEOUT; }
static inline uint32_t
usb_ep_in_flag_ahb_error(uint32_t v)     { return v & USB_DIEP_INT_AHBERR; }
static inline uint32_t
usb_ep_in_flag_disabled(uint32_t v)      { return v & USB_DIEP_INT_EPDISBLD; }
static inline uint32_t
usb_ep_in_flag_complete(uint32_t v)      { return v & USB_DIEP_INT_XFERCOMPL; }

/* USB_DIEPx_TSIZ */
static inline void
usb_ep_in_transfer_size(unsigned int i, uint32_t packets, uint32_t size)
{
	USB->DIEP[i-1].TSIZ = (packets << _USB_DIEP_TSIZ_PKTCNT_SHIFT)
		| size;
}
static inline uint32_t
usb_ep_in_bytes_left(unsigned int i)
{
	return USB->DIEP[i-1].TSIZ & _USB_DIEP_TSIZ_XFERSIZE_MASK;
}


/* USB_DIEPx_DMAADDR */
static inline const void *
usb_ep_in_dma_address(unsigned int i)
{
	return (const void *)USB->DIEP[i-1].DMAADDR;
}
static inline void
usb_ep_in_dma_address_set(unsigned int i, const void *addr)
{
	USB->DIEP[i-1].DMAADDR = (uint32_t)addr;
}

/* USB_DIEPx_TXFSTS */

/* USB_DOEP0CTL */
static inline void
usb_ep0out_config_64byte(void)
{
	USB->DOEP0CTL = USB_DOEP0CTL_MPS_64B;
}
static inline void
usb_ep0out_config_64byte_stall(void)
{
	USB->DOEP0CTL = USB_DOEP0CTL_STALL | USB_DOEP0CTL_MPS_64B;
}
static inline void
usb_ep0out_config_8byte(void)
{
	USB->DOEP0CTL = USB_DOEP0CTL_MPS_8B;
}
static inline void
usb_ep0out_enable(void)
{
	USB->DOEP0CTL |= USB_DOEP0CTL_EPENA | USB_DOEP0CTL_CNAK;
}
static inline void
usb_ep0out_enable_setup(void)
{
	USB->DOEP0CTL |= USB_DOEP0CTL_EPENA | USB_DOEP0CTL_STALL;
}

/* USB_DOEP0INT */
static inline uint32_t
usb_ep0out_flags(void)                   { return USB->DOEP0INT; }
static inline void
usb_ep0out_flags_clear(uint32_t v)
{
	USB->DOEP0INT = v &
		( USB_DOEP0INT_NAKINTRPT
		| USB_DOEP0INT_BBLEERR
		| USB_DOEP0INT_PKTDRPSTS
		| USB_DOEP0INT_BACK2BACKSETUP
		| USB_DOEP0INT_STSPHSERCVD
		| USB_DOEP0INT_OUTTKNEPDIS
		| USB_DOEP0INT_SETUP
		| USB_DOEP0INT_AHBERR
		| USB_DOEP0INT_EPDISBLD
		| USB_DOEP0INT_XFERCOMPL);
}
static inline uint32_t
usb_ep0out_flag_nak(uint32_t v)          { return v & USB_DOEP0INT_NAKINTRPT; }
static inline uint32_t
usb_ep0out_flag_babble(uint32_t v)       { return v & USB_DOEP0INT_BBLEERR; }
static inline uint32_t
usb_ep0out_flag_packet_drop(uint32_t v)  { return v & USB_DOEP0INT_PKTDRPSTS; }
static inline uint32_t
usb_ep0out_flag_setup_b2b(uint32_t v)    { return v & USB_DOEP0INT_BACK2BACKSETUP; }
static inline uint32_t
usb_ep0out_flag_out_disabled(uint32_t v) { return v & USB_DOEP0INT_OUTTKNEPDIS; }
static inline uint32_t
usb_ep0out_flag_setup(uint32_t v)        { return v & USB_DOEP0INT_SETUP; }
static inline uint32_t
usb_ep0out_flag_ahb_error(uint32_t v)    { return v & USB_DOEP0INT_AHBERR; }
static inline uint32_t
usb_ep0out_flag_disabled(uint32_t v)     { return v & USB_DOEP0INT_EPDISBLD; }
static inline uint32_t
usb_ep0out_flag_complete(uint32_t v)     { return v & USB_DOEP0INT_XFERCOMPL; }

/* USB_DOEP0TSIZ */
static inline void
usb_ep0out_transfer_size(uint32_t packets, uint32_t size)
{
	USB->DOEP0TSIZ = (3 << _USB_DOEP0TSIZ_SUPCNT_SHIFT)
		| (packets << _USB_DOEP0TSIZ_PKTCNT_SHIFT)
		| size;
}
static inline uint32_t
usb_ep0out_bytes_left(void)
{
	return USB->DOEP0TSIZ & _USB_DOEP0TSIZ_XFERSIZE_MASK;
}
static inline uint32_t
usb_ep0out_setup_left(void)
{
	return USB->DOEP0TSIZ >> _USB_DOEP0TSIZ_SUPCNT_SHIFT;
}

/* USB_DOEP0DMAADDR */
static inline void *
usb_ep0out_dma_address(void)
{
	return (void *)USB->DOEP0DMAADDR;
}
static inline void
usb_ep0out_dma_address_set(void *addr)
{
	USB->DOEP0DMAADDR = (uint32_t)addr;
}

/* USB_DOEPx_CTL */
static inline uint32_t
usb_ep_out_enabled(unsigned int i)
{
	return USB->DOEP[i-1].CTL & USB_DOEP_CTL_EPENA;
}
static inline uint32_t
usb_ep_out_active(unsigned int i)
{
	return USB->DOEP[i-1].CTL & USB_DOEP_CTL_USBACTEP;
}
static inline void
usb_ep_out_config_disabled(unsigned int i)
{
	USB->DOEP[i-1].CTL = 0;
}
static inline void
usb_ep_out_config_disabled_nak(unsigned int i)
{
	USB->DOEP[i-1].CTL = USB_DOEP_CTL_SNAK;
}
static inline void
usb_ep_out_config_control_enabled(unsigned int i, uint32_t mps)
{
	USB->DOEP[i-1].CTL = USB_DOEP_CTL_EPENA
		| USB_DOEP_CTL_CNAK
		| USB_DOEP_CTL_EPTYPE_CONTROL
		| USB_DOEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_out_config_iso_enabled(unsigned int i, uint32_t mps)
{
	USB->DOEP[i-1].CTL = USB_DOEP_CTL_EPENA
		| USB_DOEP_CTL_CNAK
		| USB_DOEP_CTL_EPTYPE_ISO
		| USB_DOEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_out_config_iso_enabled_odd(unsigned int i, uint32_t mps)
{
	USB->DOEP[i-1].CTL = USB_DOEP_CTL_EPENA
		| USB_DOEP_CTL_SETD1PIDOF
		| USB_DOEP_CTL_CNAK
		| USB_DOEP_CTL_EPTYPE_ISO
		| USB_DOEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_out_config_iso_enabled_even(unsigned int i, uint32_t mps)
{
	USB->DOEP[i-1].CTL = USB_DOEP_CTL_EPENA
		| USB_DOEP_CTL_SETD0PIDEF
		| USB_DOEP_CTL_CNAK
		| USB_DOEP_CTL_EPTYPE_ISO
		| USB_DOEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_out_config_bulk(unsigned int i, uint32_t mps)
{
	USB->DOEP[i-1].CTL = USB_DOEP_CTL_SNAK
		| USB_DOEP_CTL_EPTYPE_BULK
		| USB_DOEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_out_config_bulk_enabled(unsigned int i, uint32_t mps)
{
	USB->DOEP[i-1].CTL = USB_DOEP_CTL_EPENA
		| USB_DOEP_CTL_CNAK
		| USB_DOEP_CTL_EPTYPE_BULK
		| USB_DOEP_CTL_USBACTEP
		| mps;
}
static inline void
usb_ep_out_disable(unsigned int i)
{
	USB->DOEP[i-1].CTL |= USB_DOEP_CTL_EPDIS | USB_DOEP_CTL_SNAK;
}
static inline void
usb_ep_out_enable(unsigned int i)
{
	USB->DOEP[i-1].CTL |= USB_DOEP_CTL_EPENA | USB_DOEP_CTL_CNAK;
}
static inline void
usb_ep_out_enable_odd(unsigned int i)
{
	USB->DOEP[i-1].CTL |= USB_DOEP_CTL_EPENA | USB_DOEP_CTL_SETD1PIDOF | USB_DOEP_CTL_CNAK;
}
static inline void
usb_ep_out_enable_even(unsigned int i)
{
	USB->DOEP[i-1].CTL |= USB_DOEP_CTL_EPENA | USB_DOEP_CTL_SETD0PIDEF | USB_DOEP_CTL_CNAK;
}
static inline void
usb_ep_out_stall(unsigned int i)
{
	USB->DOEP[i-1].CTL |= USB_DOEP_CTL_STALL;
}

/* USB_DOEPx_INT */
static inline uint32_t
usb_ep_out_flags(unsigned int i)
{
	return USB->DOEP[i-1].INT;
}
static inline void
usb_ep_out_flags_clear(unsigned int i, uint32_t v)
{
	USB->DOEP[i-1].INT = v &
		( USB_DOEP_INT_NAKINTRPT
		| USB_DOEP_INT_BBLEERR
		| USB_DOEP_INT_PKTDRPSTS
		| USB_DOEP_INT_BACK2BACKSETUP
		| USB_DOEP_INT_STSPHSERCVD
		| USB_DOEP_INT_OUTTKNEPDIS
		| USB_DOEP_INT_SETUP
		| USB_DOEP_INT_AHBERR
		| USB_DOEP_INT_EPDISBLD
		| USB_DOEP_INT_XFERCOMPL);
}
static inline uint32_t
usb_ep_out_flag_nak(uint32_t v)                    { return v & USB_DOEP_INT_NAKINTRPT; }
static inline uint32_t
usb_ep_out_flag_babble(uint32_t v)                 { return v & USB_DOEP_INT_BBLEERR; }
static inline uint32_t
usb_ep_out_flag_packet_drop(uint32_t v)            { return v & USB_DOEP_INT_PKTDRPSTS; }
static inline uint32_t
usb_ep_out_flag_setup_b2b(uint32_t v)              { return v & USB_DOEP_INT_BACK2BACKSETUP; }
static inline uint32_t
usb_ep_out_flag_out_disabled(uint32_t v)           { return v & USB_DOEP_INT_OUTTKNEPDIS; }
static inline uint32_t
usb_ep_out_flag_setup(uint32_t v)                  { return v & USB_DOEP_INT_SETUP; }
static inline uint32_t
usb_ep_out_flag_ahb_error(uint32_t v)              { return v & USB_DOEP_INT_AHBERR; }
static inline uint32_t
usb_ep_out_flag_disabled(uint32_t v)               { return v & USB_DOEP_INT_EPDISBLD; }
static inline uint32_t
usb_ep_out_flag_complete(uint32_t v)               { return v & USB_DOEP_INT_XFERCOMPL; }

/* USB_DOEPx_TSIZ */
static inline void
usb_ep_out_transfer_size(unsigned int i, uint32_t packets, uint32_t size)
{
	USB->DOEP[i-1].TSIZ = (packets << _USB_DOEP_TSIZ_PKTCNT_SHIFT)
		| size;
}
static inline uint32_t
usb_ep_out_bytes_left(unsigned int i)
{
	return USB->DOEP[i-1].TSIZ & _USB_DOEP_TSIZ_XFERSIZE_MASK;
}

/* USB_DOEPx_DMAADDR */
static inline void *
usb_ep_out_dma_address(unsigned int i)
{
	return (void *)USB->DOEP[i-1].DMAADDR;
}
static inline void
usb_ep_out_dma_address_set(unsigned int i, void *addr)
{
	USB->DOEP[i-1].DMAADDR = (uint32_t)addr;
}

/* USB_PCGCCTL */
static inline uint32_t
usb_phy_sleeping(void)
{
	return USB->PCGCCTL & USB_PCGCCTL_PHYSLEEP;
}
static inline void
usb_phy_stop(void)
{
	USB->PCGCCTL = USB_PCGCCTL_GATEHCLK | USB_PCGCCTL_STOPPCLK;
}
static inline void
usb_phy_start(void)
{
	USB->PCGCCTL = 0UL;
}

/* USB_FIFO0Dx */

/* USB_FIFO1Dx */

/* USB_FIFO2Dx */

/* USB_FIFO3Dx */

/* USB_FIFORAMx */

#endif

#ifndef _NRF51PROG_H
#define _NRF51PROG_H

extern int nRF51_erase(void);
extern int nRF51_write(uint32_t addr, uint32_t v);
extern int nRF51_reboot_run(void);

#endif

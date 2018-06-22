#ifndef _GECKONATOR_LEUART0_H
#define _GECKONATOR_LEUART0_H

#include "common.h"
#include "leuart.h"

#define LEUARTn LEUART0
#define leuartn_(name, ...) leuart0_##name(__VA_ARGS__)
#include "leuartn.h"
#undef leuartn_
#undef LEUARTn

#endif

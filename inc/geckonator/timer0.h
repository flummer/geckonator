#ifndef _GECKONATOR_TIMER0_H
#define _GECKONATOR_TIMER0_H

#include "common.h"
#include "timer.h"

#define TIMERn TIMER0
#define timern_(name, ...) timer0_##name(__VA_ARGS__)
#include "timern.h"
#undef timern_
#undef TIMERn

#endif

#ifndef _GECKONATOR_TIMER2_H
#define _GECKONATOR_TIMER2_H

#include "common.h"
#include "timer.h"

#define TIMERn TIMER2
#define timern_(name, ...) timer2_##name(__VA_ARGS__)
#include "timern.h"
#undef timern_
#undef TIMERn

#endif

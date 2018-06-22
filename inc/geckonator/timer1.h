#ifndef _GECKONATOR_TIMER1_H
#define _GECKONATOR_TIMER1_H

#include "common.h"
#include "timer.h"

#define TIMERn TIMER1
#define timern_(name, ...) timer1_##name(__VA_ARGS__)
#include "timern.h"
#undef timern_
#undef TIMERn

#endif

#ifndef _GECKONATOR_USART0_H
#define _GECKONATOR_USART0_H

#include "common.h"
#include "usart.h"

#define USARTn USART0
#define usartn_(name, ...) usart0_##name(__VA_ARGS__)
#include "usartn.h"
#undef usartn_
#undef USARTn

#endif

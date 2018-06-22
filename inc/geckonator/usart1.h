#ifndef _GECKONATOR_USART1_H
#define _GECKONATOR_USART1_H

#include "common.h"
#include "usart.h"

#define USARTn USART1
#define usartn_(name, ...) usart1_##name(__VA_ARGS__)
#include "usartn.h"
#undef usartn_
#undef USARTn

#endif

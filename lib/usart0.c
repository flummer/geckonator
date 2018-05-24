#include "common.h"
#include "usart.h"

#define USARTn USART0
#define usartn_(name, ...) usart0_##name(__VA_ARGS__)
#include "usartn.c"
#undef usartn_
#undef USARTn

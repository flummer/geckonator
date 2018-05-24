#include "common.h"
#include "usart.h"

#define USARTn USART1
#define usartn_(name, ...) usart1_##name(__VA_ARGS__)
#include "usartn.c"
#undef usartn_
#undef USARTn

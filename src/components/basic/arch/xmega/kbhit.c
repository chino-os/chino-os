#include <avr/io.h>

int kbhit(void) { return USARTD0.STATUS & USART_RXCIF_bm; }

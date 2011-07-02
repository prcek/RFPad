#ifndef _UART_H_
#define _UART_H_

void uart_init(); 

#if AVR
int uart_read_ready();
#else
#define uart_read_ready() 0
#endif

#endif

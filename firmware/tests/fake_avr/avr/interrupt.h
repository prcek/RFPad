#ifndef _AVR_INTERRUPT_H_
#define _AVR_INTERRUPT_H_

#define 	sei()
#define 	cli()

#define ISR(a,...) void _fake_irq_vect_##a() 

#endif //_AVR_INTERRUPT_H_

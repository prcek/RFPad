/*
 * main.c
 *
 *  Created on: May 14, 2010
 *      Author: hluchant
 */
#include<stdio.h>
#include<inttypes.h>
#include<avr/io.h>
#include<avr/pgmspace.h>
#include<avr/interrupt.h>
#include<avr/eeprom.h>
#include<avr/version.h>
#include<util/delay.h>


#include"uart.h"
#include"print.h"
#include"prompt.h"
#include"rf.h"
#include"timer.h"
#include"client.h"
#include"config.h"

//////////////////////////////////////////
// LED      <-> PB5
// PS2 CLK  <-> PB0
// PS2 DATA <-> PC0
//////////////////////////////////////////



void flash_led(int count) {
	while((count--)>0) {
    	PORTB |= _BV(PB5);
		_delay_ms(50);
    	PORTB &= ~_BV(PB5);
		_delay_ms(50);
	}
}
#if 0
static uint16_t time_ms_counter = 0;
static uint16_t time_sec_counter = 0;


ISR(TIMER0_COMPA_vect) {
	time_ms_counter++;
	if (time_ms_counter>=1000) {
		time_ms_counter-=1000;
		time_sec_counter++;
	}
}


static uint16_t ps2_clk_count=0;
static uint8_t ps2_read_state = 0;
static uint8_t ps2_read_value = 0;

static uint8_t ps2_flip_flop=0;


// handle falling edge of PS/2 CLK
ISR(INT0_vect) {
	uint8_t data = (PINC & _BV(PC0))?1:0;
	if (ps2_flip_flop) {
		cc_store_0();
		ps2_flip_flop =0;
	} else {
		cc_store_1();
		ps2_flip_flop =1;
	}
	//uint8_t portc_data = PORTC;
	//printf("\r\n portc %d\r\n",portc_data);


	ps2_clk_count++;
	switch(ps2_read_state) {
	case 0: // wait for start bit
		if (!data) {
			// this is start bit
			ps2_read_state = 1;
			ps2_read_value = 0; // clear reading value
		} else {
			// ? where is start bit ?
		}
		break;
	case 8: // parita
		ps2_read_state++;
		// check parity
		// if (!ok} ps2_read_state = 0; (directly wait for start bit - 0, following stop bit is 1)
		break;
	case 9: // stop bit
		ps2_read_state = 0;
		if (data) {
			// valid stop bit;
			//emit byte
		}
		break;
	default:
		// reading bits;
		ps2_read_value |= data;
		ps2_read_value = ps2_read_value<<1;
		ps2_read_state++;
	}

}

void dump_cc() {
	uint16_t a;
	uint16_t b;
	a = cc_data.v1 - cc_data.v0;
	b = cc_data.v3 - cc_data.v2;
	a-=cc_data.calibrate;
	b-=cc_data.calibrate;
	print(PSTR("cc_values:"));
	print_hw(PSTR(" "),cc_data.v0);
	print_hw(PSTR(" "),cc_data.v1);
	print_hw(PSTR(" "),cc_data.v2);
	print_hw(PSTR(" "),cc_data.v3);
	print_dw(PSTR(" A = "),a);
	print_dw(PSTR(" B = "),b);
	print_nl();
//	printf_P(PSTR("cc values: 0x%04x 0x%04x 0x%04x 0x%04x, A = %u, B = %u\r\n"),cc_data.v0,cc_data.v1,cc_data.v2,cc_data.v3,a,b);
}
#endif 
#if 0
void dump_registers() {
	uint8_t adr;
	print(PSTR("registers dump\r\n"));
	for(adr=0xFF; adr>=0x20; adr--) {
		uint8_t data = _SFR_MEM8((uint16_t)adr);
		//printf_P(PSTR("0x%02x: 0x%02x "),adr,data);
		print_hb(0,adr);
		print_hb(PSTR(": "),data);
		print_bb(PSTR(" "), data);
//		uint8_t cnt;
//		for(cnt=0;cnt<8;cnt++) {
//			if (data & 0x80) {
//				putchar('1');
//			} else {
//				putchar('0');
//			}
//			data=data<<1;
//		}
		if (!(adr % 4)) {
			print_nl();
		} else {
			print_spaces(2);
		}
	}
}

void dump_register(uint8_t adr) {
	uint8_t data = _SFR_MEM8((uint16_t)adr);
//	printf_P(PSTR("REG at 0x%02x: 0x%02x "),adr,data);
	print_hb(PSTR("REG at "), adr);
	print_hb(PSTR(": "), data);
	print_bb(PSTR(" "), data);
//	uint8_t cnt;
//	for(cnt=0;cnt<8;cnt++) {
//		if (data & 0x80) {
//			putchar('1');
//		} else {
//			putchar('0');
//		}
//		data=data<<1;
//	}
	print_nl();
}

void dump_register2(uint8_t adr1,uint8_t adr2) {
	uint8_t data = _SFR_MEM8((uint16_t)adr1);
	//printf_P(PSTR("REG at 0x%02x: 0x%02x "),adr1,data);

	print_hb(PSTR("REG at "), adr1);
	print_hb(PSTR(": "), data);
	print_bb(PSTR(" "), data);

	data = _SFR_MEM8((uint16_t)adr2);
	print_hb(PSTR("; "), adr2);
	print_hb(PSTR(": "), data);
	print_bb(PSTR(" "), data);

	print_nl();
}

void dump_register3(uint8_t adr1,uint8_t adr2,uint8_t adr3) {
	uint8_t data = _SFR_MEM8((uint16_t)adr1);
	//printf_P(PSTR("REG at 0x%02x: 0x%02x "),adr1,data);

	print_hb(PSTR("REG at "), adr1);
	print_hb(PSTR(": "), data);
	print_bb(PSTR(" "), data);

	data = _SFR_MEM8((uint16_t)adr2);
	print_hb(PSTR("; "), adr2);
	print_hb(PSTR(": "), data);
	print_bb(PSTR(" "), data);

	data = _SFR_MEM8((uint16_t)adr3);
	print_hb(PSTR("; "), adr3);
	print_hb(PSTR(": "), data);
	print_bb(PSTR(" "), data);



	print_nl();
}
#endif

int main(void)
{
#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)
#ifndef BUILD_TIME
#define BUILD_TIME not_defined
#endif


	cli();
	//usb_code_test();
	uart_init();

//	int d=0;
	print_nl();
	print(PSTR("RFPAD " RFPAD_VERSION "; avr_libc " __AVR_LIBC_VERSION_STRING__ " " __AVR_LIBC_DATE_STRING__ "; F_CPU=" QUOTEME(F_CPU) "; BUILD_TIME=" QUOTEME(BUILD_TIME)  "\r\n"));

	cc_init();

//	dump_registers();
#if 0

//  clock counter setup

	TCNT1 = 0; // reset value
	TCCR1B = 1; // start at system clock
	cc_store_0();
	cc_store_1();
	cc_store_2();
	cc_store_3();
	cc_calibrate();

	print(PSTR("measure test\r\n"));

	cc_store_0();
	cc_store_2();
	cc_store_3();
	cc_store_1();
	dump_cc();

	cc_store_0();
	__asm__ __volatile__ ("nop" ::);
	cc_store_1();

	cc_store_2();
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
	cc_store_3();
	dump_cc();

	cc_store_0();
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
	cc_store_1();

	cc_store_2();
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
	cc_store_3();
	dump_cc();
	// clear;
	cc_store_0();
	cc_store_1();
	cc_store_2();
	cc_store_3();
#endif

// LED init
	print(PSTR("LED pin (PB5) to output mode\r\n"));
	DDRB |= _BV(PB5);  // LED pin to output mode


// PS2 init  - PD2 (clk), PC0 (data) - input mode without pull-up
	print(PSTR("RFM12 init, SDI:PB4 out/low, SCK:PB5 out/low, nSEL:PB2 out/high, DATA:PB1 in, SDO:PB3 in, nIRQ:PD2/INT0 in\r\n"));
	rf_io_com_init();
	rf_config_init();		
	rf_setup();
	print(PSTR("rf com init done\n\r"));



	DDRD &= ~_BV(PD2);
//	PORTD &= ~_BV(PD2); 

#if 0
	print(PSTR("PINB, DDRB, PORTB: "));
	dump_register3(0x25,0x24,0x23);  //B
	print(PSTR("PINC, DDRC, PORTC: "));
	dump_register3(0x28,0x27,0x26);  //C
	print(PSTR("PIND, DDRD, PORTD: "));
	dump_register3(0x2B,0x2A,0x29);  //D
#endif


	//interupt on fall edge of clock
	//
	//      _____   _   _   _   _   _   _   _   _   _   ____
	// CLK       | | | | | | | | | | | | | | | | | | | |
	// (EINT0)   |_| |_| |_| |_| |_| |_| |_| |_| |_| |_|
	//
	// DATA ___     ___ ___ ___ ___ ___ ___ ___ ___ ___
	//         | S | D | D | D | D | D | D | D | P | E |
	//         |_B_|_0_|_1_|_2_|_3_|_4_|_5_|_6_|_B_| B |___
	//         start           data          parita stop
	//
	//
	// EINT0 falling edge
	//
/* - from http://www.computer-engineering.org/ps2protocol/
 * Data sent from the device to the host is read on the falling edge of the clock signal;
 * data sent from the host to the device is read on the rising edge.
 * The clock frequency must be in the range 10 - 16.7 kHz.
 * This means clock must be high for 30 - 50 microseconds and low for 30 - 50 microseconds..
 * If you're designing a keyboard, mouse, or host emulator,
 * you should modify/sample the Data line in the middle of each cell.
 * I.e.  15 - 25 microseconds after the appropriate clock transition.
 * Again, the keyboard/mouse always generates the clock signal,
 * but the host always has ultimate control over communication.
 *
 */

/*
	EICRA &= ~_BV(ISC00);  // falling edge
	EICRA |= _BV(ISC01);
	EIMSK |= _BV(INT0);  //unmask EINT0



	print(PSTR("Start Timer0\r\n"));
	TCNT0 = 0; // reset TIMER0 value
	OCR0A =  15; // max value, 16x1024 = 1ms
	TCCR0A = 2; // WGM = 2 (2 bit in tccr0a, 1 bit in tccr0b)
	TIMSK0 |= _BV(OCIE0A);  // enable interupt on A
	TCCR0B = 5;  // clock select for TIMER0 - f/1024  - timer start
*/


	//enable interupt
//	sei();

	print(PSTR("INIT done\r\n"));

//	flash_led(5);

//	int c;
//	printf_P(PSTR("waiting for char\r\n"));
//	c = uart_getchar(0);
//	printf_P(PSTR("waiting for char\r\n"));
//	c = uart_getchar(0);

	uint8_t max_delay;
	uint8_t autorun;
	sei();	
again:
	autorun = 0;
	max_delay = 0;
	set_print_info();
	while(!uart_read_ready()) {
		_delay_ms(100);
		if ((max_delay &0x0f) == 0) {
			print_db(PSTR("autorun in "),50 - max_delay);
			flash_led(1);
			print_nl();
		}
		if (max_delay>50) {
			autorun = 1;
			break;
		}
		max_delay++;
	}
	if (autorun) {
		print(PSTR("autorun...\n\r"));
		flash_led(2);
		do_autorun();
		print_nl();
		print(PSTR("END of autorun\r\n"));
	} else {
		while(do_prompt());
		print_nl();
		print(PSTR("END\r\n"));
	}
//	flash_led(5);
	goto again;

//	printf_P(PSTR("eeprom write start\r\n"));
//	eeprom_busy_wait();	
//	eeprom_write_byte ((uint8_t *)(2), (uint8_t)(0xFF));		
//	eeprom_busy_wait();	

//	printf_P(PSTR("eeprom write done\r\n"));


//	printf_P(PSTR("getch() >:\r\n"));
//	while(1) {
//		c = uart_getchar(0);
//		flash_led(1);
//		printf_P(PSTR("getch() reads: [%d]\r\n"),c);
//	}
#if 0
    while (1)
    {
    	printf_P(PSTR("ps2_clk_count %d, read state %d, read value %d\r\n"),ps2_clk_count, (uint16_t)ps2_read_state, (uint16_t)ps2_read_value);
    	dump_register3(0x29,0x26,0x23);
    	dump_cc();
        printf_P(PSTR("loop %d, time %d.%03d\r\n"),d++,time_sec_counter,time_ms_counter);
    	flash_led(1);
	_delay_ms(900);
    }
#endif
    return (1);	// should never happen
}

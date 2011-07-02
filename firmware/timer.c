#include<stdio.h>
#include<string.h>
#include<inttypes.h>
#include<avr/io.h>
#include<avr/pgmspace.h>
#include<avr/interrupt.h>
#include<avr/eeprom.h>
#include<util/delay.h>



#include"print.h"
#include"scan.h"
#include"timer.h"


static struct {
	uint16_t v0;
	uint16_t v1;
	uint16_t v2;
	uint16_t v3;
	uint16_t calibrate;
} cc_data;


static uint16_t t_msec; 
static uint16_t t_sec;
volatile static uint8_t t_slot;
volatile static uint8_t t_slot_lec;
static uint8_t t_slot_len;
static uint8_t t_slot_top;
//static uint8_t t_slot_offset;

#ifdef AVR 

void cc_store_0() __attribute__ ((noinline));
void cc_store_1() __attribute__ ((noinline));
void cc_store_2() __attribute__ ((noinline));
void cc_store_3() __attribute__ ((noinline));

void cc_store_0()
{
	cc_data.v0 = TCNT1;
}
void cc_store_1()
{
	cc_data.v1 = TCNT1;
}
void cc_store_2()
{
	cc_data.v2 = TCNT1;
}
void cc_store_3()
{
	cc_data.v3 = TCNT1;
}
#else

void cc_store_0(){};
void cc_store_1(){};
void cc_store_2(){};
void cc_store_3(){};

#endif

void cc_calibrate() {
	cc_data.calibrate = cc_data.v1 - cc_data.v0;
}
void cc_clear() {
	cc_store_0();
	cc_store_1();
	cc_store_2();
	cc_store_3();
}

void cc_dump() {
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
uint16_t cc_get_a() {
	uint16_t a;
	a = cc_data.v1 - cc_data.v0;
	a-=cc_data.calibrate;
	return a;
}
uint16_t cc_get_b() {
	uint16_t b;
	b = cc_data.v3 - cc_data.v2;
	b-=cc_data.calibrate;
	return b;
}

ISR(TIMER0_COMPA_vect) {
	t_msec++;
	if (t_msec>=1000) {
		t_msec=0;
		t_sec++;
	}	
	if (!(t_slot_lec--)) {
		t_slot_lec = t_slot_len;
		if (t_slot==t_slot_top) {
			t_slot=0;
		} else {
			t_slot++;
		}
	}
}



void cc_init() {
#ifdef AVR

// set 16bit timer 1
	TCNT1 = 0; // reset value
	TCCR1B = 1; // start at system clock

// set 8bit timer 0 -- generate irq 64/us (64000 ticks = 1 sec)
	TCNT0 = 0;  //reset value
	OCR0A = 249; //TOP value
	TIMSK0 = 2; // enable compare A interupt

	TCCR0A = 2; // clear on Top value (OCR0A)	
	TCCR0B = 3; //start at 1/64 system clock (250 tick at 250Khz = 1 msec)

#endif //AVR


		

	t_msec = 0;
	t_sec = 0;
	t_slot_len = 1;

	cc_store_0();
	cc_store_1();
	cc_store_2();
	cc_store_3();
	cc_calibrate();
#if 0
	print(PSTR("timer calibration\r\n"));

	cc_store_0();
	cc_store_2();
	cc_store_3();
	cc_store_1();
	cc_dump();

	cc_store_0();
	__asm__ __volatile__ ("nop" ::);
	cc_store_1();

	cc_store_2();
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
	cc_store_3();
	cc_dump();

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
	cc_dump();
	if (cc_get_a()==3) {
		print(PSTR("A value OK\r\n"));
	} else {
		print(PSTR("A value ERROR\r\n"));
	}

	if (cc_get_b()==4) {
		print(PSTR("B value OK\r\n"));
	} else {
		print(PSTR("B value ERROR\r\n"));
	}

#endif
	// clear;
	cc_store_0();
	cc_store_1();
	cc_store_2();
	cc_store_3();

}
uint16_t timer_get_sec() {
	return t_sec;
}
void timer_show() {
	print(PSTR("timer show: "));
	print_hw(PSTR(" sec: "), t_sec);
	print_hw(PSTR(" msec: "), t_msec);
#if AVR
	print_hb(PSTR(" timer: "), TCNT0 );
#endif
	print_nl();	
}
void timer_clear() {
	cli();
	t_sec = 0;
	t_msec = 0;
#if AVR
	TCNT0 = 0;
#endif
	sei();
	timer_show();
}


void cc_slot_set_top(uint8_t t) {
	t_slot_top = t;	
}
void cc_slot_set(uint8_t s) {
	cli();
	t_slot = s;
	t_slot_lec = t_slot_len;
#if AVR
	//t_slot_offset = TCNT0;
	TCNT0 = 0;
#else
	//t_slot_offset = 0;
#endif
	sei();
}
uint8_t cc_slot_get() {
	uint8_t s;
	cli();	
	s = t_slot;
	sei();
	return s;
}
void cc_slot_wait(uint8_t s) {
	while(s!=cc_slot_get()); 
	
}

void cc_slot_set_len(uint8_t sl) {
	t_slot_len = sl;
}


int timer_do_prompt() {
        char cmd[17];
        //scanf("%16s",cmd);
        scan_key(cmd,17);
        if (strcmp(cmd,"show")==0) {
			timer_show();
                        return 1;
        }
        if (strcmp(cmd,"clear")==0) {
			timer_clear();
                        return 1;
        }

	if (strcmp(cmd,"slot")==0) {
//		print_hb(PSTR("slot offset = "),t_slot_offset);
		print_hb(PSTR("top = "), t_slot_top);
		print_hb(PSTR(", val = "), t_slot);
		print_nl();
	}
	if (strcmp(cmd,"slot_get")==0) {
		uint16_t c = scan_uint16();
		while(c--) {
		print_hb(PSTR("slot get = "),cc_slot_get()); print_nl();
		}
	}

	if (strcmp(cmd,"slot_set_top")==0) {
		uint8_t t = scan_uint8();
		cc_slot_set_top(t);
	}
//	if (strcmp(cmd,"slot_set_off")==0) {
//		uint8_t t = scan_uint8();
//		t_slot_offset = t;
//	}
	
	if (strcmp(cmd,"slot_wait")==0) {
		uint8_t t = scan_uint8();
		cc_slot_wait(t);
	}

	if (strcmp(cmd,"slot_test")==0) {
	//	cc_slot_set_top(20);
	//	cc_slot_set(20);


		cc_slot_wait(20);	
		cc_start_a();
		cc_slot_wait(0);	
		cc_start_b();
		cc_slot_wait(1);
		cc_stop_a();
		cc_stop_b();
		print_dw(PSTR("a = "),cc_get_a());
		print_dw(PSTR(", b = "),cc_get_b());
		cc_slot_wait(19);
		cc_start_a();
		cc_slot_wait(20);	
		cc_start_b();
		cc_slot_wait(0);
		cc_stop_a();
		cc_stop_b();
		print_dw(PSTR("a = "),cc_get_a());
		print_dw(PSTR(", b = "),cc_get_b());
	
		print_nl();
		
	}
        return 1;
}


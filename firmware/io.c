
#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<util/delay.h>
#include<avr/pgmspace.h>
#include"print.h"
#include"scan.h"
#include"ee.h"

#include"io.h"


#ifdef AVR 
	#define io_write(a,v) *((volatile uint8_t *)a)=(uint8_t)(v)
	#define io_read(a) ((uint8_t) *((volatile const uint8_t *)a));
#else
	static uint8_t io_ports[65536];
	#define io_write(a,v) io_ports[(a)]=(v)	
	#define io_read(a) io_ports[(a)];
#endif

int io_do_prompt() {
	char cmd[17];
	//scanf("%16s",cmd);
	scan_key(cmd,17);
	if (strcmp(cmd,"w")==0) {
			uint16_t a;
			uint16_t v;
			//scanf("%" SCNu16 " %" SCNu16 ,&a, &v);	
			a = scan_uint16();
			v = scan_uint16();
			io_write(a,(uint8_t)v);			
			return 1;
	}
	if (strcmp(cmd,"r")==0) {
			uint16_t a;
			//scanf("%" SCNu16 ,&a);	
			a = scan_uint16();
			uint8_t v = io_read(a);				

			print_hw(PSTR(""), a);
			print_hb(PSTR(" "), v);
        	       	print_bits(PSTR("["),v,7,0);
	                print(PSTR("]\n"));

			return 1;

	}
	if (strcmp(cmd,"m")==0) {
			uint16_t a;
			uint16_t c;
			//scanf("%" SCNu16 ,&a);	
			a = scan_uint16();
			c = scan_uint16();
			while(c--) {
				uint8_t v = io_read(a);				
				print_hw(PSTR(""), a);
				print_hb(PSTR(" "), v);
	        	       	print_bits(PSTR("["),v,7,0);
		                print(PSTR("]\n"));
				_delay_ms(50);
			}
	}

	return 1;
}



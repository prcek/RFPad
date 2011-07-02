#include<inttypes.h>
#include<stdio.h>
#include<ctype.h>
#include<avr/pgmspace.h>
#include"scan.h"
#include"print.h"


static uint8_t last_char = 0;
static uint8_t last_error = 0;

uint8_t scan_get_last_error() {
	return last_error;
}

uint8_t scan_gc() {
	last_char = fgetc(stdin);
	return last_char;
}
void scan_ungc() {
	ungetc(last_char,stdin);	
}

uint8_t scan_key(char *str, uint8_t max_len) {
	uint8_t c;
	uint8_t r = 0;
	do {
		c = scan_gc();
	} while(isspace(c));
	while(--max_len) {
		r++;
		*str = c;
		str++;
		c = scan_gc();
		if (isspace(c)) {
			scan_ungc();	
			break;
		}
	}				
	*str = 0;
	return r;
}

uint8_t is_hexdigit(uint8_t c) {
	if ((c>='0') && (c<='9')) { return c-'0'; }
	if ((c>='a') && (c<='f')) { return c-'a'+10; }
	if ((c>='A') && (c<='F')) { return c-'A'+10; }
	return 255;
}

uint8_t scan_uint8() {
	uint8_t c;	
	uint8_t r = 0;
	do {
		c = scan_gc();
	} while(isspace(c));
	while(isdigit(c)) {
		r = r*10+(c - '0');
		c = scan_gc();
	}
	if ((c=='x') && (r == 0)) {
		c = scan_gc();
		while( (c=is_hexdigit(c))!=255 ) {
			r = r*16+c;
			c = scan_gc();
		}			
	}
	scan_ungc();	
	return r;
}

uint16_t scan_uint16() {
	uint8_t c;	
	uint16_t r = 0;
	do {
		c = scan_gc();
	} while(isspace(c));
	while(isdigit(c)) {
		r = r*10+(c - '0');
		c = scan_gc();
	}		
	if ((c=='x') && (r == 0)) {
		c = scan_gc();
		while( (c=is_hexdigit(c))!=255 ) {
			r = r*16+c;
			c = scan_gc();
		}			
	}
	scan_ungc();	
	return r;
}

void scan_test() {
	print(PSTR("scan test, type 123: "));	
	uint8_t v0 = scan_uint8();
	print_db(PSTR("value = "), v0);
	print_nl();

	print(PSTR("type 0xff: "));	
	v0 = scan_uint8();
	print_hb(PSTR("value = "), v0);
	print_nl();



	print(PSTR("type 12345: "));	
	uint16_t v1 = scan_uint16();
	print_dw(PSTR("value = "), v1);
	print_nl();

	print(PSTR("type 0x55AA: "));	
	v1 = scan_uint16();
	print_hw(PSTR("value = "), v1);
	print_nl();



	print(PSTR("ok, end of scan test\r\n"));

}

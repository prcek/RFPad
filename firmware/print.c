#include<inttypes.h>
#include<stdio.h>
#include<avr/pgmspace.h>
#include"print.h"





void print(const char *str) {
	if (str==0) {
		return ;
	}
	fputs_P(str,stdout);
}

void print_nl() {
	fputc('\n',stdout);
	fputc('\r',stdout);
}
void print_ok_nl() {
	fputc('O',stdout);
	fputc('K',stdout);
	fputc('\n',stdout);
	fputc('\r',stdout);
}


void print_space() {
	fputc(' ',stdout);
}
void print_spaces(uint8_t c) {
	while(c--) {
		fputc(' ',stdout);
	}
}
void print_char(uint8_t c) {
	fputc(c,stdout);
}

void print_hex_prefix() {
	fputc('0',stdout);
	fputc('x',stdout);
}


void print_hb_internal(uint8_t v) {
	uint8_t c;
	c = (v & 0xf0)>>4;
	c += c<10?'0':'A'-10;
	fputc(c,stdout);

	c=(v&0x0f);
	c += c<10?'0':'A'-10;
	fputc(c,stdout);
}

void print_hex(uint8_t v) {
	print_hb_internal(v);	
}

void print_hb(const char *str, uint8_t v) {
	print(str);
	print_hex_prefix();
	print_hb_internal(v);
//	printf_P(PSTR("%02X"),v);
}
void print_hw(const char *str, uint16_t v) {
	print(str);
	print_hex_prefix();
	print_hb_internal((uint8_t)(v >> 8));
	print_hb_internal((uint8_t)(v));
//	printf_P(PSTR("%04X"),v);
}
void print_bit(const char *str, uint16_t v, uint8_t b) {
	print(str);
	if (v&(1<<b)) {
		fputc('1',stdout);
	} else {
		fputc('0',stdout);
	}
}
void print_bits(const char *str, uint16_t v, uint8_t bh, uint8_t bl) {
	print(str);
	while(bh>bl) {
		print_bit(0,v,bh);
		bh--;
	}
	print_bit(PSTR(""),v,bl);
}

void print_hbits(const char *str, uint16_t v, uint8_t bh, uint8_t bl) {
	v = v & ((2U<<bh)-1U);
	v = v>>bl;
	if ((bh-bl)>7) {
		print_hw(str,v);
	} else {
		print_hb(str,v);
	}
}
void print_bb(const char *str, uint8_t v) {
	print_bits(str,(uint16_t)v, 7,0);
}

void print_bw(const char *str, uint16_t v) {
	print_bits(str,v, 15,0);
}

void print_db(const char *str, uint8_t v) {
	uint8_t n1,n0;
	print(str);
	if (v==0) {
		fputc('0',stdout);
		return;
	}
	n0 = v % 10; 
	v = v / 10;
	n1 = v % 10;
	v = v / 10;
	if (v!=0) {
		fputc('0'+v,stdout);
		fputc('0'+n1,stdout);
		fputc('0'+n0,stdout);
	} else if (n1 != 0) {
		fputc('0'+n1,stdout);
		fputc('0'+n0,stdout);
	} else {
		fputc('0'+n0,stdout);
	}	
}

void print_dw(const char *str, uint16_t v) {
	print(str);
	uint8_t n3,n2,n1,n0;
	if (v==0) {
		fputc('0',stdout);
		return;
	}
	n0 = v % 10; 
	v = v / 10;
	n1 = v % 10;
	v = v / 10;
	n2 = v % 10;
	v = v / 10;
	n3 = v % 10;
	v = v / 10;
	
	if (v!=0) {
		fputc('0'+v,stdout);
		fputc('0'+n3,stdout);
		fputc('0'+n2,stdout);
		fputc('0'+n1,stdout);
		fputc('0'+n0,stdout);
	} else if (n3 != 0) {
		fputc('0'+n3,stdout);
		fputc('0'+n2,stdout);
		fputc('0'+n1,stdout);
		fputc('0'+n0,stdout);
	} else if (n2 != 0) {
		fputc('0'+n2,stdout);
		fputc('0'+n1,stdout);
		fputc('0'+n0,stdout);
	} else if (n1 != 0) {
		fputc('0'+n1,stdout);
		fputc('0'+n0,stdout);
	} else {
		fputc('0'+n0,stdout);
	}	

}


void print_test() {
	print(PSTR("print test\n\r"));
	print_hw(PSTR("hex numbers: 0x0000 = "),0x0000); 
	print_hw(PSTR(", 0x1234 = "),0x1234); 
	print_hw(PSTR(", 0xFFFF = "),0xFFFF); 
	print_hb(PSTR(", 0x00 = "),0x00); 
	print_hb(PSTR(", 0x12 = "),0x12); 
	print_hb(PSTR(", 0xA5 = "),0xA5); 
	print_nl();	
	print_bw(PSTR("bin numbers: 0xAA55 (1010101001010101) = "),0xAA55);
	print_bb(PSTR(", 0x81 (10000001) = "),0x81);
	print_nl();	
	print_db(PSTR("dec numbers: 255 = "),255);
	print_db(PSTR(", 0 = "),0);
	print_db(PSTR(", 1 = "),1);
	print_db(PSTR(", 99 = "),99);
	print_db(PSTR(", 101 = "),101);
	print_dw(PSTR(", 65535 = "), 65535);
	print_dw(PSTR(", 1 = "), 1);
	print_dw(PSTR(", 12 = "), 12);
	print_dw(PSTR(", 123 = "), 123);
	print_dw(PSTR(", 1234 = "), 1234);
	print_dw(PSTR(", 12345 = "), 12345);
	print_nl();	
}

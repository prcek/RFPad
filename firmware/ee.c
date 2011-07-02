#include<inttypes.h>
#include<avr/pgmspace.h>
#include<avr/eeprom.h>
#include"ee.h"
#include"print.h"

//static uint8_t ee_data[256] = { 0xE7,0x39,0x06,0x40,0x47,0x04,0xA0,0xAC,0x81,0xD4,0x83,0x00,0x50,0x17,0x00,0x00,0x00,0x00,0x40 };
void eewr_b(uint16_t a, uint8_t v) {
	uint8_t *ee_base = 0;
	eeprom_write_byte(ee_base+a,v);					
}
uint8_t eerd_b(uint16_t a) {
	const uint8_t *ee_base = 0;
	return eeprom_read_byte(ee_base+a);
}
void eewr_w(uint16_t a, uint16_t v) {
        eewr_b(a,(v&0xff00)>>8);
        eewr_b(a+1,(v&0xff));
}
uint16_t eerd_w(uint16_t a) {
        uint16_t r = eerd_b(a) << 8;
        r+=eerd_b(a+1);
        return r;
}
void ee_dump(){
        uint16_t a;
        print("ee:");
        for(a=0; a<32; a++) {
                print_hb(PSTR(" "), eerd_b(a));
        }
        print_nl();
}


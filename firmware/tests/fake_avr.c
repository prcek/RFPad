#include"fake_avr/avr/eeprom.h"

uint8_t PORTB;
uint8_t DDRB;
uint8_t PINB;

uint8_t PORTC;
uint8_t DDRC;
uint8_t PINC;

uint8_t PORTD;
uint8_t DDRD;
uint8_t PIND;

static uint8_t ee_data[256] = { 0xE7,0x39,0x06,0x40,0x47,0x04,0xA0,0xAC,0x81,0xD4,0x83,0x00,0x50,0x17,0x00,0x00,0x00,0x00,0x40 };

uint8_t eeprom_read_byte(const uint8_t *p) {
	const uint8_t *base=0;
	int r = (int)(p-base);
	return ee_data[r];
}
void eeprom_write_byte(uint8_t *p, uint8_t v) {
	uint8_t *base=0;
	int r = (int)(p-base);
	ee_data[r]=v;
}


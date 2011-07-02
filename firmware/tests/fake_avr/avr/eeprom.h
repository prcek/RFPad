#ifndef _EEPROM_H_

#include<inttypes.h>


uint8_t eeprom_read_byte(const uint8_t *);
void eeprom_write_byte(uint8_t *, uint8_t);

#endif

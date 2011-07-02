#ifndef _EE_H_
#define _EE_H_

#include<inttypes.h>

#define EE_RF_BASE 0
#define EE_AUTO_MODE 20
#define EE_CLIENT_ID 21
#define EE_SLOT_SIZE 22
#define EE_SLOT_COUNT 23
#define EE_SLOT_REST_DELAY 24
#define EE_SLOT_RF_TIMEOUT 25


void eewr_b(uint16_t a, uint8_t v);
uint8_t eerd_b(uint16_t a);
void eewr_w(uint16_t a, uint16_t v);
uint16_t eerd_w(uint16_t a);
void ee_dump();

#endif


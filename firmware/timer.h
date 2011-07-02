#ifndef _TIMER_H_
#define _TIMER_H_



void cc_init();
void cc_clear();
void cc_dump();
void cc_store_0();
void cc_store_1();
void cc_store_2();
void cc_store_3();
uint16_t cc_get_a();
uint16_t cc_get_b();


#define cc_start_a cc_store_0
#define cc_stop_a cc_store_1
#define cc_start_b cc_store_2
#define cc_stop_b cc_store_3


uint16_t cc_get_sec();
uint16_t cc_get_msec();

void cc_slot_set_top(uint8_t);
void cc_slot_set(uint8_t);
uint8_t cc_slot_get();
void cc_slot_wait(uint8_t);
void cc_slot_set_len(uint8_t sl);

int timer_do_prompt();
uint16_t timer_get_sec();


#endif

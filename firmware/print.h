#ifndef _PRINT_H_
#define _PRINT_H_



void print(const char *str);
void print_nl();
void print_ok_nl();
void print_err_nl();
void print_space();
void print_spaces(uint8_t c);
void print_char(uint8_t c);
void print_hex(uint8_t c);
void print_hb(const char *str, uint8_t v);
void print_hw(const char *str, uint16_t v);
void print_bit(const char *str, uint16_t v, uint8_t b);
void print_bits(const char *str, uint16_t v, uint8_t bh, uint8_t bl);
void print_hbits(const char *str, uint16_t v, uint8_t bh, uint8_t bl);

void print_bb(const char *str, uint8_t v);
void print_bw(const char *str, uint16_t v);

void print_db(const char *str, uint8_t v);
void print_dw(const char *str, uint16_t v);



void print_test();



#endif


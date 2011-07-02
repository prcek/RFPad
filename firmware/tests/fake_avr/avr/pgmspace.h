#ifndef _PGMSPACE_H_

#define PSTR(x) x
#define printf_P(args...) printf(args)
#define fputs_P(args...) fputs(args)

#define prog_uint8_t uint8_t
#define pgm_read_byte(a) (*((uint8_t *)(a)))

#endif

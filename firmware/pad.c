#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<util/delay.h>
#include<avr/pgmspace.h>
#include"timer.h"
#include"print.h"
#include"ee.h"
#include"timer.h"
#include"uart.h"
#include"config.h"
#include"pad.h"
#include"scan.h"


//TODO: check jeenode for correct value
#define SCL 0
#define SDA 0

#define HI(x) (PORTB |= (1<<(x)))
#define LO(x) (PORTB &= ~(1<<(x)))
#define IN(x) (PINB & (1<<(x)))

#define OUTPUT 1
#define INPUT 1
#define PINMODE(x,m) { if ((m)==OUTPUT) {DDRB |= (1 <<(x));} else {DDRB &= ~(1<<(x)); } }

void i2c_hold() {
    _delay_ms(1); //TODO: change to 5us!
    /*
        from pcf8574 spec:
        max 100KHz, one tick 10us, correct delay is 5us

        20*10us = 200us, total read time < 0.5ms - OK
    */
}

void i2c_sdaOut(uint8_t value){
    if (value) {
        PINMODE(SDA,INPUT);
        HI(SDA);
    } else {
        PINMODE(SDA,OUTPUT);
        LO(SDA);
    }
}

uint8_t i2c_sdaIn() {
    if (IN(SDA)) {
        return 1;
    }
    return 0;
}

void i2c_sclHi() {
    i2c_hold();
    HI(SCL);
}

void i2c_sclLo() {
    i2c_hold();
    HI(SCL);
}

void i2c_init() {
    i2c_sdaOut(1);     
    PINMODE(SCL,OUTPUT); 
    i2c_sclHi();
}

uint8_t i2c_write(uint8_t data) {
    i2c_sclLo();
    for (uint8_t mask = 0x80; mask != 0; mask >>=1) {
        i2c_sdaOut(data & mask);
        i2c_sclHi();
        i2c_sclLo();
    } 
    i2c_sdaOut(1);
    i2c_sclHi();
    uint8_t ack =  ! i2c_sdaIn();
    i2c_sclLo();
    return ack;
}

uint8_t i2c_start(uint8_t addr) {
    i2c_sclLo();
    i2c_sclHi();
    i2c_sdaOut(0);
    return i2c_write(addr);
}

void i2c_stop() {
    i2c_sdaOut(0);
    i2c_sclHi();
    i2c_sdaOut(1);
}

uint8_t i2c_read(uint8_t last) {
    uint8_t data = 0;
    for (uint8_t mask = 0x80; mask !=0; mask >>=1) {
        i2c_sclHi();
        if (i2c_sdaIn()) {
            data |= mask;
        }
        i2c_sclLo();
    }
    i2c_sdaOut(last);
    i2c_sclHi();
    i2c_sclLo();
    if (last) {
        i2c_stop();
    }
    i2c_sdaOut(1);
    return data;
}


void pad_init() {
}


void pad_test() {

}

uint8_t pad_read() {
    return 0;
}

void pad_autoread() {
	print(PSTR("pad autoread\n\r"));
	pad_init();
	while(!uart_read_ready()) {
        pad_read();
    }
}



uint8_t pad_do_prompt() {
        char cmd[17];
        scan_key(cmd,17);
        if (strcmp(cmd,"init")==0) {
            pad_init();
            return 1;
        }
        if (strcmp(cmd,"test")==0) {
            pad_test();
            return 1;
        }
        if (strcmp(cmd,"read")==0) {
            pad_read();
            return 1;
        }
        if (strcmp(cmd,"autoread")==0) {
            pad_autoread();
            return 1;
        }
 
        return 1;
	
}


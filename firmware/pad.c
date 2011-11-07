#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<util/delay.h>
#include<avr/io.h>
#include<avr/pgmspace.h>
#include"timer.h"
#include"print.h"
#include"ee.h"
#include"timer.h"
#include"uart.h"
#include"config.h"
#include"pad.h"
#include"scan.h"


//JeeNode port pins
// jeenode PORT1 = IRQ:PD3, SDA:AIO:PC0, SCL:DIO:PD4
// jeenode PORT2 = IRQ:PD3, SDA:AIO:PC1, SCL:DIO:PD5
// jeenode PORT3 = IRQ:PD3, SDA:AIO:PC2, SCL:DIO:PD6
// jeenode PORT4 = IRQ:PD3, SDA:AIO:PC3, SCL:DIO:PD7

// iface A, bottom (1) port = (io) = AIO = SDA, DIO = SCL, IRQ = INT
// iface A, upper (2) port = (led) = AIO = LED G, DIO = LED Y


#define SCL_PORT PORTD
#define SCL_DDR DDRD
#define SCL_PIN PIND
#define SCL 4

#define SDA_PORT PORTC
#define SDA_DDR DDRC
#define SDA_PIN PINC
#define SDA 0

#define LEDY_PORT PORTD
#define LEDY_DDR DDRD
#define LEDY_PIN PIND
#define LEDY 5

#define LEDG_PORT PORTC
#define LEDG_DDR DDRC
#define LEDG_PIN PINC
#define LEDG 1



#define HI(port,x) ( port |= (1<<(x)))
#define LO(port,x) ( port &= ~(1<<(x)))
#define IN(port,x) ( port & (1<<(x)))

#define OUTPUT 1
#define INPUT 0
#define PINMODE(port,x,m) { if ((m)==OUTPUT) {(port) |= (1 <<(x));} else {(port) &= ~(1<<(x)); } }

void led_y_on() { HI(LEDY_PORT,LEDY); }
void led_g_on() { HI(LEDG_PORT,LEDG); }
void led_y_off() { LO(LEDY_PORT,LEDY); }
void led_g_off() { LO(LEDG_PORT,LEDG); }

void i2c_hold() {
    _delay_us(5); //TODO: change to 5us!
    /*
        from pcf8574 spec:
        max 100KHz, one tick 10us, correct delay is 5us

        20*10us = 200us, total read time < 0.5ms - OK
    */
}

void i2c_sdaOut(uint8_t value){
    if (value) {
        PINMODE(SDA_DDR, SDA,INPUT);
	led_g_on();
        HI(SDA_PORT,SDA);
    } else {
        PINMODE(SDA_DDR, SDA,OUTPUT);
	led_g_off();
        LO(SDA_PORT,SDA);
    }
}

uint8_t i2c_sdaIn() {
    if (IN(SDA_PIN,SDA)) {
        return 1;
    }
    return 0;
}

void i2c_sclHi() {
    i2c_hold();
    led_y_on();
    HI(SCL_PORT,SCL);
}

void i2c_sclLo() {
    i2c_hold();
    led_y_off();
    LO(SCL_PORT,SCL);
}

void i2c_init() {
    i2c_sdaOut(1);     
    PINMODE(SCL_DDR,SCL,OUTPUT); 
    i2c_sclHi();
}

uint8_t i2c_write(uint8_t data) {
    i2c_sclLo();
    uint8_t mask;
    for (mask = 0x80; mask != 0; mask >>=1) {
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
    uint8_t mask; 
    for (mask = 0x80; mask !=0; mask >>=1) {
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


void led_init() {
	PINMODE(LEDY_DDR, LEDY,OUTPUT);
        LO(LEDY_PORT,LEDY);
	PINMODE(LEDG_DDR, LEDG,OUTPUT);
        LO(LEDG_PORT,LEDG);
}
#define I2C_ADDR 0x40

void pad_init() {
    i2c_init();
    led_init();
}


void pad_led() {
	led_y_on();
    _delay_ms(1000); //TODO: change to 5us!
	led_y_off();
	led_g_on();
    _delay_ms(1000); //TODO: change to 5us!
	led_g_off();
}

void pad_test() {
    uint8_t ok = i2c_start(I2C_ADDR);
    i2c_stop();
    if (ok) {
	    print(PSTR("pad test ok\n\r"));
    } else {
	    print(PSTR("pad test error\n\r"));
    }
}

uint8_t pad_write(uint8_t data) {
    uint8_t ok = i2c_start(I2C_ADDR ); 
    uint8_t ok2 = i2c_write(data);
    i2c_stop();
}

uint8_t pad_read() {
    uint8_t ok = i2c_start(I2C_ADDR | 1); 
    uint8_t data = i2c_read(1);  
    return data;
}

void pad_autoread() {
	print(PSTR("pad autoread\n\r"));
	pad_init();
 //   pad_write(0xff); //all pins HIGH = INPUT
	uint8_t ld =0;
	while(!uart_read_ready()) {
        uint8_t d =pad_read();
	if (ld!=d) {	

		print_hb(0,d);
		ld=d;
		}
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
    if (strcmp(cmd,"led")==0) {
            pad_led();
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


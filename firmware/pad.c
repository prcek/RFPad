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


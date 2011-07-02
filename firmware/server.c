#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<util/delay.h>
#include<avr/pgmspace.h>
#include"timer.h"
#include"print.h"
#include"scan.h"
#include"ee.h"
#include"rf.h"
#include"timer.h"
#include"uart.h"

#include"config.h"

//#define MAX_CLIENTS 16
//#define SLOTS 20


// slot def:
// 0 - server 0,0xff, seq , data 
// 1 - tx/rx switch gap
// 2 .. 17 - clients  - id(0x80 + id), seq, data, data
// 18 .. 19 - tx/rx switch gap (and processing gap)


// client status:

// 0 - empty
// 1 - ?
// 0x80 + last seq 



static uint8_t server_seq;
static uint8_t cfg_slot_rf_timeout;
//static uint8_t clients[MAX_CLIENTS];


void server_init() {
	server_seq  = 0;
	cc_slot_set_len(eerd_b(EE_SLOT_SIZE));
	cc_slot_set_top(SLOTS-1);
	cc_slot_set(0);
	cfg_slot_rf_timeout =  eerd_b(EE_SLOT_RF_TIMEOUT);
}

void server_read_client(uint8_t cid) {
	uint8_t rc = rf_recv_packet(cfg_slot_rf_timeout);
	if (rc==1) {
		uint8_t cid = rf_get_packet_v0(); //client id
		//seq = rf_get_packet_v1(); //client seq
		uint8_t data0 = rf_get_packet_v2(); //client data0
		uint8_t data1 = rf_get_packet_v3(); //client data1
//		print_hex(cid);
//		print_char('{');
		print_hex(data0);
//		print_hex(data1);
//		print_char('}');
	} if (rc==0) {
		//timeout
		print_char('.');
	} else {
		//crc error or unexpected 
		print_char(':');
	}
}

void server_loop() {
	rf_prepare_for_send();
	cc_slot_wait(FIRST_SERVER_SLOT);		
	rf_send_packet(0x0,server_seq, 0, 0);
	server_seq++;
	cc_slot_wait(FIRST_SERVER_SLOT+1);
	rf_prepare_for_recv();
	uint8_t c;
	print_char('[');
	for(c=0; c<MAX_CLIENTS; c++) {
		cc_slot_wait(FIRST_CLIENT_SLOT+c);
		server_read_client(c);		
		if (cc_slot_get()!=(FIRST_CLIENT_SLOT+c)) {
			print_char('^');
		}
	}
	cc_slot_wait(FIRST_CLIENT_SLOT+MAX_CLIENTS);
	print_char(']');
}

void server_autorun() {
	print(PSTR("server autorun\n\r"));
	server_init();
	while(!uart_read_ready()) {
        	server_loop();
        }
}



uint8_t server_do_prompt() {
        char cmd[17];
        //scanf("%16s",cmd);
        scan_key(cmd,17);
        if (strcmp(cmd,"init")==0) {
                        server_init();
                        return 1;
        }
        if (strcmp(cmd,"loop")==0) {
			uint16_t count = scan_uint16();
			if (count == 0) {
				uint16_t last_s = timer_get_sec();
				uint8_t last_seq = server_seq;
				print(PSTR("server loop...."));	
				while(!uart_read_ready()) {
					server_loop();
				//	uint16_t now_s = timer_get_sec();
				//	if (now_s!=last_s) {
				//		last_s = now_s;
				//		print_hb(PSTR(" "),server_seq-last_seq);
				//		last_seq = server_seq;
				//	}
				}	
				print(PSTR("   end\n\r"));
			} else { 
				while(count--) {
					print_char('.');					
                        		server_loop();
				}
			}
                        return 1;
        }
        return 1;
	
}


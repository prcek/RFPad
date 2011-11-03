#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<avr/pgmspace.h>
#include"print.h"
#include"scan.h"

#include"rf.h"
#include"io.h"
#include"ee.h"
#include"timer.h"
#include"server.h"
#include"client.h"
#include"pad.h"

void set_print_info() {
	uint8_t mode = eerd_b(EE_AUTO_MODE);
	print_hb(PSTR("auto mode (0:client, 1:server) = "),mode);
	print_nl();

	uint8_t slot_size = eerd_b(EE_SLOT_SIZE);
	print_hb(PSTR("slot_size (in ms) = "),slot_size);
	print_nl();

	uint8_t slot_count = eerd_b(EE_SLOT_COUNT);
	print_hb(PSTR("slot_count (clients = slots-3) = "),slot_count);
	print_nl();

	uint8_t slot_rest_delay = eerd_b(EE_SLOT_REST_DELAY);
	print_hb(PSTR("slot_rest_delay (in 50us) = "),slot_rest_delay);
	print_nl();

	uint8_t slot_rf_timeout = eerd_b(EE_SLOT_RF_TIMEOUT);
	print_hb(PSTR("slot_rf_timeout (in 20us) = "),slot_rf_timeout);
	print_nl();


	uint8_t cid = eerd_b(EE_CLIENT_ID);
	print_hb(PSTR("client_id = "),cid);
	print_nl();
}

uint8_t set_do_prompt() {
        char cmd[17];
        scan_key(cmd,17);
        if (strcmp(cmd,"auto_client")==0) {
		eewr_b(EE_AUTO_MODE,0);
		print_ok_nl();
		return 1;
        }
	 if (strcmp(cmd,"auto_server")==0) {
		eewr_b(EE_AUTO_MODE,1);
		print_ok_nl();
      		return 1;
        }

	if (strcmp(cmd,"client_id")==0) {
		uint8_t cid = scan_uint8();
		eewr_b(EE_CLIENT_ID,cid);
		print_ok_nl();
		return 1;
	}

	if (strcmp(cmd,"slot_size")==0) {
		uint8_t slot_size = scan_uint8();
		eewr_b(EE_SLOT_SIZE,slot_size);
		print_ok_nl();
		return 1;
	}

	if (strcmp(cmd,"slot_count")==0) {
		uint8_t slot_count = scan_uint8();
		eewr_b(EE_SLOT_COUNT,slot_count);
		print_ok_nl();
		return 1;
	}

	if (strcmp(cmd,"slot_rest_delay")==0) {
		uint8_t slot_rest_delay = scan_uint8();
		eewr_b(EE_SLOT_REST_DELAY,slot_rest_delay);
		print_ok_nl();
		return 1;
	}

	if (strcmp(cmd,"slot_rf_timeout")==0) {
		uint8_t slot_rf_timeout = scan_uint8();
		eewr_b(EE_SLOT_RF_TIMEOUT,slot_rf_timeout);
		print_ok_nl();
		return 1;
	}


	if (strcmp(cmd,"info")==0) {
		set_print_info();
		return 1;
	}
	return 1;

}
void do_autorun() {
	uint8_t mode = eerd_b(EE_AUTO_MODE);
	if (mode == 0) {
		client_autorun();
	} else  if (mode == 1) {
		server_autorun();
	} else {
		print(PSTR("autorun mode error\n\r"));
	}
}

uint8_t do_prompt() {

	print(PSTR(">"));
	char group[10];
//	scanf("%4s",group);
	scan_key(group,10);

	if (strcmp(group,"rf")==0) {
		return rf_do_prompt();
	} else if (strcmp(group,"io")==0) {
		return io_do_prompt();
	} else if (strcmp(group,"test")==0) {
		print_test();
		scan_test();
	} else if (strcmp(group,"timer")==0) {
		return timer_do_prompt();
	} else if (strcmp(group,"server")==0) {
		return server_do_prompt();
	} else if (strcmp(group,"client")==0) {
		return client_do_prompt();
	} else if (strcmp(group,"set")==0) {
		return set_do_prompt();
    } else if (strcmp(group,"pad")==0) {
        return pad_do_prompt();
	} else if (strcmp(group,".")==0) {
		return 0;
	} 
	return 1;
}

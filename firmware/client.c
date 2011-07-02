#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<util/delay.h>
#include<avr/pgmspace.h>
#include<avr/io.h>
#include"timer.h"
#include"print.h"
#include"scan.h"
#include"ee.h"
#include"rf.h"
#include"timer.h"
#include"uart.h"
#include"config.h"



static uint16_t stats_ok;
static uint16_t stats_inv_seq;
static uint16_t stats_bad_crc;
static uint16_t stats_timeout;

static uint8_t dbg_mode;

static uint8_t cfg_client_id;
static uint8_t cfg_slot_rest_delay;
static uint8_t cfg_slot_rf_timeout;

#define DEBUG_CHAR(c) if (dbg_mode) { print_char(c);}
#define DEBUG_NL() if (dbg_mode) { print_nl(); }
#define DEBUG(s) if (dbg_mode) { print(PSTR(s)); }

void client_init() {
	dbg_mode = 1;	
	cfg_client_id = eerd_b(EE_CLIENT_ID);
	cfg_slot_rest_delay = eerd_b(EE_SLOT_REST_DELAY);
	cfg_slot_rf_timeout = eerd_b(EE_SLOT_RF_TIMEOUT);
	//pad input - PC0:5, as input
	DDRC &=~0x3f;


	cc_slot_set_len(eerd_b(EE_SLOT_SIZE));
	cc_slot_set_top(SLOTS-1);
        cc_slot_set(0);
	
}

uint8_t client_read_pad() {
	return PINC &0x3f;	
}
/*
void client_loop() {
	uint8_t it = 64;	
	uint8_t ok = 0;
	uint8_t next_seq = 0;
	uint8_t next_seq_v = 0;
	while(it--) {
		uint8_t rc = rf_recv_packet(30);
		if (rc == 1) {
			uint8_t seq = rf_get_packet_v2();
			if (next_seq_v==0) {
				next_seq = seq;
				next_seq_v = 1;
			}
			if (next_seq==seq) {
				stats_ok++;
				ok++;
				next_seq++;
			} else {
				next_seq_v = 0;
				stats_inv_seq++;
			}
	//		print_hb(0, seq);
		} else {
			if (rc == 0) {
				stats_timeout++;
			} else if (rc == 2) {
				stats_bad_crc++;
			}
			next_seq_v = 0;
		}
	}		
	print_char('0'+(ok >> 3));
}
*/


void client_loop() {
	static uint8_t connected = 0;
	static uint8_t last_seq = 0;
	static uint8_t client_seq = 0;
	rf_prepare_for_recv();
	if (!connected) {
		DEBUG_CHAR('?');
		uint8_t rc = rf_recv_packet(SLOTS*cfg_slot_rf_timeout);
		DEBUG_CHAR('0'+rc);
		if (rc == 1) {
			if (rf_get_packet_v0()!=0) { 
				// not server packet...	
				DEBUG_CHAR('n');
				return; // break loop = try again
			}	
			DEBUG_CHAR('s');
			connected = 1; // server sync received;
			last_seq = rf_get_packet_v1();
			connected = 1;
			uint8_t srd = cfg_slot_rest_delay;	
			while(srd--) {
			_delay_us(50);	 // post packet delay 	
			}
			cc_slot_set(1);	 // sync clock
		} else {
			// log error
			return;
		}
	} else {
		DEBUG_CHAR('[');
		cc_slot_wait(0);
		uint8_t rc = rf_recv_packet(cfg_slot_rf_timeout);
		DEBUG_CHAR('0'+rc);
		if (rc == 1) {
			if (rf_get_packet_v0()!=0) { 
				// not server packet...	
				DEBUG_CHAR('n');
				return; // break loop = try again
			}	
			DEBUG_CHAR('s');
			connected = 1; // server sync received;
			last_seq = rf_get_packet_v1();
			connected = 1;
			uint8_t srd = cfg_slot_rest_delay;	
			while(srd--) {
			_delay_us(50);	 // post packet delay 	
			}
			cc_slot_set(1);	 // sync clock
			
		} else {
			//log error
			connected = 0;
			DEBUG_CHAR('d');
		} 	
		if (!connected) return;	
	}
	DEBUG_CHAR(':');
	rf_prepare_for_send();
	cc_slot_wait(FIRST_CLIENT_SLOT+cfg_client_id);
	uint8_t client_data0 = client_read_pad();
	uint8_t client_data1 = ~client_data0;
	rf_send_packet(0x80+cfg_client_id,client_seq++, client_data0, client_data1);
	if (cc_slot_get()!=(FIRST_CLIENT_SLOT+cfg_client_id)) {
		DEBUG_CHAR('^');
	}
	
	cc_slot_wait(FIRST_CLIENT_SLOT+MAX_CLIENTS);
	DEBUG_CHAR(']');
	
}


uint8_t client_check_for_server(uint8_t c) {
	rf_tune_channel(c);
	uint8_t it = 100;
	uint8_t ok = 0;
	uint8_t next_seq = 0;
	while(it--) {
		uint8_t rc = rf_recv_packet(30);
		if (rc == 1) {
			uint8_t seq = rf_get_packet_v2();
	//		print_hb(0, seq);
			if (ok==0) {
				next_seq = seq;
			} else if (next_seq!=seq) {
				return 0;
			}
			ok++;
			next_seq++;
		} else {
			return 0;
		}
	}
	return 1;
}


void client_autotune() {
	uint8_t channels = rf_cfg_get_channel_count();
	print_hw(PSTR("autotune channels="), channels);	
	uint8_t bw = rf_cfg_get_channel_bw();
	print_hb(PSTR(", bw = "), bw);
	print_nl();
	rf_scan_start();
	uint8_t c;
	for(c=0; c<channels; c++) {
		uint16_t v = rf_scan_get_signal_level_at_channel(c);
		print_hb(PSTR("channel "),c);
		uint16_t f = rf_cfg_get_channel_f0(c, bw);
		print_hw(PSTR(", freq "),f);
		print_hw(PSTR(" - sig_level "),v);
		if (v > 0) {
			rf_scan_stop();
			if (client_check_for_server(c)) {
				print(PSTR(" server detected"));
			} else {
				print(PSTR(" no server"));
			}
			rf_scan_start();
		}
		print_nl();
	}
	rf_scan_stop();
}

void client_select_channel(uint8_t c) {
	print_hb(PSTR("setting channel "), c);
	rf_tune_channel(c);
	print_nl();
	print(PSTR("listening for server packet..."));
	uint8_t it = 10;
	uint8_t ok = 0;
	while(it--) {
		uint8_t rc = rf_recv_packet(50);
		print_char('0'+rc);
		print_hb(PSTR(" v0:"), rf_get_packet_v0());
		print_hb(PSTR(" v1:"), rf_get_packet_v1());
		print_hb(PSTR(" v2:"), rf_get_packet_v2());
		print_hb(PSTR(" v3:"), rf_get_packet_v3());
		print_nl();
		if (rc == 1) {
			ok++;
		}
	}
	print_nl();
	print_hb(PSTR("result = "),ok);
	print_nl();
}

void client_autorun() {
	print(PSTR("client autorun\n\r"));
	rf_client_hack();
	client_init();
	while(!uart_read_ready()) {
        	client_loop();
        }
}

uint8_t client_do_prompt() {
        char cmd[17];
        //scanf("%16s",cmd);
        scan_key(cmd,17);
        if (strcmp(cmd,"init")==0) {
                        client_init();
			print_ok_nl();
                        return 1;
        }
        if (strcmp(cmd,"loop")==0) {
                        uint16_t count = scan_uint16();

                        if (count == 0) {
                                print(PSTR("client loop...."));
                                while(!uart_read_ready()) {
                                        client_loop();
                                }
                                print(PSTR("   end\n\r"));
                        } else {
                                while(count--) {
                                        print_char('.');
                                        client_loop();
                                }
                        }

                        return 1;
        }
	if (strcmp(cmd,"autotune")==0) {
			client_autotune();
			return 1;
	}

	if (strcmp(cmd,"channel")==0) {
			uint8_t c = scan_uint8();
			client_select_channel(c);
			return 1;
	}

	if (strcmp(cmd,"check")==0) {
			uint8_t c = scan_uint8();
			if (client_check_for_server(c)) {
				print(PSTR("ok\r\n"));
			} else {
				print(PSTR("fail\r\n"));
			}
			return 1;
	}

	if (strcmp(cmd,"pad")==0) {
		uint8_t l = client_read_pad();
		print_hb(0,l);
		while(!uart_read_ready()) {
			uint8_t d = client_read_pad();		
			if (d!=l) {
				l = d;
				print_hb(PSTR(" "), d);
			}
		}
		
		return 1;
	}

	if (strcmp(cmd,"reset_stats")==0) {
		stats_ok = 0;
		stats_inv_seq = 0;
		stats_bad_crc = 0;
		stats_timeout = 0;
		print(PSTR("stat reset ok"));
		return 1;
	}

	if (strcmp(cmd,"stats")==0) {
		print_dw(PSTR("ok: "), stats_ok);
		print_dw(PSTR(" inv_seq: "), stats_inv_seq);
		print_dw(PSTR(" bad_crc: "), stats_bad_crc);
		print_dw(PSTR(" timeout: "), stats_timeout);
		print_nl();
		return 1;
	}



        return 1;

}


#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<util/delay.h>
#include<util/crc16.h>
#include<avr/pgmspace.h>
#include<avr/io.h>
#include"timer.h"
#include"print.h"
#include"scan.h"
#include"ee.h"
#include"rf.h"


#define	RF_EE_BASE EE_RF_BASE

typedef struct {
	uint8_t config; //cmd=0x80[E7]: band and capacitors (el ef b1 b0 x3 x2 x1 x0 ) default 11 10 0111  EL,EF, 868MHz, 12.5pF 				
	uint8_t power; //cmd=0x82[39]: power management
	uint16_t freq; //cmd=0xA[640]: frequency (f11 .. f0, in range 96 - 3903)	
	uint8_t rate; //cmd=0xC6[02]: data rate ( cs, r6 .. r0) 	
	uint16_t rcontrol; //cmd=0x9[4A0]: receiver control (P16, d1,d0, i2,i1,i0, g1,g0, r2,r1,r0)	
	uint8_t dfilter; //cmd=0xC2[AC]: data filter command (al, ml, 1, s, 1, f2,f1,f0)	
	uint8_t afc; //cmd=0xC4[83]:
	uint16_t txcfg; //cmd=0x98[..]
} rf_config_t;

rf_config_t rf_config;



static uint8_t rf_packet[10] = { 0xaa,0xaa, 0x2d, 0xd4, 0,0,0,0, 0xff, 0x0 };



uint16_t rf_config_get(uint8_t r); 
//void rf_config_load(uint8_t r);


#define RF_RECEIVER_ON  0x82DD
#define RF_XMITTER_ON   0x823D
#define RF_IDLE_MODE    0x820D
#define RF_SLEEP_MODE   0x8205
#define RF_WAKEUP_MODE  0x8207
#define RF_TXREG_WRITE  0xB800
#define RF_RX_FIFO_READ 0xB000
#define RF_WAKEUP_TIMER 0xE000

#define RF_LBD_BIT      0x0400
#define RF_RSSI_BIT     0x0100


#define RF_FREQ_MIN 100
#define RF_FREQ_MAX 3900

#define RF_BW_0 40
#define RF_BW_1 80
#define RF_BW_2 68
#define RF_BW_3 54
#define RF_BW_4 40
#define RF_BW_5 27
#define RF_BW_6 16
#define RF_BW_7 40

#define RF_FREQ_COUNT ((RF_FREQ_MAX) - (RF_FREQ_MIN))

static prog_uint8_t rf_channel_bw_consts[8] = { RF_BW_0, RF_BW_1, RF_BW_2, RF_BW_3, RF_BW_4, RF_BW_5, RF_BW_6, RF_BW_7 };
static prog_uint8_t rf_channel_count_consts[8] = { RF_FREQ_COUNT/RF_BW_0, RF_FREQ_COUNT/RF_BW_1, RF_FREQ_COUNT/RF_BW_2, RF_FREQ_COUNT/RF_BW_3, RF_FREQ_COUNT/RF_BW_4, RF_FREQ_COUNT/RF_BW_5, RF_FREQ_COUNT/RF_BW_6, RF_FREQ_COUNT/RF_BW_7 };

#define RF_CHANNEL_BW(c) pgm_read_byte(rf_channel_bw_consts+c)
#define RF_CHANNEL_COUNT(c) pgm_read_byte(rf_channel_count_consts+c)

//#define DELAY _delay_us(1)  //ms
#define SCK 5
#define SDI 4
#define SDO 3 
#define CS 2

#define HI(x) PORTB |= (1<<(x))
#define LO(x)  PORTB &= ~(1<<(x))

#define nIRQ (PIND & _BV(2))


#define _SPI_MODE


void rf_io_com_init() {
#ifdef AVR

#ifdef SPI_MODE
	HI(CS);
	DDRB = (1<<CS) | (1<<SDI) | (1<<SCK);
	// use clk/8 = 2MHz (2x 1/16th) to avoid exceeding RF12's SPI specs of 2.5 MHz
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
	SPSR |= _BV(SPI2X);
#else //SPI_MODE
	HI(CS);
	HI(SDI);
	LO(SCK);
	DDRB = (1<<CS) | (1<<SDI) | (1<<SCK);
#endif //!SPI_MODE

#endif //AVR
}

uint16_t rf_io_com_write(uint16_t cmd) {
	uint16_t recv = 0;
#ifdef AVR
#ifdef SPI_MODE
	LO(CS);
	SPDR = cmd >> 8;
	while (!(SPSR & _BV(SPIF)))
	;
	recv = SPDR << 8;
	SPDR = cmd;
	while (!(SPSR & _BV(SPIF)))
	; 
	recv |= SPDR;
	HI(CS);
#else
	uint8_t i;

	LO(SCK);
	LO(CS);
	for(i=0; i<16; i++) {
		if (cmd & 0x8000) {
			HI(SDI);
		} else {
			LO(SDI);
		}
		HI(SCK);	
		recv<<=1;
		if (PINB & (1<<SDO)) {
			recv|=0x0001;
		}
		LO(SCK);
		cmd<<=1;
	}
	HI(CS);
#endif
#endif
	return recv;
}

void rf_io_com_write_only(uint16_t cmd) {
#ifdef AVR
#ifdef SPI_MODE
	LO(CS);
	SPDR = cmd >> 8;
	while (!(SPSR & _BV(SPIF)))
	;
//	recv = SPDR << 8;
	SPDR = cmd;
	while (!(SPSR & _BV(SPIF)))
	; 
//	recv |= SPDR;
	HI(CS);
#else
	uint8_t i;

	LO(SCK);
	LO(CS);
	for(i=0; i<16; i++) {
		if (cmd & 0x8000) {
			HI(SDI);
		} else {
			LO(SDI);
		}
		HI(SCK);	
		cmd<<=1;
		LO(SCK);
	}
	HI(CS);
#endif
#endif
}


uint16_t rf_cmd_write(uint16_t cmd) {
	return rf_io_com_write(cmd);
}

uint8_t rf_cfg_get_channel_bw() {
	uint8_t i = (rf_config_get(5) & 0x00e0) >> 5;	
	uint8_t v = RF_CHANNEL_BW(i);
	return v;
}
uint16_t rf_cfg_get_channel_f0(uint8_t channel, uint8_t bw) {
	uint16_t f = ((uint16_t)RF_FREQ_MIN)+(((uint16_t)bw)*channel) + (bw>>1);
	return f;
}
uint8_t rf_cfg_get_channel_count() {
	uint8_t i = (rf_config_get(5) & 0x00e0) >> 5;	
	uint8_t v = RF_CHANNEL_COUNT(i);
	return v;
}


void rf_scan_change_freq(uint16_t f) {
	rf_io_com_write_only( 0xA000 | ( f &0xfff));	
}
void rf_scan_change_rssi(uint8_t s) {
	uint16_t c = rf_config_get(5);	
	rf_io_com_write_only( (c & 0xfff8) | ( ((uint16_t)s) & 7 ));
}
uint8_t rf_scan_is_rssi() {
	uint16_t s = rf_io_com_write(0);
	if (s & (1<<8) /* rssi bit */) {
		return 1;
	}
	return 0;	
}
void rf_scan_start() {
	rf_io_com_write_only(RF_RECEIVER_ON);
}

void rf_scan_stop() {
	rf_io_com_write_only(RF_IDLE_MODE);
}

uint8_t rf_scan_get_signal_level_at_freq(uint16_t freq) {
	uint8_t ret = 0;
	rf_scan_change_freq(freq);
	rf_scan_change_rssi(1);
	_delay_ms(1);
	uint8_t s;
	uint8_t l;
	uint8_t sd = 0;
	for(l = 0; l<200; l++) {
		if (rf_scan_is_rssi()) {
			sd = 1;
			break;
		}
		_delay_us(100);
	}
	if (!sd) return 0;
	ret++;

	for(s = 2; s<7 ;s++) {
		rf_scan_change_rssi(s);
		for(l=0; l<200; l++) {
			if (rf_scan_is_rssi()) {
				ret++;
				break;
			}
		}
	}			
	return ret;
}

uint16_t rf_scan_get_signal_level_at_channel(uint8_t channel) {
	uint16_t val = 0;
	uint8_t bw = rf_cfg_get_channel_bw();
	uint16_t freq = rf_cfg_get_channel_f0(channel,bw);
	freq-=bw>>1;	
	while(bw--) {
		val += rf_scan_get_signal_level_at_freq(freq);
		freq++;
	}
	return val;	
}
void rf_tune_channel(uint8_t channel) {
	uint8_t bw = rf_cfg_get_channel_bw();
	uint16_t freq = rf_cfg_get_channel_f0(channel,bw);
	rf_scan_change_freq(freq);
}

void rf_scan(uint16_t start_f, uint16_t end_f, uint16_t step, uint16_t loops) {
//	print(PSTR("rf scan\n\r"));
	while(loops--) {
		uint16_t f;
		for(f = start_f; f < end_f; f+=step) {
			uint8_t s = 1;
		 	rf_scan_change_rssi(1);
			rf_scan_change_freq(f);
			_delay_ms(10);
			while(rf_scan_is_rssi() && (s<7)) {
				s++;
				rf_scan_change_rssi(s);
				_delay_ms(10);
			}
			print_char(s+'0');			
		}	
		print_nl();
	}	
//	print(PSTR("end of scan\n"));
}

void rf_send_packet(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3) {

//	rf_io_com_write(0);
	rf_io_com_write_only(RF_XMITTER_ON);

	rf_packet[4]=v0;
	rf_packet[5]=v1;
	rf_packet[6]=v2;
	rf_packet[7]=v3;

	uint8_t crc = 0;		
	crc = _crc_ibutton_update(crc,v0);
	crc = _crc_ibutton_update(crc,v1);
	crc = _crc_ibutton_update(crc,v2);
	crc = _crc_ibutton_update(crc,v3);
	
	rf_packet[8] = crc;

	uint8_t c; 
	for(c=0; c<10; c++) {
		while (nIRQ);
		rf_io_com_write_only(0xB800 | rf_packet[c]);
	}
	while (nIRQ);
	rf_io_com_write_only(RF_IDLE_MODE);


//	rf_io_com_write(0);
}

uint16_t rf_recv_select(uint16_t tv) {
	while(tv) {
		uint8_t i = 1;
		while(i--) {
			uint16_t s = rf_io_com_write(0);			
			if (s&0x8000) {
				return tv;
			}
			_delay_us(5);
		}
		tv--;
	} 
	return tv;
}

uint8_t rf_recv_packet(uint16_t tv) {
	uint8_t rc = 0;
	rf_io_com_write_only(0xCA81);
	rf_io_com_write_only(0xCA83);
	rf_io_com_write_only(0xB000); // fake read 
	rf_io_com_write_only(0xB000); // fake read... 
	rf_io_com_write_only(RF_RECEIVER_ON);

	uint8_t crc = 0;
	uint8_t c;
// hack: skip first byte.... unexpected zero...
	tv = rf_recv_select(tv); if (tv==0) goto abort;
	uint8_t d = rf_io_com_write(0xB000);
	
	for(c=4; c<9; c++) {
		tv = rf_recv_select(tv); if (tv==0) goto abort;
		d = rf_io_com_write(0xB000);
		//print_hb(PSTR(" "),d);
		rf_packet[c] = d;
		if (c<8) {
			crc = _crc_ibutton_update(crc,d);
		}	
	}
	if (crc == rf_packet[8]) {
		rc = 1;
	} else {
		rc = 2;
	}
abort:
//	print_nl();
	rf_io_com_write(RF_IDLE_MODE);
	return rc;
}

uint8_t rf_get_packet_v0() {
	return rf_packet[4];
}
uint8_t rf_get_packet_v1() {
	return rf_packet[5];
}
uint8_t rf_get_packet_v2() {
	return rf_packet[6];
}
uint8_t rf_get_packet_v3() {
	return rf_packet[7];
}


void rf_prepare_for_send() {}
void rf_prepare_for_recv() {}


void rf_print_packet() {
	uint8_t p;
	for(p=0; p<10; p++) {
		uint8_t d = rf_packet[p];
		if (p!=0) {
			print_char(' ');
		}
		print_hb(0,d);
	}
}


#if 0
void rf_send(char *data, uint16_t count) {

	print_char('i');
	rf_io_com_write(0);
	rf_io_com_write(RF_XMITTER_ON);
	while(count--) {
		uint8_t aa = 10;
		while(aa--) {
			while (nIRQ);
	//		rf_io_com_write(0);
			rf_io_com_write(0xB8AA);
			print_char('.');
		}

		while (nIRQ);
		rf_io_com_write(0xB82D);
		while (nIRQ);
		rf_io_com_write(0xB8D4);
		print_char('#');

			char *d = data;
			char b;
			while((b = *(d++))) {
				while (nIRQ);
				rf_io_com_write(0xB800 | ((uint16_t)b));
				print_char(b);
			}

		while (nIRQ);
		rf_io_com_write(0xB800);
		print_char('#');
		
		aa = 10;
		while(aa--) {
			while (nIRQ);
	//		rf_io_com_write(0);
			rf_io_com_write(0xB8AA);
			print_char('.');
		}
	}

	print(PSTR("\r\nEND\r\n"));
}



void rf_recv(uint16_t c) {
	print(PSTR("recv on\n\r"));
	rf_io_com_write(RF_RECEIVER_ON);
	while(c--) {
		uint8_t w = 100;
		uint16_t s = 0;
		while(w--) {
			s = rf_io_com_write(0);	
			if (s & 0x8000) break;		
		}
		if (s & 0x8000) {
			s = rf_io_com_write(0xb000);	
			print_hb(0,(uint8_t) s);
		} else {
			print_char('.');
		}
	}

	print(PSTR("\r\nEND\r\n"));
	
}
void rf_tx_test() {
#if AVR
	print(PSTR("off tx\n\r"));	
	rf_io_com_write(RF_SLEEP_MODE);
	rf_io_com_write(0);
	while (!(PIND & _BV(2))) {
		print(PSTR("dummy write "));
		rf_io_com_write(0xB8AA);
		rf_io_com_write(0);
	}
	print(PSTR("test start\r\n"));
	cc_start_a();
	rf_io_com_write(RF_XMITTER_ON);
	cc_start_b();
	while(PIND & _BV(2)) {
	}	
	cc_stop_b();
	cc_stop_a();
	print_dw(PSTR("A: "),cc_get_a()); 	
	print_dw(PSTR(" B: "),cc_get_b()); 	
	print_nl();

	print(PSTR("wakeup tx\n\r"));	
	rf_io_com_write(RF_WAKEUP_MODE);
	rf_io_com_write(0);
	while (!(PIND & _BV(2))) {
		print(PSTR("dummy write "));
		rf_io_com_write(0xB8AA);
		rf_io_com_write(0);
	}
	print(PSTR("test start\r\n"));
	cc_start_a();
	rf_io_com_write(RF_XMITTER_ON);
	cc_start_b();
	while(PIND & _BV(2)) {
	}	
	cc_stop_b();
	cc_stop_a();
	print_dw(PSTR("A: "),cc_get_a()); 	
	print_dw(PSTR(" B: "),cc_get_b()); 	
	print_nl();

	print(PSTR("idle tx\n\r"));	
	rf_io_com_write(RF_IDLE_MODE);
	rf_io_com_write(0);
	while (!(PIND & _BV(2))) {
		print(PSTR("dummy write "));
		rf_io_com_write(0xB8AA);
		rf_io_com_write(0);
	}
	print(PSTR("test start\r\n"));
	cc_start_a();
	rf_io_com_write(RF_XMITTER_ON);
	cc_start_b();
	while(PIND & _BV(2)) {
	}	
	cc_stop_b();
	cc_stop_a();
	print_dw(PSTR("A: "),cc_get_a()); 	
	print_dw(PSTR(" B: "),cc_get_b()); 	
	print_nl();
#endif

}

void rf_rx_test(uint16_t m, uint16_t c) {
	rf_io_com_write(0);
	print(PSTR("rx on\n\r"));
	print_hw(PSTR("mask = "),m);
	print_hw(PSTR("count = "),c);
	print_nl();
	rf_io_com_write(RF_RECEIVER_ON);
	uint16_t ls = 0xffff;
	while(c--) {
		uint16_t s = rf_io_com_write(0) & m;
		
		if (s!=ls) {
			print_hw(0,s);
			print_nl();
		} 
		ls = s;
	}
	
}
#endif


void rf_config_init() {
	memset(&rf_config,0,sizeof(rf_config_t));

	rf_config.config=eerd_b(RF_EE_BASE+0); 
	rf_config.power=eerd_b(RF_EE_BASE+1); 
	rf_config.freq=eerd_w(RF_EE_BASE+2); 
	rf_config.rate=eerd_b(RF_EE_BASE+4);
	rf_config.rcontrol=eerd_w(RF_EE_BASE+5); 
	rf_config.dfilter=eerd_b(RF_EE_BASE+7);
	rf_config.afc=eerd_b(RF_EE_BASE+10);
	rf_config.txcfg=eerd_w(RF_EE_BASE+11);

}


void rf_config_dump() {

	print_hb(PSTR("01:cfg="),rf_config.config);
	print_bit(PSTR("[EL="),rf_config.config,7); 
	print_bit(PSTR(", EF="),rf_config.config,6); 
	print_bits(PSTR(", B="),rf_config.config,5,4); print_hbits(PSTR(":"), rf_config.config,5,4);
	print_bits(PSTR(", X="),rf_config.config,3,0); print_hbits(PSTR(":"), rf_config.config,3,0);
	print(PSTR("]\n"));

	print_hb(PSTR("02:power="),rf_config.power);
	print_bit(PSTR("[ER="),rf_config.power,7);
	print_bit(PSTR(", EBB="), rf_config.power,6);
	print_bit(PSTR(", ET="), rf_config.power,5);
	print_bit(PSTR(", ES="), rf_config.power,4);
	print_bit(PSTR(", EX="), rf_config.power,3);
	print_bit(PSTR(", EB="), rf_config.power,2);
	print_bit(PSTR(", EW="), rf_config.power,1);
	print_bit(PSTR(", DC="), rf_config.power,0);
	print(PSTR("]\n"));

	print_hw(PSTR("03:freq="),rf_config.freq);
	print_bits(PSTR("[F="), rf_config.freq,11,0); 
	print(PSTR("]\n"));

	print_hb(PSTR("04:rate="),rf_config.rate);
	print_bit(PSTR("[CS="), rf_config.rate,7);
	print_bits(PSTR(", R="), rf_config.rate,6,0);  print_hbits(PSTR(":"), rf_config.rate,6,0);
	print(PSTR("]\n"));

	print_hw(PSTR("05:rcontrol="), rf_config.rcontrol);
	print_bit(PSTR("[P16="), rf_config.rcontrol,10);
	print_bits(PSTR(", D="), rf_config.rcontrol,9,8); print_hbits(PSTR(":"), rf_config.rcontrol,9,8);
	print_bits(PSTR(", I="), rf_config.rcontrol,7,5); print_hbits(PSTR(":"), rf_config.rcontrol,7,5);
	print_bits(PSTR(", G="), rf_config.rcontrol,4,3); print_hbits(PSTR(":"), rf_config.rcontrol,4,3);
	print_bits(PSTR(", R="), rf_config.rcontrol,2,0); print_hbits(PSTR(":"), rf_config.rcontrol,2,0);
	print(PSTR("]\n"));
	
	print_hb(PSTR("06:dfilter="), rf_config.dfilter);	
	print_bit(PSTR("[AL="), rf_config.dfilter, 7);
	print_bit(PSTR(", ML="), rf_config.dfilter, 6);
	print_bit(PSTR(", S="), rf_config.dfilter, 4);
	print_bits(PSTR(", F="), rf_config.dfilter,2,0); print_hbits(PSTR(":"), rf_config.dfilter,2,0);
	print(PSTR("]\n"));

	print_hb(PSTR("10:afc="), rf_config.afc);
	print_bits(PSTR("[A="), rf_config.afc,7,6);  print_hbits(PSTR(":"), rf_config.afc,7,6);
	print_bits(PSTR(", RL="), rf_config.afc,5,4);  print_hbits(PSTR(":"), rf_config.afc,5,4);
	print_bit(PSTR(", ST="), rf_config.afc,3);
	print_bit(PSTR(", FI="), rf_config.afc,2);
	print_bit(PSTR(", OE="), rf_config.afc,1);
	print_bit(PSTR(", EN="), rf_config.afc,0);
	print(PSTR("]\n"));

	print_hw(PSTR("11:txcfg="), rf_config.txcfg);
	print_bit(PSTR("[MP="),rf_config.txcfg,8);
	print_bits(PSTR(", M="), rf_config.txcfg,7,4);  print_hbits(PSTR(":"), rf_config.txcfg,7,4);
	print_bits(PSTR(", P="), rf_config.txcfg,2,0);  print_hbits(PSTR(":"), rf_config.txcfg,2,0);
	print(PSTR("]\n"));


}
void rf_config_set(uint8_t r, uint16_t v) {
	switch(r) {
	case 1: rf_config.config = v;	break;
	case 2: rf_config.power = v; break;
	case 3: rf_config.freq = v; break;
	case 4: rf_config.rate = v; break;
	case 5: rf_config.rcontrol = v; break;
	case 6: rf_config.dfilter = v; break;
	case 10: rf_config.afc = v; break;
	case 11: rf_config.txcfg = v; break;
	}
}

uint16_t rf_config_get(uint8_t r) {
	uint16_t cmd = 0;		
	switch(r) {
	case 1: cmd=0x8000 | (rf_config.config&0xff);	break;
	case 2: cmd=0x8200 | (rf_config.power&0xff); break;
	case 3: cmd=0xA000 | (rf_config.freq&0xfff); break;
	case 4: cmd=0xC600 | (rf_config.rate&0xff); break;
	case 5: cmd=0x9000 | (rf_config.rcontrol&0x7ff); break;
	case 6: cmd=0xC200 | (rf_config.dfilter&0xff); break;
	case 10: cmd=0xC400 | (rf_config.afc&0xff); break;
	case 11: cmd=0x9800 | (rf_config.txcfg&0x1ff); break;
	default: 
		return 0;
	}
	return cmd;	
}

void rf_config_write(uint8_t r) {
	uint16_t cmd = rf_config_get(r);	
	print_hw(PSTR("cmd "), cmd);
	uint16_t res = rf_cmd_write(cmd);
	print_hw(PSTR(":"),res);
	print_nl();
}

void rf_config_save() {
	eewr_b(RF_EE_BASE+0, rf_config.config);
	eewr_b(RF_EE_BASE+1, rf_config.power);
	eewr_w(RF_EE_BASE+2, rf_config.freq);
	eewr_b(RF_EE_BASE+4, rf_config.rate);
	eewr_w(RF_EE_BASE+5, rf_config.rcontrol);
	eewr_b(RF_EE_BASE+7, rf_config.dfilter);
	eewr_b(RF_EE_BASE+10, rf_config.afc);
	eewr_w(RF_EE_BASE+11, rf_config.txcfg);
}
#if 0
void rf_config_load(uint8_t r) {
	switch(r) {
	case 1: rf_config.config=eerd_b(RF_EE_BASE+0);	break;
	case 2: rf_config.power=eerd_b(RF_EE_BASE+1);  break;
	case 3: rf_config.freq=eerd_w(RF_EE_BASE+2); break;
	case 4: rf_config.rate=eerd_b(RF_EE_BASE+4); break;
	case 5: rf_config.rcontrol=eerd_w(RF_EE_BASE+5); break;
	case 6: rf_config.dfilter=eerd_b(RF_EE_BASE+7); break;
//	case 7: rf_config.fifoc=eerd_b(RF_EE_BASE+8); break;
//	case 8: rf_config.syncp=eerd_b(RF_EE_BASE+9); break;
	case 10: rf_config.afc=eerd_b(RF_EE_BASE+10); break;
	case 11: rf_config.txcfg=eerd_w(RF_EE_BASE+11); break;
//	case 12: rf_config.pll=eerd_b(RF_EE_BASE+13); break;
	//case 14: rf_config.wtimer=eerd_w(RF_EE_BASE+14); break;		
	//case 15: rf_config.lduty=eerd_w(RF_EE_BASE+16); break;		
	//case 16: rf_config.lbatt=eerd_b(RF_EE_BASE+18); break;
	default:
		return;	
	}

}
#endif

void rf_setup() {
        uint16_t cmd,res;

	rf_cmd_write(RF_IDLE_MODE);

	cmd = rf_config_get(1);//cfg
	res = rf_cmd_write(cmd);

	cmd = rf_config_get(3);//freq
	res = rf_cmd_write(cmd);

	cmd = rf_config_get(4);//data rate
	res = rf_cmd_write(cmd);
	
	cmd = rf_config_get(5);//receiver control
	res = rf_cmd_write(cmd);

	cmd = rf_config_get(6);//data filter
	res = rf_cmd_write(cmd);

	rf_cmd_write(0xCA81); //fifo & rst mode	
		
	cmd = rf_config_get(10); //ACF 
	res = rf_cmd_write(cmd);
		
	cmd = rf_config_get(11); //tx config
	res = rf_cmd_write(cmd);

	rf_io_com_write_only(0xE000); //wakeup timer
	rf_io_com_write_only(0xC800); //low duty 
	rf_io_com_write_only(0xC049); //low battery


}

void rf_client_hack() {
	rf_io_com_write_only(0);
	rf_io_com_write_only(0xCA81);
	rf_io_com_write_only(0xCA83);
	rf_io_com_write_only(0xB000); // fake read 
	rf_io_com_write_only(0xB000); // fake read... 
	rf_io_com_write(RF_IDLE_MODE);	
	_delay_ms(100);
	rf_io_com_write_only(0);
	_delay_ms(100);
	rf_io_com_write_only(0);
	rf_send_packet(0xff,0xff,0xff,0xff);
	rf_io_com_write_only(0);
}

uint8_t rf_do_prompt() {
	char cmd[17];
	scan_key(cmd,17);

	if (strcmp(cmd,"w")==0) {
		uint16_t i;
		i = scan_uint16();
		rf_config_write(i);
	}
	if (strcmp(cmd,"set")==0) {
		uint16_t i;
		uint16_t v;
		i = scan_uint16();
		v = scan_uint16();
		rf_config_set(i,v);
		print_ok_nl();
	}
	
	if (strcmp(cmd,"i")==0) {
		rf_config_dump();
	}
	if (strcmp(cmd,"config_save")==0) {
		rf_config_save();
		print_ok_nl();
	}
	if (strcmp(cmd,"config_init")==0) {
		rf_config_init();
		print_ok_nl();
	}

	if (strcmp(cmd,"config_reset")==0) {
		rf_config.config=0xE7;
		rf_config.power=0x39;
		rf_config.freq=0x640;
		rf_config.rate=0x02;
		rf_config.rcontrol=0x4A0;
		rf_config.dfilter=0xAC;
		rf_config.afc=0x83;
		rf_config.txcfg=0x50;
		print_ok_nl();
	}
	if (strcmp(cmd,"ci")==0) {
		rf_io_com_init();	
		print(PSTR("rf com init done\n\r"));
	}
	if (strcmp(cmd,"ct")==0) {
		uint16_t cmd;
		uint16_t r;
		cmd = scan_uint16();
		print_hw(PSTR("in: "), cmd);
		cc_start_a();
		r = rf_io_com_write(cmd);	
		cc_stop_a();
		print_hw(PSTR(" out: "), r);
		print_dw(PSTR(" ticks: "),cc_get_a());
		print_nl();

	}

	if (strcmp(cmd,"s")==0) {
		uint16_t s = rf_io_com_write(0);	
		print_hw(PSTR("rf status: "),s);
		print_bw(PSTR(" "),s);
		#ifdef AVR 
		if (PIND & _BV(2)) {
			print(PSTR(" PD2: 1"));
		} else {
			print(PSTR(" PD2: 0"));
		}
		#endif	
		print_nl();
		print(PSTR("decoded: "));
		if (s & 1<<15) { print(PSTR("RGIT/FFIT ")); }
		if (s & 1<<14) { print(PSTR("POR ")); }
		if (s & 1<<13) { print(PSTR("RGUR/FFOV ")); }
		if (s & 1<<12) { print(PSTR("WKUP ")); }
		if (s & 1<<11) { print(PSTR("EXT ")); }
		if (s & 1<<10) { print(PSTR("LBD ")); }
		if (s & 1<<9) { print(PSTR("FFEM ")); }
		if (s & 1<<8) { print(PSTR("RSSI/ATS ")); }
		if (s & 1<<7) { print(PSTR("DQD ")); }
		if (s & 1<<6) { print(PSTR("CRL ")); }
		if (s & 1<<5) { print(PSTR("ATGL ")); }
		if (s & 1<<4) { print(PSTR("OFFS<6> ")); }
		if (s & 1<<3) { print(PSTR("OFFS<3> ")); }
		if (s & 1<<2) { print(PSTR("OFFS<2> ")); }
		if (s & 1<<1) { print(PSTR("OFFS<1> ")); }
		if (s & 1) { print(PSTR("OFFS<0> ")); }
		print_nl();
	}

	if (strcmp(cmd,"scan")==0) {
		uint16_t sf=scan_uint16();
		uint16_t ef=scan_uint16();
		uint16_t step=scan_uint16();
		uint16_t loops=scan_uint16();
		
		rf_scan(sf,ef,step,loops);	
	}

	if (strcmp(cmd,"out")==0) {
		uint8_t v0 = scan_uint8();
		uint8_t v1 = scan_uint8();
		uint8_t v2 = scan_uint8();
		uint8_t v3 = scan_uint8();
		cc_start_a();
		rf_send_packet(v0,v1,v2,v3);
		cc_stop_a();
		print_dw(PSTR("packet out.... in "),cc_get_a());
		print(PSTR("ticks\n\r"));
		uint8_t c;
		for(c=0;c<10;c++) {
		print_hb(PSTR(" "), rf_packet[c]);
		}
		print_nl();
	}

	if (strcmp(cmd,"in")==0) {
		uint16_t tv = scan_uint16();
		cc_start_a();
		uint8_t rc = rf_recv_packet(tv);
		cc_stop_a();
		print_hb(PSTR("rc = "),rc);
		print_hb(PSTR(", v0 = "), rf_packet[4]);
		print_hb(PSTR(", v1 = "), rf_packet[5]);
		print_hb(PSTR(", v2 = "), rf_packet[6]);
		print_hb(PSTR(", v3 = "), rf_packet[7]);
		print_hb(PSTR(", crc = "), rf_packet[8]);
		print_dw(PSTR(" in "),cc_get_a());
		print(PSTR("ticks\n\r"));
	}

	if (strcmp(cmd,"rx_on")==0) {
		print_hw(PSTR("rx on - "), RF_RECEIVER_ON);
		rf_io_com_write(RF_RECEIVER_ON);	
		print_nl();
	}

	if (strcmp(cmd,"idle")==0) {
		print_hw(PSTR("idle - "), RF_IDLE_MODE);
		rf_io_com_write(RF_IDLE_MODE);	
		print_nl();
	}

	if (strcmp(cmd,"off")==0) {
		print_hw(PSTR("off - "), RF_SLEEP_MODE);
		rf_io_com_write(RF_SLEEP_MODE);	
		print_nl();
	}

	if (strcmp(cmd,"tx_on")==0) {
		print_hw(PSTR("tx on - "), RF_XMITTER_ON);
		rf_io_com_write(RF_XMITTER_ON);	
		print_nl();
	}

	return 1;
}




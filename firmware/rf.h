#ifndef _RF_H_
#define _RF_H_

void rf_setup();
void rf_config_init();
void rf_io_com_init();
uint8_t rf_do_prompt();
void rf_prepare_for_send();
void rf_prepare_for_recv();
void rf_send_packet(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3);
uint8_t rf_recv_packet(uint16_t tv);
void  rf_client_hack();
uint8_t rf_get_packet_v0();
uint8_t rf_get_packet_v1();
uint8_t rf_get_packet_v2();
uint8_t rf_get_packet_v3();

void rf_scan_start();
void rf_scan_stop(); 
void rf_tune_channel(uint8_t channel);
uint8_t rf_scan_get_signal_level_at_freq(uint16_t freq); 
uint16_t rf_scan_get_signal_level_at_channel(uint8_t channel);
uint8_t rf_cfg_get_channel_bw();
uint16_t rf_cfg_get_channel_f0(uint8_t channel, uint8_t bw);
uint8_t rf_cfg_get_channel_count();


#endif

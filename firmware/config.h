#ifndef __CONFIG_H__
#define __CONFIG_H__


#define RFPAD_VERSION "0.1"

#define MAX_CLIENTS 16u
#define SLOT_SIZE_IN_MS 2u // in ms
//#define POST_SLOT_DELAY_IN_US 1000u //in us


//calculated params

#define SLOTS (2u+MAX_CLIENTS+1u)
#define FIRST_CLIENT_SLOT 2u
#define FIRST_SERVER_SLOT 0u
#define SLOT_SIZE_IN_US (SLOT_SIZE_IN_MS*1000u)
//#define SLOT_RF_TIMEOUT (SLOT_SIZE_IN_MS*20u)


#endif //__CONFIG_H__

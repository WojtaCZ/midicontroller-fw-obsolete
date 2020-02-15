#include "msgDecoder.h"
#include "oled.h"

void decodeMessage(char *msg, uint32_t len){
	sprintf(oledHeader, "%s", msg);

	switch(msg[0]){
		case MSG_TYPE_INTERNAL:
			internalMessageDecoder(msg, len);
		break;

		case MSG_TYPE_BLUETOOTH:

		break;

		case MSG_TYPE_DISPLAY:

		break;
	}

	uint8_t answer[] = {0, 0, 0, 0, msg[0], 0x01};
}


void internalMessageDecoder(char *msg, uint32_t len){
	//Current source
	if(msg[1] == 0x03){
		if(msg[2] == 0x00){

		}else if(msg[2] == 0x01){

		}
	}
}



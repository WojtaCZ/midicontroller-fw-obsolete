#ifndef msgdecoder_h
#define msgdecoder_h

#include <stdio.h>
#include <stdint.h>


//Typy zprav
#define ADDRESS_MAIN 				0x02
#define ADDRESS_PC 					0x00
#define ADDRESS_CONTROLLER 			0x01

#define ADDRESS_OTHER 				0x03

#define AOKERR 						0x07
#define ERR 						0x00
#define AOK 						0x80

#define INTERNAL 					0x01
#define EXTERNAL_BT 				0x02
#define EXTERNAL_DISP 				0x03
#define EXTERNAL_MIDI				0x04
#define EXTERNAL_USB 				0x05

#define INTERNAL_COM 				0x00
#define INTERNAL_COM_KEEPALIVE 		0xAB
#define INTERNAL_COM_STOP 			0x00
#define INTERNAL_COM_PLAY 			0x01
#define INTERNAL_COM_REC 			0x02
#define INTERNAL_COM_CHECK_NAME 	0x03
#define INTERNAL_COM_GET_SONGS 		0x04
#define INTERNAL_COM_GET_TIME 		0x05
#define INTERNAL_COM_SET_TIME 		0x06

#define INTERNAL_DISP 				0x01
#define INTERNAL_DISP_GET_STATUS 	0x00
#define INTERNAL_DISP_SET_SONG	 	0x01
#define INTERNAL_DISP_SET_VERSE		0x02
#define INTERNAL_DISP_SET_LETTER 	0x03
#define INTERNAL_DISP_SET_LED	 	0x04

#define INTERNAL_USB 				0x02
#define INTERNAL_USB_GET_CONNECTED	0x00

#define INTERNAL_CURR 				0x03
#define INTERNAL_CURR_GET_STATUS	0x00
#define INTERNAL_CURR_SET_STATUS 	0x01

#define INTERNAL_BT		 			0x04
#define INTERNAL_BT_GET_STATUS		0x00

#define INTERNAL_MIDI 				0x05
#define INTERNAL_MIDI_GET_STATUS	0x00

char decoderBuffer[255];

//Internal messages
//	Periferals
//MIDI
//DISPLAY
//USB
//CURRENT SOURCE
//BLUETOOTH

//	CPU
//VERSION
//
//
//
//
//
//
//

void decodeMessage(char *msg, uint8_t broadcast);
void msgAOK(uint8_t aokType, uint8_t recType, uint16_t recSize, uint16_t dataSize, char * msg);
void msgERR(uint8_t errType, uint8_t recType, uint16_t recSize);
void sendMsg(uint8_t src, uint8_t dest, uint8_t broadcast, uint8_t type, char * msg, uint16_t len);

#endif

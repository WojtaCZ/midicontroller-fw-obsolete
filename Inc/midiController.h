#ifndef midicontroller_h
#define midicontroller_h

#include <stdint.h>
#include <oled.h>

uint32_t keyboardState, keyboardStateOld;

signed int keyboardSidePos, keyboardSidePosMax, keyboardVertPos, keyboardVertPosMax, keyboardVertPosOld;

struct menuitem songMenu[100];

uint8_t songMenuSize;
char *songs[100];
char selectedSong[40];

#define WORKER_WAITING	0x00
#define WORKER_ERR		0x01
#define WORKER_OK		0x02

struct worker{
	uint8_t assert;
	uint8_t status;
};


uint8_t alivePC, aliveRemote, alivePCCounter, aliveRemoteCounter;

//Flagy pro "workery"
struct worker workerBtScanDev, workerBtConnect, workerBtBondDev, workerBtEnterPairingKey, workerBtRemoveController, workerGetSongs;

char * workerBtConnectMAC;

void workerAssert(struct worker * wrk);
void workerDesert(struct worker * wrk);

uint32_t midiControl_checkKeyboard();
void midiController_record(uint8_t initiator, char * songname);
void midiController_play(uint8_t initiator, char * songname);
void midiController_stop(uint8_t initiator);

void midiController_init();
void midiController_display_getState();
void midiController_current_On();
void midiController_current_Off();
void midiController_keepalive_process();
uint8_t midiController_setDisplay(uint16_t cislo_pisne, uint8_t cislo_sloky, uint8_t barva, uint8_t napev);
uint8_t midiController_setDisplayRaw(uint8_t * data, uint16_t len);


struct keyboard{
	uint8_t one;
	uint8_t two;
	uint8_t three;
	uint8_t four;
	uint8_t five;
	uint8_t six;
	uint8_t seven;
	uint8_t eight;
	uint8_t nine;
	uint8_t zero;

	uint8_t mf1;
	uint8_t power;

	uint8_t up;
	uint8_t down;
	uint8_t left;
	uint8_t right;
	uint8_t enter;

	uint8_t changed;
};


struct keyboard keypad, keypadClicks;

#endif

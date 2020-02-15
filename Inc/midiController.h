#ifndef midicontroller_h
#define midicontroller_h

#include <stdint.h>

uint32_t keyboardState, keyboardStateOld;

uint32_t midiControl_checkKeyboard();

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


struct keyboard keypad;

#endif

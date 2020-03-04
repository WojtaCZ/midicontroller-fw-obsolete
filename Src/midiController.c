#include "midiController.h"
#include <stm32g0xx_hal.h>
#include "msgDecoder.h"
#include "oled.h"


void midiController_record(uint8_t initiator, char * songname){
	//Spusteno z PC
	if(initiator == ADDRESS_PC){
		oled_setDisplayedSplash(oled_recordingSplash, songname);
	}else if(initiator == ADDRESS_MAIN){
		//Spusteno z hl jednotky
		oled_setDisplayedSplash(oled_recordingSplash, songname);
	}else if(initiator == ADDRESS_CONTROLLER){
	//Spusteno z ovladace
		//Odesle se info o pisnicce
		char msg[100];
		msg[0] = INTERNAL_COM;
		msg[1] = INTERNAL_COM_REC;
		memcpy(&msg[2], songname, strlen(songname));
		sendMsg(ADDRESS_CONTROLLER, ADDRESS_OTHER, 1, INTERNAL, msg, strlen(songname)+2);
		oled_setDisplayedSplash(oled_recordingSplash, songname);
	}


}

void midiController_play(uint8_t initiator, char * songname){
	//Spusteno z PC
	if(initiator == ADDRESS_PC){
		oled_setDisplayedSplash(oled_playingSplash, songname);
	}else if(initiator == ADDRESS_MAIN){
		//Spusteno z hl jednotky
		oled_setDisplayedSplash(oled_playingSplash, songname);
	}else if(initiator == ADDRESS_CONTROLLER){
	//Spusteno z ovladace
		char msg[100];
		msg[0] = INTERNAL_COM;
		msg[1] = INTERNAL_COM_PLAY;
		memcpy(&msg[2], songname, strlen(songname));
		sendMsg(ADDRESS_CONTROLLER, ADDRESS_OTHER, 1, INTERNAL, msg, strlen(songname)+2);
		oled_setDisplayedSplash(oled_playingSplash, songname);
	}


}

void midiController_stop(uint8_t initiator){
	if(initiator == ADDRESS_CONTROLLER){
			char msg[5];
			msg[0] = INTERNAL_COM;
			msg[1] = INTERNAL_COM_STOP;
			sendMsg(ADDRESS_CONTROLLER, ADDRESS_OTHER, 1, INTERNAL, msg,3);
		}else{
			oledType = OLED_MENU;
		}

}

uint32_t midiControl_checkKeyboard(){

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

	keyboardState = 0;

	keyboardState |= ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0))) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1))) << 1) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2))) << 2));

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);

	keyboardState |= ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0))) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1)) << 1)) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2)) << 2))) << 3;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);

	keyboardState |= ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0))) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1)) << 1)) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2)) << 2))) << 6;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);

	keyboardState |= ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0))) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1)) << 1)) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2)) << 2))) << 9;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

	keyboardState |= ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0))) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1)) << 1)) | ((!(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2)) << 2))) << 12;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

	keyboardState |= (/*(!(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14))) | */(((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)) << 1))) << 15;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

	if(keyboardState != keyboardStateOld){
		keypad.one = (keyboardState & 0x4) >> 2;
		keypad.two = (keyboardState & 0x2) >> 1;
		keypad.three = (keyboardState & 0x1);

		keypad.four = ((keyboardState >> 3) & 0x4) >> 2;
		keypad.five = ((keyboardState >> 3) & 0x2) >> 1;
		keypad.six = ((keyboardState >> 3) & 0x1);

		keypad.seven = ((keyboardState >> 6) & 0x4) >> 2;
		keypad.eight = ((keyboardState >> 6) & 0x2) >> 1;
		keypad.nine = ((keyboardState >> 6) & 0x1);

		keypad.mf1 = ((keyboardState >> 9) & 0x4) >> 2;
		keypad.zero = ((keyboardState >> 9) & 0x2) >> 1;
		keypad.power = ((keyboardState >> 12) & 0x10) >> 4;

		keypad.up = ((keyboardState >> 9) & 0x1);
		keypad.down = ((keyboardState >> 12) & 0x4) >> 2;
		keypad.left = ((keyboardState >> 12) & 0x2) >> 1;
		keypad.right = ((keyboardState >> 12) & 0x1);
		//keypad.enter |= (((keyboardState >> 12) & 0x8) >> 3) & 0x01;

		keypad.changed = 1;

		keyboardStateOld = keyboardState;
	}


	return keyboardState;
}

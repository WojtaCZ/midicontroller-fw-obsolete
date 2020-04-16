#include "midiController.h"
#include <stm32g0xx_hal.h>
#include "msgDecoder.h"
#include "oled.h"


//Funkce pro jednodussi ovladani workeru - aktivace
void workerAssert(struct worker * wrk){
	wrk->assert = 1;
	wrk->status = WORKER_WAITING;
}

//Funkce pro jednodussi ovladani workeru - deaktivace
void workerDesert(struct worker * wrk){
	wrk->assert = 0;
	wrk->status = WORKER_WAITING;
}

//Funkce pro vytvoreni menu pisni ze stringu vraceneho z PC
void strToSongMenu(char * str, uint8_t * size){

	//Spocita pocet pisni
	uint8_t items = countOccurances(str, "\n");
	*size = items;
	splitString(str, "\n", songs);

	//Projde je a pro kazdou vytvori zaznam v menu
	for(int i = 0; i <= items; i++){
			songMenu[i].name = songs[i];
			songMenu[i].font = &Font_11x18;
			songMenu[i].selected = 0;
			songMenu[i].hasSpecialSelector = 0;
			songMenu[i].specharNotSelected = 0;
			songMenu[i].specharSelected = 0;
			songMenu[i].submenuLevel = 3;
			songMenu[i].parentItem = &bluetoothmenu[0].name;
		}

	//Vytvori tlacitko zpet
	songMenu[items+1].font = &Font_11x18;
	songMenu[items+1].name = "Zpet";
	songMenu[items+1].selected = 0;
	songMenu[items+1].hasSpecialSelector = 1;
	songMenu[items+1].specharNotSelected = 36;
	songMenu[items+1].specharSelected = 37;
	songMenu[items+1].submenuLevel = 3;
	songMenu[items+1].parentItem = 0;

}



void midiController_init(){
	//Inicializace promennych pro chod zarizeni
	alivePC = 0;
	aliveMain = 0;
	alivePCCounter = 0;
	aliveMainCounter = 0;
	btDataIcon = -1;
	workerDesert(&workerBtRemoveController);
}

//Rutina pro kontrolu stavu displeje a zobrazeni na LED
void midiController_display_getState(){

}

//Rutina pro zapnuti proudoveho zdroje
void midiController_current_On(){
	char msg[2];
	msg[0] = INTERNAL_CURR;
	msg[1] = INTERNAL_CURR_ON;
	sendMsg(ADDRESS_CONTROLLER, ADDRESS_MAIN, 0, INTERNAL, msg, 2);
}

//Rutina pro vypnuti proudoveho zdroje
void midiController_current_Off(){
	char msg[2];
	msg[0] = INTERNAL_CURR;
	msg[1] = INTERNAL_CURR_OFF;
	sendMsg(ADDRESS_CONTROLLER, ADDRESS_MAIN, 0, INTERNAL, msg, 2);
}


//Rutina pro spusteni nahravani
void midiController_record(uint8_t initiator, char * songname){
	//Spusteno z PC
	if(initiator == ADDRESS_PC){
		//Jen se zobrazi obrazovka nahravani
		oled_setDisplayedSplash(oled_recordingSplash, songname);
	}else if(initiator == ADDRESS_CONTROLLER){
	//Spusteno ovladacem
		//Jen se zobrazi obrazovka nahravani
		oled_setDisplayedSplash(oled_recordingSplash, songname);
	}else if(initiator == ADDRESS_MAIN){
	//Spusteno ze zakladnove stanice
		//Posle se zprava do PC aby zacalo nahravat
		char msg[100];
		msg[0] = INTERNAL_COM;
		msg[1] = INTERNAL_COM_REC;
		memcpy(&msg[2], songname, strlen(songname));
		sendMsg(ADDRESS_MAIN, ADDRESS_PC, 0, INTERNAL, msg, strlen(songname)+2);
	}


}

void midiController_play(uint8_t initiator, char * songname){
	//Spusteno z PC
	if(initiator == 0x00){
		memset(selectedSong, 0, 40);
		sprintf(selectedSong, "%s", songname);
		//Jen se zobrazi obrazovka prehravani
		oled_setDisplayedSplash(oled_playingSplash, songname);
	}else if(initiator == 0x01){
	//Spusteno ovladacem
		//Jen se zobrazi obrazovka prehravani
		oled_setDisplayedSplash(oled_playingSplash, songname);
	}else if(initiator == 0x02){
	//Spusteno ze zakladnove stanice
		//Posle se zprava do PC aby zacalo prehravat
		char msg[100];
		msg[0] = INTERNAL_COM;
		msg[1] = INTERNAL_COM_PLAY;
		memcpy(&msg[2], songname, strlen(songname));
		sendMsg(ADDRESS_MAIN, ADDRESS_PC, 0, INTERNAL, msg, strlen(songname)+2);
	}


}

void midiController_stop(uint8_t initiator){
	//Spusteno z hlavni jednotky
	if(initiator == ADDRESS_MAIN){
		//Posle se zprava do PC o zastaveni
		char msg[2] = {INTERNAL_COM, INTERNAL_COM_STOP};
		sendMsg(ADDRESS_MAIN, ADDRESS_PC, 0, INTERNAL, msg, 2);
	}else{
		//Vrati se do menu, zapne OLED refresh a vypne LED
		oledType = OLED_MENU;
		oled_refreshResume();
	}

}

uint32_t midiController_checkKeyboard(){

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

//Rutina pro kontrolu pripojeni PC a ovladace a odesilani info o "zijici" hlavni jednotce
midiController_keepalive_process(){
	//Pricita citace - jak dlouho nedostal odpoved
	alivePCCounter++;
	aliveMainCounter++;

	//Pokud nedostal odpoved za 2s, neni PC pripojeno
	if(alivePCCounter > 6){
		alivePC = 0;
		alivePCCounter = 0;
	}

	//Pokud nedostal odpoved za 2s, neni ovladac pripojen
	if(aliveMainCounter > 6){
		aliveMain = 0;
		aliveMainCounter = 0;
	}


}

//Rutina pro nastaveni dat zobrazenych na displeji
uint8_t midiController_setDisplay(uint16_t cislo_pisne, uint8_t cislo_sloky, uint8_t barva, uint8_t napev){
	uint8_t data[9];
	//Asi nejaky kontrolni znak, vzdy stejny
	data[0] = 176;
	//Jednotky sloky
	data[1] = cislo_sloky-(uint8_t)(cislo_sloky/10)*10;
	//Desitky sloky
	data[2] = (uint8_t)(cislo_sloky/10);
	//Jednotky pisne
	data[3] = cislo_pisne - (uint8_t)(cislo_pisne/10)*10;
	//Desitky pisne
	data[4] = (uint8_t)(cislo_pisne/10) - (uint8_t)(cislo_pisne/100);
	//Stovky pisne
	data[5] = (uint8_t)(cislo_pisne/100) - (uint8_t)(cislo_pisne/1000);
	//Tisice pisne
	data[6] = (uint8_t)(cislo_pisne/1000);
	//Pismeno
	data[7] = napev-55;
	//Barva
	data[8] = barva;

	midiController_setDisplayRaw(data, 9);
}

uint8_t midiController_setDisplayRaw(uint8_t * data, uint16_t len){

}

void midiControl_get_time(){
	//Odesle zadost o nastaveni casu
	char msg[] = {INTERNAL_COM, INTERNAL_COM_GET_TIME};
	sendMsg(ADDRESS_CONTROLLER, ADDRESS_PC, 0, INTERNAL, msg, 2);
}

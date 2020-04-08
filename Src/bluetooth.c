#include "bluetooth.h"
#include <stm32g0xx_hal.h>
#include <main.h>
#include <stm32g0xx_hal_uart.h>
#include <usart.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "midiController.h"

extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

//ZADOST PRIPOJENI
//AOK{0D}{0A}CMD> %CONNECT,1,699238C2F7D5%%KEY:123456%%CONN_PARAM,0006,0000,01F4%1,699238C2F7D5,1{0D}{0A}END

uint8_t bluetoothInit(){
	btFifoIndex = 0;
	btMsgFifoIndex = 0;
	btCmdMode = 1;
	btStatusMsg = 0;
	btStreamOpen = 0;
	btNullCounter = 0;
	btComMessageFlag = 0;
	btComMessageSizeFlag = 0;
	btMessageLen = 0;
	btMsgReceivedFlag = 0;
	workerDesert(&workerBtEnterPairingKey);
	workerDesert(&workerBtBondDev);
	workerDesert(&workerBtScanDev);

	//Zecne se prijem
	HAL_UART_Receive_IT(&huart2, &btFifoByte, 1);

	HAL_GPIO_WritePin(BT_RST_GPIO_Port, BT_RST_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BT_MODE_GPIO_Port, BT_MODE_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(BT_RST_GPIO_Port, BT_RST_Pin, GPIO_PIN_SET);

	HAL_Delay(100);

	//Zapne se CMD
	if(!bluetoothEnterCMD()) return 0;

	//Dev info a UART
	if(!bluetoothCMD_ACK("GS\r", "C0")){
		if(!bluetoothCMD_ACK("SS,C0\r", BT_AOK)) return 0;
		if(!bluetoothCMD_ACK("R,1\r", "REBOOT")) return 0;
	}

	//Zkontroluje se název
	if(!bluetoothCMD_ACK("GN\r", "MIDIController")){
		if(!bluetoothCMD_ACK("SN,MIDIController\r", BT_AOK)) return 0;
	}

	//Nastavi se vyzarovaci vykon
	if(!bluetoothCMD_ACK("SGA,0\r", BT_AOK)) return 0;
	if(!bluetoothCMD_ACK("SGC,0\r", BT_AOK)) return 0;

	//Apperance jako Remote
	if(!bluetoothCMD_ACK("SDA,0180\r", BT_AOK)) return 0;

	//Vyrobce
	if(!bluetoothCMD_ACK("SDN,Vojtech Vosahlo\r", BT_AOK)) return 0;

	//Automaticky potvrdi pin
	if(!bluetoothCMD_ACK("SA,5\r", BT_AOK)) return 0;

	//Vypne CMD
	if(!bluetoothLeaveCMD()) return 0;


	return 1;
}


uint8_t bluetoothDecodeMsg(){
	char * index = 0;
	if(strstr((char *)btMsgFifo, "%BONDED") != 0){
		btStreamOpen = 1;
	}

	if(strstr((char *)btMsgFifo, "%CONNECT") != 0){
		index = strstr((char *)btMsgFifo, "%CONNECT");
		sscanf((char *)index+9, "%*d,%02X%02X%02X%02X%02X%02X", &btPairReq.mac[0], &btPairReq.mac[1], &btPairReq.mac[2], &btPairReq.mac[3], &btPairReq.mac[4], &btPairReq.mac[5]);
		sprintf(btPairReq.name, "%02X-%02X-%02X-%02X-%02X-%02X", btPairReq.mac[0], btPairReq.mac[1], btPairReq.mac[2], btPairReq.mac[3], btPairReq.mac[4], btPairReq.mac[5]);
	}

	if(strstr((char *)btMsgFifo, "%DISCONNECT") != 0){
		oledType = OLED_MENU;
		btStreamOpen = 0;
	}

	if(strstr((char *)btMsgFifo, "%KEY:") != 0){
		index = strstr((char *)btMsgFifo, "%KEY:");
		sscanf((char *)index+5, "%06ld", &btPairReq.pin);
		oled_setDisplayedSplash(oled_BtDevPairRequestSplash, &btPairReq);
		//sprintf(oledHeader, "%s", index+5);
	}

	if(strstr((char *)btMsgFifo, "%STREAM_OPEN") != 0){
		btStreamOpen = 1;
	}

	if(strstr((char *)btMsgFifo, "%SECURED") != 0){
		btStreamOpen = 1;
	}

	if(strstr((char *)btMsgFifo, "%KEY_REQ") != 0){
		btPairReq.pin = 0;
		oled_setDisplayedSplash(oled_BtDevKeyRequestSplash, &btPairReq);
	}

	bluetoothMsgFifoFlush();

	return 1;
}
uint8_t bluetooth_refreshSelfInfo(){

	//Neimplementovane

	return 1;
}


//Vymazani bt bufferu
void bluetoothFifoFlush(){
	memset(btFifo, 0, btFifoIndex);
	btFifoIndex = 0;
}

//Vymazani bufferu pro bt zpravy
void bluetoothMsgFifoFlush(){
	memset(btMsgFifo, 0, btMsgFifoIndex);
	btMsgFifoIndex = 0;
}

//Zapnuti CMD modu modulu
uint8_t bluetoothEnterCMD(){
	if(!bluetoothCMD_ACK("$$$", "CMD>")) return 0;
	btCmdMode = 1;
	return 1;
}

//Vypnuti CMD modu modulu
uint8_t bluetoothLeaveCMD(){
	if(!bluetoothCMD_ACK("---\r", BT_END)) return 0;
	btCmdMode = 0;
	return 1;
}



//Rutina pro odeslani prikazu a kontrolu odpovedi v podobe stringu ale bez zaznamenani odpovedi
uint8_t bluetoothCMD_ACK(char *cmd, char *ack){

	//Vycisti se buffer
	bluetoothFifoFlush();

	//Odesle se prikaz
	if(strlen(cmd) > 0){
		if(HAL_UART_Transmit_IT(&huart2, (uint8_t*)cmd, strlen(cmd)) != HAL_OK) return 0;
	}

	//Ceka se nez se vrati vyzadana odpoved nebo nez nastane timeout
	uint32_t now = HAL_GetTick();
	while(strstr((char *)btFifo, ack) == 0 && HAL_GetTick() - now < BT_TIMEOUT);

	//Odpoved se zkopiruje
	memcpy(btMsgFifo, btFifo, btFifoIndex);
	btMsgFifoIndex = btFifoIndex;
	//Radeji se dekoduje, pokud do ni prisel napriklad jeste nejaky prikaz
	bluetoothDecodeMsg();
	//Vycisti se buffer
	bluetoothFifoFlush();

	//Pokud byl timeout, vrati 0
	if(HAL_GetTick() - now >= BT_TIMEOUT) return 0;

	return 1;

}

//Rutina pro odeslani prikazu a zastaveni na terminatoru, vrati zpet i text odpovedi
uint8_t bluetoothCMD_Until(char *cmd, char *terminator, char (*recvBuffer)[]){

	//Vycisti se buffer
	bluetoothFifoFlush();

	//Odesle se prikaz
	if(strlen(cmd) > 0){
		if(HAL_UART_Transmit_IT(&huart2, (uint8_t*)cmd, strlen(cmd)) != HAL_OK) return 0;
	}

	//Ceka se nez se vrati terminator nebo nez nastane timeout
	uint32_t now = HAL_GetTick();
	while(strstr((char *)btFifo, terminator) == 0 && HAL_GetTick() - now < BT_TIMEOUT);

	//Odpoved se okopiruje do bufferu na vraceni
	memcpy(recvBuffer, (char *)btFifo, btFifoIndex-1);

	//Vycisti se buffer
	bluetoothFifoFlush();

	//Pokud byl timeout, vrati 0
	if(HAL_GetTick() - now >= BT_TIMEOUT) return 0;

	return 1;
}

//Rutina pro odeslani prikazu a zaznamenavani odpovedi po nejaky cas
uint8_t bluetoothCMD_Time(char *cmd, uint8_t s, char (*recvBuffer)[]){

	//Vycisti se buffer
	bluetoothFifoFlush();

	//Odesle se prikaz
	if(strlen(cmd) > 0){
		if(HAL_UART_Transmit_IT(&huart2, (uint8_t*)cmd, strlen(cmd)) != HAL_OK) return 0;
	}

	//Pocka se zadany cas
	uint32_t now = HAL_GetTick();

	while(HAL_GetTick() - now < (s*1000));

	//Odpoved se okopiruje do bufferu na vraceni
	memcpy(recvBuffer, (char *)btFifo, btFifoIndex-1);

	//Vycisti se buffer
	bluetoothFifoFlush();

	return 1;
}

//Rutina pro pripojeni sparovanych zarizeni
uint8_t bluetoothConnectKnown(){
	//Oskenuji se zarizeni v okoli
	bluetoothGetScannedDevices();
	//Ziska se seznam sparovanych zarizeni
	bluetoothGetBondedDevices();

	int8_t match = 0;
	int8_t selected = -1;

	//Najde se shoda v parovanych a skenovanych
	for(uint8_t b = 0; b < btBondedCount; b++){
		for(uint8_t s = 0; s < btScannedCount; s++){
			match = 0;
			for(uint8_t i = 0; i < 6; i++){
				if(btScanned[s].mac[i] == btBonded[b].mac[i]){
					match++;
				}

				if(match == 6){
					selected = b;
					break;
				}
			}
		}
	}

	//Pokud je shoda
	if(selected != -1){
		//Okopiruje se MAC adresa
		char * mac = (char*) malloc(20);
		sprintf(mac, "%02X%02X%02X%02X%02X%02X", btBonded[match].mac[0], btBonded[match].mac[1], btBonded[match].mac[2], btBonded[match].mac[3], btBonded[match].mac[4], btBonded[match].mac[5]);
		sprintf(oledHeader, "%s" ,mac);
		//Pokusi se pripojit k MAC
		if(!bluetoothConnect(mac)) return 0;
	}else return 0;

	//Odejde z CMD modu
	bluetoothLeaveCMD();

	return 1;
}


//Rutina pro oskenovani zarizeni v okoli a vytvoreni menu
uint8_t bluetoothGetScannedDevices(){

	char buff[300];
	memset(buff, 0, 300);

	//Vstoupi do CMD modu
	if(!bluetoothEnterCMD()) return 0;

	//Skenuje 15s
	if(!bluetoothCMD_Time("F\r", 30, &buff)){
		if(!bluetoothLeaveCMD()) return 0;
		return 0;
	}

	char *devices[20];

	//Spocita pocet zarizeni a rozdeli vracene stringy na jednotliva zarizeni
	btScannedCount = countOccurances(buff, "%")/2;
	replacechar(buff, '\r', ' ');
	splitString(buff, "\n", devices);

	//Pokud nasel zarizeni v okoli
	if(btScannedCount != 0){
		uint8_t i;

		//Projde seznam a z ruznych typu odpovedi vycucne data
		for(i = 0; i < btScannedCount; i++){
			//Nactou se informace do seznamu zarizeni
			if(strstr((char *)devices[i+1], ",,") - (char *)devices[i+1] < 16 && strstr((char *)devices[i+1], ",,")){
				//Pokud prijde response kde neni jmeno
				sscanf((char *)devices[i+1], "%*[%]%02X%02X%02X%02X%02X%02X,%d,,%[^,],%02X", &btScanned[i].mac[0], &btScanned[i].mac[1], &btScanned[i].mac[2], &btScanned[i].mac[3], &btScanned[i].mac[4], &btScanned[i].mac[5], &btScanned[i].mactype, btScanned[i].uuid, &btScanned[i].rssi);
				sprintf(btScanned[i].name, "%02X-%02X-%02X-%02X-%02X-%02X", btScanned[i].mac[0], btScanned[i].mac[1], btScanned[i].mac[2], btScanned[i].mac[3], btScanned[i].mac[4], btScanned[i].mac[5]);
			}else if(strstr((char *)devices[i+1], ",,") - (char *)devices[i+1] >= 16){
				//Pokud prijde response bez uuid
				sscanf((char *)devices[i+1], "%*[%]%02X%02X%02X%02X%02X%02X,%d,%[^,],,%02X", &btScanned[i].mac[0], &btScanned[i].mac[1], &btScanned[i].mac[2], &btScanned[i].mac[3], &btScanned[i].mac[4], &btScanned[i].mac[5], &btScanned[i].mactype, btScanned[i].name, &btScanned[i].rssi);
			}else if(strstr((char *)devices[i+1], "Brcst") - (char *)devices[i+1] >= 16){
				//Pokud prijde response s broadcastem
				sscanf((char *)devices[i+1], "%*[%]%02X%02X%02X%02X%02X%02X,%d,%02X,%*[^%]", &btScanned[i].mac[0], &btScanned[i].mac[1], &btScanned[i].mac[2], &btScanned[i].mac[3], &btScanned[i].mac[4], &btScanned[i].mac[5], &btScanned[i].mactype, &btScanned[i].rssi);
				sprintf(btScanned[i].name, "%02X-%02X-%02X-%02X-%02X-%02X", btScanned[i].mac[0], btScanned[i].mac[1], btScanned[i].mac[2], btScanned[i].mac[3], btScanned[i].mac[4], btScanned[i].mac[5]);
			}else{
				//Pokud prijde normalni response
				sscanf((char *)devices[i+1], "%*[%]%02X%02X%02X%02X%02X%02X,%d,%[^,],%[^,],%02X", &btScanned[i].mac[0], &btScanned[i].mac[1], &btScanned[i].mac[2], &btScanned[i].mac[3], &btScanned[i].mac[4], &btScanned[i].mac[5], &btScanned[i].mactype, btScanned[i].name, btScanned[i].uuid, &btScanned[i].rssi);
			}

			//Z vyccnutych dat vytvori item v menu
			btScanedDevices[i].font = &Font_11x18;
			btScanedDevices[i].name = btScanned[i].name;
			btScanedDevices[i].selected = 0;
			btScanedDevices[i].hasSpecialSelector = 0;
			btScanedDevices[i].specharNotSelected = 0;
			btScanedDevices[i].specharSelected = 0;
			btScanedDevices[i].submenuLevel = 3;
			btScanedDevices[i].parentItem = &bluetoothmenu[0].name;

		}

		//Vytvori tlacitko zpet
		btScanedDevices[btScannedCount].font = &Font_11x18;
		btScanedDevices[btScannedCount].name = "Zpet";
		btScanedDevices[btScannedCount].selected = 0;
		btScanedDevices[btScannedCount].hasSpecialSelector = 1;
		btScanedDevices[btScannedCount].specharNotSelected = 36;
		btScanedDevices[btScannedCount].specharSelected = 37;
		btScanedDevices[btScannedCount].submenuLevel = 3;
		btScanedDevices[btScannedCount].parentItem = 0;

	}

	//Vypne skenovani
	if(!bluetoothCMD_Until("X\r", BT_CMD, &buff)){
		if(!bluetoothLeaveCMD()) return 0;
		return 0;
	}

	//Odejde z CMD modu
	if(!bluetoothLeaveCMD()) return 0;

	//Pokud nic nenaskenoval, vrati 0
	if(btScannedCount == 0) return 0;

	return 1;

}

//Rutina pro ziskani sparovanych zarizeni
uint8_t bluetoothGetBondedDevices(){

	char buff[300];
	memset(buff, 0, 300);

	//Vstoupi do CMD modu
	if(!bluetoothEnterCMD()) return 0;

	//Nacte seznam sparovanych zarizeni
	if(!bluetoothCMD_Until("LB\r", BT_END, &buff)){
		if(!bluetoothLeaveCMD()) return 0;
		return 0;
	}

	char *devices[20];

	//Spocita jejich pocet a rozdeli je na jednotliva zarizeni
	btBondedCount = countOccurances(buff, "\n");
	replacechar(buff, '\r', ' ');
	splitString(buff, "\n", devices);

	//Pokud nejaka nasel
	if(btBondedCount != 0){

		uint8_t i;

		//Projde zaznamy a vycucne informace
		for(i = 0; i < btBondedCount; i++){
			//Nactou se informace do seznamu zarizeni
			sscanf((char *)devices[i], "%*d,%02X%02X%02X%02X%02X%02X,%d", &btBonded[i].mac[0], &btBonded[i].mac[1], &btBonded[i].mac[2], &btBonded[i].mac[3], &btBonded[i].mac[4], &btBonded[i].mac[5], &btBonded[i].mactype);
			sprintf(btBonded[i].name, "%02X-%02X-%02X-%02X-%02X-%02X", btBonded[i].mac[0], btBonded[i].mac[1], btBonded[i].mac[2], btBonded[i].mac[3], btBonded[i].mac[4], btBonded[i].mac[5]);

			//Vytvori item v menu
			btBondedDevicesMenu[i].font = &Font_11x18;
			btBondedDevicesMenu[i].name = btBonded[i].name;
			btBondedDevicesMenu[i].selected = 0;
			btBondedDevicesMenu[i].hasSpecialSelector = 0;
			btBondedDevicesMenu[i].specharNotSelected = 0;
			btBondedDevicesMenu[i].specharSelected = 0;
			btBondedDevicesMenu[i].submenuLevel = 3;
			btBondedDevicesMenu[i].parentItem = &bluetoothmenu[0].name;


		}

		//Vytvori tlacitko zpet
		btBondedDevicesMenu[btBondedCount].font = &Font_11x18;
		btBondedDevicesMenu[btBondedCount].name = "Zpet";
		btBondedDevicesMenu[btBondedCount].selected = 0;
		btBondedDevicesMenu[btBondedCount].hasSpecialSelector = 1;
		btBondedDevicesMenu[btBondedCount].specharNotSelected = 36;
		btBondedDevicesMenu[btBondedCount].specharSelected = 37;
		btBondedDevicesMenu[btBondedCount].submenuLevel = 3;
		btBondedDevicesMenu[btBondedCount].parentItem = 0;
	}

	//Odejde z CMD modu
	if(!bluetoothLeaveCMD()) return 0;

	//Pokud nenasel zadna zarizeni, vrati 0
	if(btBondedCount == 0) return 0;

	return 1;

}

//Rutina pro ziskani zarizeni pro parovani (stejne jako skenovani akorat cpe do jineho menu)
uint8_t bluetoothGetBondableDevices(){

	char buff[300];
	memset(buff, 0, 300);

	//Vstoupi do CMD modu
	if(!bluetoothEnterCMD()) return 0;

	//Skenuje 15s
	if(!bluetoothCMD_Time("F\r", 30, &buff)){
		if(!bluetoothLeaveCMD()) return 0;
		return 0;
	}

	char *devices[20];

	//Spocita pocet zarizeni a rozdeli vracene stringy na jednotliva zarizeni
	btBondableCount = countOccurances(buff, "%")/2;
	replacechar(buff, '\r', ' ');
	splitString(buff, "\n", devices);

	//Pokud nasel zarizeni v okoli
	if(btBondableCount != 0){
		uint8_t i;

		//Projde seznam a z ruznych typu odpovedi vycucne data
		for(i = 0; i < btBondableCount; i++){

			//Nactou se informace do seznamu zarizeni
			if(strstr((char *)devices[i+1], ",,") - (char *)devices[i+1] < 16 && strstr((char *)devices[i+1], ",,")){
				//Pokud prijde response kde neni jmeno
				sscanf((char *)devices[i+1], "%*[%]%02X%02X%02X%02X%02X%02X,%d,,%[^,],%02X", &btBondable[i].mac[0], &btBondable[i].mac[1], &btBondable[i].mac[2], &btBondable[i].mac[3], &btBondable[i].mac[4], &btBondable[i].mac[5], &btBondable[i].mactype, btBondable[i].uuid, &btBondable[i].rssi);
				sprintf(btBondable[i].name, "%02X-%02X-%02X-%02X-%02X-%02X", btBondable[i].mac[0], btBondable[i].mac[1], btBondable[i].mac[2], btBondable[i].mac[3], btBondable[i].mac[4], btBondable[i].mac[5]);
			}else if(strstr((char *)devices[i+1], ",,") - (char *)devices[i+1] >= 16){
				//Pokud prijde response bez uuid
				sscanf((char *)devices[i+1], "%*[%]%02X%02X%02X%02X%02X%02X,%d,%[^,],,%02X", &btBondable[i].mac[0], &btBondable[i].mac[1], &btBondable[i].mac[2], &btBondable[i].mac[3], &btBondable[i].mac[4], &btBondable[i].mac[5], &btBondable[i].mactype, btBondable[i].name, &btBondable[i].rssi);
			}else if(strstr((char *)devices[i+1], "Brcst") - (char *)devices[i+1] >= 16){
				//Pokud prijde response s broadcastem
				sscanf((char *)devices[i+1], "%*[%]%02X%02X%02X%02X%02X%02X,%d,%02X,%*[^%]", &btBondable[i].mac[0], &btBondable[i].mac[1], &btBondable[i].mac[2], &btBondable[i].mac[3], &btBondable[i].mac[4], &btBondable[i].mac[5], &btBondable[i].mactype, &btBondable[i].rssi);
				sprintf(btBondable[i].name, "%02X-%02X-%02X-%02X-%02X-%02X", btBondable[i].mac[0], btBondable[i].mac[1], btBondable[i].mac[2], btBondable[i].mac[3], btBondable[i].mac[4], btBondable[i].mac[5]);
			}else{
				//Pokud prijde normalni response
				sscanf((char *)devices[i+1], "%*[%]%02X%02X%02X%02X%02X%02X,%d,%[^,],%[^,],%02X", &btBondable[i].mac[0], &btBondable[i].mac[1], &btBondable[i].mac[2], &btBondable[i].mac[3], &btBondable[i].mac[4], &btBondable[i].mac[5], &btBondable[i].mactype, btBondable[i].name, btBondable[i].uuid, &btBondable[i].rssi);
			}

			//Vytvori item v menu
			btBondableDevices[i].font = &Font_11x18;
			btBondableDevices[i].name = btBondable[i].name;
			btBondableDevices[i].selected = 0;
			btBondableDevices[i].hasSpecialSelector = 0;
			btBondableDevices[i].specharNotSelected = 0;
			btBondableDevices[i].specharSelected = 0;
			btBondableDevices[i].submenuLevel = 3;
			btBondableDevices[i].parentItem = &bluetoothmenu[0].name;


		}

		//Vytvori tacitko zpet
		btBondableDevices[btBondableCount].font = &Font_11x18;
		btBondableDevices[btBondableCount].name = "Zpet";
		btBondableDevices[btBondableCount].selected = 0;
		btBondableDevices[btBondableCount].hasSpecialSelector = 1;
		btBondableDevices[btBondableCount].specharNotSelected = 36;
		btBondableDevices[btBondableCount].specharSelected = 37;
		btBondableDevices[btBondableCount].submenuLevel = 3;
		btBondableDevices[btBondableCount].parentItem = 0;

	}

	//Ukonci skenovani
	if(!bluetoothCMD_Until("X\r", BT_CMD, &buff)){
		if(!bluetoothLeaveCMD()) return 0;
		return 0;
	}

	//Odejde z CMD modu
	if(!bluetoothLeaveCMD()) return 0;

	//Pokud nenasel zarizeni, vrati 0
	if(btBondableCount == 0) return 0;

	return 1;
}

//Rutina pro pripojeni k zarizeni
uint8_t bluetoothConnect(char * mac){
	//Vstoupi do CMD modu
	if(!btCmdMode) bluetoothEnterCMD();
	//Okopiruje si MAC
	char * cmd = (char *) malloc(50);
	//Vysle prikaz pro pripojeni
	sprintf(cmd, "C,0,%s\r", mac);
	sprintf(oledHeader, "%s" ,cmd);
	//Pokud se pripojeni nepovedlo, vrati 0
	if(!bluetoothCMD_ACK(cmd, "%STREAM_OPEN")){
		//Odejde z CMD modu
		bluetoothLeaveCMD();
		return 0;
	}else btStreamOpen = 1;

	//Odejde z CMD modu
	//bluetoothLeaveCMD();

	return 1;
}

uint8_t bluetoothBond(){
	//Vstoupi do CMD modu
	if(!btCmdMode) bluetoothEnterCMD();

	//Odesle parovaci prikaz
	if(!bluetoothCMD_ACK("B\r", "%BONDED")){
		//Pokud se nepovedlo, odejde z CMD modu
		bluetoothLeaveCMD();
		//Vrati 0
		return 0;
	}else btStreamOpen = 1;

	//Odejde z CMD modu
	bluetoothLeaveCMD();

	return 1;
}

//Rutina pro pocitani vyskytu v textu
uint32_t countOccurances(char * buff, char * what){
	uint32_t count = 0;
	const char * tmp = buff;
	while((tmp = strstr(tmp, what)))
	{
		count++;
		tmp++;
	}

	return count;
}

//Rutina pro nahrazeni charakteru v textu
uint32_t replacechar(char *str, char orig, char rep){
    char *ix = str;
    uint32_t n = 0;
    while((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
        n++;
    }
    return n;
}

//Rutina pro rozdeleni stringu na substringy
uint32_t splitString(char * string, char * delim, char ** array){

	uint32_t count = 0;
	char * token = strtok(string, delim);
	array[count++] = token;

	while( token != NULL ) {
	      token = strtok(NULL, delim);
	      array[count++] = token;
	}

	return count;
}

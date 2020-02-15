#include "bluetooth.h"

#include <stm32g0xx_hal.h>
#include <main.h>
#include <stm32g0xx_hal_uart.h>
#include <usart.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//ZADOST PRIPOJENI
//AOK{0D}{0A}CMD> %CONNECT,1,699238C2F7D5%%KEY:123456%%CONN_PARAM,0006,0000,01F4%1,699238C2F7D5,1{0D}{0A}END

uint8_t bluetoothInit(){

	if(HAL_UART_Receive_DMA(&huart2, btRxBuff, 8) != HAL_OK) return 0;
	HAL_GPIO_WritePin(BT_RST_GPIO_Port, BT_RST_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BT_MODE_GPIO_Port, BT_MODE_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(BT_RST_GPIO_Port, BT_RST_Pin, GPIO_PIN_SET);

	HAL_Delay(100);

	while(HAL_DMA_GetState(&hdma_usart2_rx) == HAL_DMA_STATE_BUSY);
	if(strcmp("%REBOOT%", (char *)btRxBuff) != 0) return 0;
	HAL_UART_DMAStop(&huart2);


	//Zapne se CMD
	if(!bluetoothCMD_ACK("$$$", "CMD> ")) return 0;

	//Zkontroluje se n�zev
	if(!bluetoothCMD_ACK("GN\r", "MIDIController\r\nCMD> ")){
		if(!bluetoothCMD_ACK("SN,MIDIController\r", BT_AOK)) return 0;
	}

	//Nastavi se vyzarovaci vykon
	if(!bluetoothCMD_ACK("SGA,0\r", BT_AOK)) return 0;
	if(!bluetoothCMD_ACK("SGC,0\r", BT_AOK)) return 0;

	//bluetoothGetScannedDevices();

	if(!bluetoothCMD_ACK("SA,0\r", BT_AOK)) return 0;


	return 1;
}


uint8_t bluetoothCMD_ACK(char *cmd, char *ack){
		memset(btRxBuff, 0, sizeof(btRxBuff));


		if(HAL_UART_Transmit_DMA(&huart2, (uint8_t*)cmd, strlen(cmd)) != HAL_OK) return 0;
		while(HAL_DMA_GetState(&hdma_usart2_tx) == HAL_DMA_STATE_BUSY);
		while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_TX);
		if(HAL_UART_Receive_DMA(&huart2, btRxBuff, strlen(ack)) != HAL_OK) return 0;
		while(HAL_DMA_GetState(&hdma_usart2_rx) == HAL_DMA_STATE_BUSY);
		while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX);

		HAL_UART_DMAStop(&huart2);
		if(strstr((char *)btRxBuff, ack) == 0) return 0;

		return 1;

}

uint8_t bluetoothCMD_Until(char *cmd, char *terminator, char *recvBuffer){

	memset(btRxBuff, 0, sizeof(btRxBuff));

	btRxStatus = 2;
	btRxIndex = 0;

	if(HAL_UART_Receive_DMA(&huart2, (uint8_t*)&btRxByte, 1) != HAL_OK) return 0;
	if(HAL_UART_Transmit_DMA(&huart2, (uint8_t*)cmd, strlen(cmd)) != HAL_OK) return 0;
	//Ceka nez prijde terminator
	while(!strstr((char *)btRxBuff, terminator) && btRxIndex < BT_RX_BUFF_SIZE);

	if(strstr((char *)btRxBuff, terminator) == 0) return 0;

	memcpy(recvBuffer, (char *)btRxBuff, btRxIndex);

	HAL_UART_DMAStop(&huart2);

	return 1;
}

uint8_t bluetoothCMD_Time(char *cmd, uint8_t s, char (*recvBuffer)[]){
	memset(btRxBuff, 0, sizeof(btRxBuff));

	btRxStatus = 2;
	btRxIndex = 0;
	if(HAL_UART_Receive_DMA(&huart2, (uint8_t*)&btRxByte, 1) != HAL_OK) return 0;
	if(HAL_UART_Transmit_DMA(&huart2, (uint8_t*)cmd, strlen(cmd)) != HAL_OK) return 0;


	long long int start = HAL_GetTick();

	//Ceka dany cas
	while(btRxIndex < BT_RX_BUFF_SIZE && HAL_GetTick() < (start+s*1000));

	memcpy(recvBuffer, (char *)btRxBuff, btRxIndex-1);

	HAL_UART_DMAStop(&huart2);

	return 1;
}


uint8_t bluetoothGetScannedDevices(){

	char buff[300];
	memset(buff, 0, 300);
	//memset(oledHeader, 0, 30);
	//memset(buff, 0, 100);

	if(!bluetoothCMD_Time("F\r", 15, &buff)) return 0;

	char *devices[20];
	/*for(int i=0; i < 20; i++){
		devices[i] = (char*)malloc(100);
		memset(devices[i], 0, 100);
	}*/

	btScannedCount = countOccurances(buff, "%")/2;
	replacechar(buff, 300, '\r', ' ');
	splitString(buff, "\n", devices);

	//sprintf(oledHeader, "Count: %d", btScannedCount);

	uint8_t i;

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

		btScanedDevices[i].font = &Font_11x18;
		btScanedDevices[i].name = btScanned[i].name;
		//sprintf(btScanedDevices[i].name, "%s", btScanned[i].name);
		btScanedDevices[i].selected = 0;
		btScanedDevices[i].hasSpecialSelector = 0;
		btScanedDevices[i].specharNotSelected = 0;
		btScanedDevices[i].specharSelected = 0;
		btScanedDevices[i].submenuLevel = 3;
		btScanedDevices[i].parentItem = &bluetoothmenu[0].name;


	}


	btScanedDevices[btScannedCount].font = &Font_11x18;
	btScanedDevices[btScannedCount].name = "Zpet";
	btScanedDevices[btScannedCount].selected = 0;
	btScanedDevices[btScannedCount].hasSpecialSelector = 1;
	btScanedDevices[btScannedCount].specharNotSelected = 36;
	btScanedDevices[btScannedCount].specharSelected = 37;
	btScanedDevices[btScannedCount].submenuLevel = 3;
	btScanedDevices[btScannedCount].parentItem = 0;

	bluetoothCMD_Until("X\r", "CMD> ", buff);


	return 1;

}

uint8_t bluetoothGetBondedDevices(){

	sprintf(oledHeader, "1");

	char buff[300];
	memset(buff, 0, 300);

	sprintf(oledHeader, "2");


	if(!bluetoothCMD_Until("LB\r", "END", buff)) return 0;

	//char *devices[20];
	/*for(int i=0; i < 20; i++){
		devices[i] = (char*)malloc(100);
		memset(devices[i], 0, 100);
	}*/

	//btBondedCount = countOccurances(buff, "\n");
	//replacechar(buff, 300, '\r', ' ');
	//splitString(buff, "\n", devices);

	sprintf(oledHeader, "Count: %d", btBondedCount);
	oled_refresh();

	/*uint8_t i;

	for(i = 0; i < btBondedCount; i++){
		//Nactou se informace do seznamu zarizeni
		sscanf((char *)devices[i], "%*d,%02X%02X%02X%02X%02X%02X,%d", &btBonded[i].mac[0], &btBonded[i].mac[1], &btBonded[i].mac[2], &btBonded[i].mac[3], &btBonded[i].mac[4], &btBonded[i].mac[5], &btBonded[i].mactype);
		sprintf(btBonded[i].name, "%02X-%02X-%02X-%02X-%02X-%02X", btBonded[i].mac[0], btBonded[i].mac[1], btBonded[i].mac[2], btBonded[i].mac[3], btBonded[i].mac[4], btBonded[i].mac[5]);

		btBondedDevicesMenu[i].font = &Font_11x18;
		btBondedDevicesMenu[i].name = btBonded[i].name;
		//sprintf(btScanedDevices[i].name, "%s", btBonded[i].name);
		btBondedDevicesMenu[i].selected = 0;
		btBondedDevicesMenu[i].hasSpecialSelector = 0;
		btBondedDevicesMenu[i].specharNotSelected = 0;
		btBondedDevicesMenu[i].specharSelected = 0;
		btBondedDevicesMenu[i].submenuLevel = 3;
		btBondedDevicesMenu[i].parentItem = &bluetoothmenu[0].name;


	}


	btBondedDevicesMenu[btBondedCount].font = &Font_11x18;
	btBondedDevicesMenu[btBondedCount].name = "Zpet";
	btBondedDevicesMenu[btBondedCount].selected = 0;
	btBondedDevicesMenu[btBondedCount].hasSpecialSelector = 1;
	btBondedDevicesMenu[btBondedCount].specharNotSelected = 36;
	btBondedDevicesMenu[btBondedCount].specharSelected = 37;
	btBondedDevicesMenu[btBondedCount].submenuLevel = 3;
	btBondedDevicesMenu[btBondedCount].parentItem = 0;

*/
	return 1;

}


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

uint32_t replacechar(char *str, uint16_t size, char orig, char rep){

	uint32_t n = 0;

	for(uint16_t i = 0; i < size; i++){
		if(*(str+i) == orig){
			//*(str+i) = rep;
			n++;
		}
	}

	/*char *ix = str;
    uint32_t n = 0;
    while((ix = strchr(ix, orig)) != NULL) {
        *(ix)++ = rep;
        n++;
    }*/
    //return n;
}

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

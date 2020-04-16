#include "msgDecoder.h"
#include "midiController.h"
#include "oled.h"
#include "usart.h"
#include <stm32g0xx_hal_rtc.h>
#include <rtc.h>
#include <stdlib.h>

extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

void decodeMessage(char * msg, uint16_t len, uint8_t broadcast){

	btData = 1;

	//Internal
	char msgType = msg[6];

	uint8_t src = ((msg[6] & 0x18) >> 3);

	if((msgType & 0xE0) == 0x20){
		if(msg[7] == INTERNAL_COM){
			if(msg[8] == INTERNAL_COM_PLAY){
				midiController_play(src, &msg[9]);
			}else if(msg[8] == INTERNAL_COM_STOP){
				midiController_stop(src);
			}else if(msg[8] == INTERNAL_COM_REC){
				midiController_record(src, &msg[9]);
				msgAOK(0, msgType, len, 0, NULL);
			}else if(msg[8] == INTERNAL_COM_KEEPALIVE){
				if(src == ADDRESS_MAIN){
					aliveMain = 1;
					btStreamOpen = 1;
					btStreamSecured = 1;
					btStreamBonded = 1;
					aliveMainCounter = 0;
				}else if(src == ADDRESS_PC){
					alivePC = 1;
					btStreamOpen = 1;
					btStreamSecured = 1;
					btStreamBonded = 1;
					alivePCCounter = 0;
				}
			}else if(msg[8] == INTERNAL_COM_SET_TIME){
				RTC_TimeTypeDef time;
				RTC_DateTypeDef date;

				time.Seconds = msg[9];
				time.Minutes = msg[10];
				time.Hours = msg[11];
				time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
				time.SecondFraction = 0;
				time.SubSeconds = 0;
				time.TimeFormat = RTC_HOURFORMAT_24;
				time.StoreOperation = RTC_STOREOPERATION_SET;

				date.WeekDay = RTC_WEEKDAY_MONDAY;
				date.Date = msg[12];
				date.Month = msg[13];
				date.Year = msg[14];

				HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
				HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);

			}else msgERR(0, msgType, len);
		}else if(msg[7] == INTERNAL_CURR){
			if(msg[8] == INTERNAL_CURR_ON){
				//midiControl_current_On();
				msgAOK(0, msgType, len, 0, NULL);
			}else if(msg[8] == INTERNAL_CURR_OFF){
				//midiControl_current_Off();
				msgAOK(0, msgType, len, 0, NULL);
			}else msgERR(0, msgType, len);

		}else msgERR(0, msgType, len);
	}else msgERR(0, msgType, len);
}


void msgAOK(uint8_t aokType, uint8_t recType, uint16_t recSize, uint16_t dataSize, char * msg){
	char * buffer = (char*)malloc(dataSize);
	//Utvori se AOK znak s typem
	buffer[0] = 0x80 | (aokType & 0x7f);
	buffer[1] = recType;
	buffer[2] = ((recSize-6) & 0xff00) >> 8;
	buffer[3] = (recSize-6) & 0xff;
	memcpy(&buffer[4], msg, dataSize);
	sendMsg(ADDRESS_MAIN, ((recType & 0x18) >> 3), 0, 0x07, buffer, dataSize+5);
	free(buffer);
}

void msgERR(uint8_t errType, uint8_t recType, uint16_t recSize){
	char * buffer = (char*)malloc(5);
	//Utvori se ERR znak s typem
	buffer[0] = 0x7f & (errType & 0x7f);
	buffer[1] = recType;
	buffer[2] = ((recSize-6) & 0xff00) >> 8;
	buffer[3] = (recSize-6) & 0xff;

	sendMsg(ADDRESS_MAIN, ((recType & 0x18) >> 3), 0, 0x07, buffer, 5);
	free(buffer);
}

void sendMsg(uint8_t src, uint8_t dest, uint8_t broadcast, uint8_t type, char * msg, uint16_t len){
	uint8_t * buffer = (uint8_t*)malloc(len+7);
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	buffer[4] = ((len+1) >> 4) & 0xff;
	buffer[5] = (len+1) & 0xff;
	buffer[6] = ((type & 0x07) << 5) | ((src & 0x3) << 3) | ((broadcast & 0x01) << 2) | (dest & 0x03);
	memcpy(&buffer[7], msg, len);


	if(btStreamOpen && !btCmdMode){
		HAL_UART_Transmit_IT(&huart2, buffer, len+7);
		btData = 1;
	}

}


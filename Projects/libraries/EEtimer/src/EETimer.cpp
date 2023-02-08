#include <Templates.h>
#include <EETimer.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <Arduino.h>
#define __ASSERT_USE_STDERR
#include <assert.h>


#include "Sendf.h"


const char* EETimer::cerrortime() {
	return ctime(dateTime + secSResetAtError - secSReset);
}
 
char*EETimer::clocktime(char hourminsep) {
	time_t timestamp = dateTime+millis()/1000-secSReset;
	tm* ptm = gmtime(&timestamp);
	sprintf(EETimer::ctimeBuf,"%02d%c%02d",ptm->tm_hour,hourminsep,ptm->tm_min);
	return EETimer::ctimeBuf;
}
char *EETimer::ctime(time_t timestamp) {
	tm* ptm = gmtime(&timestamp);
	//ptm->tm_isdst=0;
	strcpy(EETimer::ctimeBuf,itoa(ptm->tm_year-100,EETimer::ctimeBuf,10));
	strftime(EETimer::ctimeBuf+2,sizeof(EETimer::ctimeBuf)-2,"-%m-%d %X",ptm);
	return EETimer::ctimeBuf;
} 


char *EETimer::ctime() {
	return ctime(dateTime+millis()/1000-secSReset);
}


uint32_t EETimer::errorSource() {
	return errSource;
}
uint16_t EETimer::errorLine() {
	return errLineNr;
}

bool EETimer::hasSettedError() {
	return errSource && errHasBeenSet;
}

EETimer & EETimer::instance() {
    static EETimer eETimer;
    return eETimer;
} 


void EETimer::setError(uint32_t errSrc,uint16_t lNr) {
	errHasBeenSet=true;
	errSource=errSrc;
	errLineNr = lNr;
	secSResetAtError = millis()/1000;
	writeEEProm();
}


void EETimer::setError(const char file[],uint16_t line) {
	setError(crc32(file),line);
}
                     
const char* EETimer::setTime(char timestamp[]) {
		//dateTime = atol(timestamp);
		dateTime = atol(timestamp) + 3600;
		secSReset = millis()/1000;
		writeEEProm();
		return ctime(dateTime);

}

uint32_t EETimer::time() {
	return dateTime+millis()/1000-secSReset;
}

void EETimer::writeEEProm() {
    EEPROMWrite(EETIMER_LOG_ADDRESS,*this);
}



uint32_t EETimer::crc32(const char cstr[]) {
   auto *pstr=cstr;
   uint32_t crc = 0xFFFFFFFF;
   while (*pstr) {  
      crc = crc ^ *pstr++;
      for (int cnt = 8; cnt; cnt--)
         crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
   }
   return ~crc;
}


/**********************************
            _            _       
 _ __  _ __(_)_   ____ _| |_ ___ 
| '_ \| '__| \ \ / / _` | __/ _ \
| |_) | |  | |\ V / (_| | ||  __/
| .__/|_|  |_| \_/ \__,_|\__\___|
|_|                              
**/

EETimer::EETimer() {
    EEPROMRead<EETimer>(EETIMER_LOG_ADDRESS,*this);
}





/**************************************************************************
       _       _           _                   _       _        _   _      
  __ _| | ___ | |__   __ _| |   __ _ _ __   __| |  ___| |_ __ _| |_(_) ___ 
 / _` | |/ _ \| '_ \ / _` | |  / _` | '_ \ / _` | / __| __/ _` | __| |/ __|
| (_| | | (_) | |_) | (_| | | | (_| | | | | (_| | \__ \ || (_| | |_| | (__ 
 \__, |_|\___/|_.__/ \__,_|_|  \__,_|_| |_|\__,_| |___/\__\__,_|\__|_|\___|
 |___/                                                                     
**/

bool EETimer::errHasBeenSet=false;
char EETimer::ctimeBuf[26];

EETimer& eeTimer = EETimer::instance(); 
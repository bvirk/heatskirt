#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include "Commands.h"
#include "RiExec.h"
#include "TaskRunner.h"
#include "Sendf.h"
#include <avr/pgmspace.h>

#define __ASSERT_USE_STDERR
#include <assert.h>
#include "EETimer.h"
//#include "DallasTemperature.h"
//#include "DS18b20.h"
//#include "Wire.h"
#include "SSD1306.h"
#include "Thermistor.h"
#include "DHT22.h"
#include <math.h>


extern TaskRunner taskRunner;
extern RiExec cmds;
extern EETimer& eeTimer;
#ifdef SSD1306_h
	extern SSD1306 oled;
#endif

#ifdef DS18b20_h
extern DeviceAddress devAddr[1];
#endif

#ifdef DHT_H
extern DHT dht;
#endif


namespace cmdFuncs {

bool assertEnabled=true;

/*!
 * @details autoexec is called on usb connection\n
 */
uint8_t autoexec(uint8_t argc, char *argv[]) {
	if (eeTimer.errorSource()) {
		sendfRED("%s",eeTimer.cerrortime());
		sendfALERT("%lu %d",eeTimer.errorSource(),eeTimer.errorLine());
		eeTimer.setError(0);
	}
	
	if (argc && isdigit(*argv[0])) {
		const char * setimeResult =eeTimer.setTime(argv[0]);
		sendf("%s",setimeResult);
		
	}
	#ifdef SSD1306_h
    	taskRunner.viewClock();
  	#endif

	return 0;
}
/*!
 * @details humidity\n
 */
uint8_t humidity(uint8_t argc, char *argv[]) {
	#ifdef DHT_H
		float hum = dht.readHumidity();
		if (isnan(hum) && assertEnabled) {
			sendf("hum, powerOnCnt=%hu",dht.powerOnCnt);
		}
        sendf("Humidity %s",dtostr(hum,0));
		float temp=dht.readTemperature();
		if (isnan(temp) && assertEnabled) {
			sendf("temp, powerOnCnt=%hu",dht.powerOnCnt);
        }
		sendf("Temperature %s grader celius",dtostr(temp,1));
                
	#else
		sendf("facility not enabled");
	#endif
		return 0;
}


/*!
 * @details lsdht22!\n
 */
uint8_t lsdht22(uint8_t argc, char *argv[]) {
	#ifdef DHT_H
		char bufb[30];
		strcpy(bufb,dtostr(dht.readHumidity(),0)); 
		sendf("humidit=%s, temp=%s, restarts=%hu, drift=%lu, failCause=%u"
			,bufb,dtostr(dht.readTemperature(),1),dht.powerOnCnt,dht.cycleCnt,toInt(dht.failCause));
	#endif
	return 0;
}

/*!
 * @details lsinput\n
 */
uint8_t lsinput(uint8_t argc, char *argv[]) {
	taskRunner.viewInputPins();
	//taskRunner.printButtons();
	return 0;
}


/*!
 * @details syntax\n  oledputs string [xstart[ystart[scale]]]\n\n  defaults\n  xstart=0\n  ystart=0\n  scale=1\n\n  oled -h\n
 */
uint8_t oledputs(uint8_t argc, char *argv[]) {

	#ifdef SSD1306_h
		taskRunner.viewNothing();
		uint8_t x = argc > 1 ? atoi(argv[1]) : 0;
		uint8_t y = argc > 2 ? atoi(argv[2]) : 0;
		uint8_t s = argc > 3 ? atoi(argv[3]) : 1;
		bool inv  = argc > 4 && atoi(argv[4]) > 0 ? true : false;
		//sendf("called with x,y=%d,%d",x,y);
		oled.fill();
		uint32_t xyStart_xyEnd = oled.puts(argv[0],x,y,s,inv);
		//uint32_t xyStart_xyEnd = 0x0a041405l;
		uint8_t xStart = xyStart_xyEnd >> 24;
		uint8_t yStart = (xyStart_xyEnd >> 16) & 0xff;
		uint8_t xEnd = (xyStart_xyEnd >> 8) & 0xff;
		uint8_t yEnd = xyStart_xyEnd & 0xff;
		sendf("xStart=%hu, yStart=%hu, xEnd=%hu, yEnd=%hu",xStart,yStart,xEnd,yEnd);
	#else
		sendf("facility disabled");
	#endif
	return 0;
}
/*!
 * @details syntax\n  oledascii [asciicode]\n\nascii code is a multiplum of 32\nwithout startpage means starting at 0\n
 */
uint8_t oledascii(uint8_t argc, char *argv[]) {
	#ifdef SSD1306_h	
		uint8_t p = argc ? atoi(argv[0]) : 0;
		//sendf("called with x,y=%d,%d",x,y);
		oled.fill();
		oled.puts("0123456789ABCDEF",18,0,1);
		oled.puts("----------------",0,1,1);
		char buf[20];
		uint8_t page=2;
		for (int i=p; i<p+64; i+=16) {
			snprintf(buf,3,"%02X ",i);
			buf[2]=' ';
			for (int col=0; col<16; col++)
				buf[col+3]=i+col != 0 ? i+col : 0X2E ;
			buf[19]='\0';
			oled.puts(buf,0,page++,1);
		}
	#else
		sendf("facility disabled");
	#endif
		return 0;
}
/*!
 * @details oledview [n[op]] [c[lock]] [a[ds18b20Temp]] [d[ht22Temp]] [t[emp-ncc]]  [r[esistance]] [s[amples]] [h[umidity]]\n\nas much as enabled hardware\n
 */
uint8_t oledview(uint8_t argc, char *argv[]) {
	#ifdef SSD1306_h
		if (argv) {
			switch(*argv[0]) {
				case 'n':
					taskRunner.viewNothing();
					break;
				case 'c':
					oled.fill();
					oled.puts(eeTimer.clocktime(),0,3,4);
					taskRunner.viewClock();
					break;
	#ifdef ds18b20_h
				case 'a':
					taskRunner.viewDS18B20Temp();
					break;
	#endif
	#ifdef Thermistor_h
				case 'r':
					taskRunner.viewRest();
					break;
				case 's':
					taskRunner.viewSample();
					break;
				case 't':
					taskRunner.viewNTCTemp();
					break;
	#endif
	#ifdef DHT_H
				case 'd':
					taskRunner.viewDHTTemp();
					break;
				case 'h':
					taskRunner.viewHum();
					break;
	#endif
			} //end switch 
		} // end argv
	#else
		sendf("no oled enabled");
	#endif
	return 0;
}

/*!
 * @details pin value\n
 */
uint8_t out(uint8_t argc, char *argv[]) {
    if (argc==2) {
		uint8_t pin = atoi(argv[0]);
		pinMode(pin,OUTPUT);
		digitalWrite(pin,atoi(argv[1]) & 1);
	}

    return 0;
}

/*!
 * @details detects i2c device - disabled\n
 */
uint8_t onewiredetect(uint8_t argc, char *argv[]) {
	#ifndef TwoWire_h
		sendf("TwoWire_h not included");
		return 0;
	#else
		sendf("searching for i2c" );
		Wire.begin();
		uint8_t error, address;
		int nDevices=0;
		for(address = 1; address < 127; address++ ) {
			Wire.beginTransmission(address);
			error = Wire.endTransmission();
			if (error == 0) {
				sendf("I2C device found at address %x",address);
				nDevices++;
			} else if (error==4) 
				sendf("Unknown error at address %x",address);
		}
		if (!nDevices)
			sendf("no I2C devices found");
		return 0;
	#endif
}


/*!
 * @details play start slut\n
 */
uint8_t play(uint8_t argc, char *argv[]) {
	taskRunner.viewNothing();
	if (argc>1) {
		int delayV=argc>2? atoi(argv[2]): 500;
		for (uint8_t i=atoi(argv[0]); i <= atoi(argv[1]); i++ ) {
			//sendf("%d: somefunc(%d)=%d",i,i,2*i);
			char buf[30];
			memset(buf,32,2);
			itoa(i,buf+2,10);
			char *nb = buf+3;
			while(*nb++);
			*nb='\0';
			*(nb-1)=i;
			oled.puts(nb-4,0,3,4);
			delay(delayV);

		}
		
	}
	return 0;
}

/*!
 * @details reset\n
 */
uint8_t reset(uint8_t argc, char *argv[]) {
	void(* resetFunc) (void) = 0;
	(resetFunc)();
	return 0;
}
/*!
 * @details syntax\n  showargs [arg [ arg]..]...\n
 */
uint8_t showargs(uint8_t argc, char *argv[]) {
	for (int i=0; i<argc; i++)
		sendf("%d: %s",i,argv[i]);
	return 0;
}
/*!
 * @details syntax\n  showerror\n\n  shows the eeTimers errNr field\n
 */
uint8_t showerror(uint8_t argc, char *argv[]) {
	sendf("errNr=%d",eeTimer.errorSource());
	return 0;
}
/*!
 * @details syntax\n  seterror filename linenumber\n  seterror [number]\n\n  sets eeTimers errorSorce crc32 and linenumber fields\n
 */
uint8_t seterror(uint8_t argc, char *argv[]) {
	if (argc >1 )
		eeTimer.setError(argv[0],atoi(argv[1]));
	else
		eeTimer.setError(atoi(argv[0]));
	return 0;
}
/*!
 * @details syntax\n  time\n  shows full time in epoch -h\n
 */
uint8_t showTime(uint8_t argc, char *argv[]) {//time
	sln(eeTimer.ctime());
	return 0;
}
/*!
 * @details sets the tasks switch delay\n
 */
uint8_t taskmaxdelay(uint8_t argc, char *argv[]) {
	//if (argc) {
	//	uint16_t newtaskmaxtime = atoi(argv[0]);
	//	if (newtaskmaxtime > 2)
	//		tasks::onBlinksTaskDelay=newtaskmaxtime;
	//}
	//sendf("%d",tasks::onBlinksTaskDelay);
	return 0;
}
/*!
 * @details toogleassertenable\n
 */
uint8_t toogleassertenable(uint8_t argc, char *argv[]) {
	assertEnabled ^=1;
	sendf("assert is now %s",assertEnabled ? "enabled" : "disabled");
	return 0;
}
/*!
 * @details toogledebug seconds\n
 */
uint8_t toogledebug(uint8_t argc, char *argv[]) {
	int period = argc == 1 ? atoi(argv[0]) : 10; 
	sendf("debug is now %s",taskRunner.toogleDebug() ? "enabled" : "disabled");
	int32_t curtime = millis();
	while (millis()-curtime < period*1000);
	sendf("debug is now %s",taskRunner.toogleDebug() ? "enabled" : "disabled");
	return 0;
}
/*!
 * @details plays a tone\n
 */
uint8_t tone(uint8_t argc, char *argv[]) {
	//if (argc > 1) {
	//	uint16_t frek = atoi(argv[0]);
	//	uint16_t dur = atoi(argv[1]);
	//	::tone(8,frek,dur);
	//}
	assert(1==2);
	return 0;
}

};

//#include "sendf.h"
#include "TaskRunner.h"
#include <EETimer.h>
#include <Arduino.h>
#include "Sendf.h"
#include "SSD1306.h"
#include "DHT22.h"
#include "Thermistor.h"

#define __ASSERT_USE_STDERR
#include <assert.h>



#define mDelay(varname,val) const uint32_t varname ## MillisDelay=val; \
                            static uint32_t varname ## LastMillis=0

#define gone(varname) curTime-varname ## LastMillis > varname ## MillisDelay
#define newtick(varname) varname ## LastMillis=curTime

// following two macroes contains opening curly bracket - editor syntaks highligtning will be confused!

#define repeatFor(varname,val) static uint32_t varname ## LastMillis=0; \
        if ( curTime-varname ## LastMillis > val) { varname ## LastMillis=curTime       

#define repeatForOledView(varname,val) static uint32_t varname ## LastMillis=0; \
        if ( oledView == OledView::varname && curTime-varname ## LastMillis > val) { varname ## LastMillis=curTime

#ifdef DHT_H
extern DHT dht;
#endif

#ifdef SSD1306_h
extern SSD1306 oled;
#endif

extern EETimer& eeTimer;

const char strFlsh1[] PROGMEM = "Clock     "; // CLOCK
const char strFlsh2[] PROGMEM = "Temp DTH22"; // DHTTEMP
const char strFlsh3[] PROGMEM = "Humidity  "; // HUM
const char strFlsh4[] PROGMEM = "Temp NTC  "; // NTCTEMP
const char strFlsh5[] PROGMEM = "Temp level"; // TEMPLEVEL
const char strFlsh6[] PROGMEM = "Hum level "; // HUMLEVEL
const char strFlsh7[] PROGMEM = "Resistance"; // REST
const char strFlsh8[] PROGMEM = "SamplesNTC"; // SAMPLE    

PGM_P const selLines[] PROGMEM = {
     strFlsh1
    ,strFlsh2
    ,strFlsh3
    ,strFlsh4
    ,strFlsh5
    ,strFlsh6
    ,strFlsh7
    ,strFlsh8
};
	



TaskRunner::TaskRunner() : 
     oledView(OledView::CLOCK)
    ,putSampleIndex(0)
    ,debug(false)
    ,topline(0),invline(0)
    ,tempLevel(15)
    ,humLevel(80) 
    ,humToHigh(false)
    ,tempToLow(false) {
    
    viewClock();
    #ifdef DHT_H
        dht22Temp = dht.readTemperature();
        dht22Hum = dht.readHumidity();
    #endif
}

void TaskRunner::setPinModes() {
    int8_t pins[] = {10,16,14,15};
    for (int i=0; i<4; i++)
        pinMode(pins[i],INPUT_PULLUP);
}

bool TaskRunner::hasButtonPress() {
    return lastPressed != 0;
}

bool TaskRunner::lastPressOf(Button b) {
    if (static_cast<uint8_t>(b) == lastPressed) {
        lastPressed=0;
        return true;
    }
    return false;
}

   
uint8_t TaskRunner::lastPress() {
    uint8_t butbits = lastPressed;
    lastPressed=0;
    return butbits;
}


void TaskRunner::timedSnippets() {
    
    int32_t curTime = millis();
    
    repeatFor(debounce,50);
        const uint8_t port = 2;
        
        uint8_t readAll =(*portInputRegister(port) & 0x4e)^0x4e;
		readAll =  // rearrange bit to buttonrow 
    		  (readAll & 0x40) >> 3 
    		| (readAll & 0x04)
    		| (readAll & 0x08) >> 2
    		| (readAll & 0x02) >> 1;

        if (previous != readAll)
            previous = readAll;
        else { 
            confirmed=previous;
            if (confirmed)
                lastPressed = confirmed;
        }        
    //    curTime = millis();
    }

    repeatFor(flashLed,2000);    
        digitalWrite(PINFLASHLED,!digitalRead(PINFLASHLED));
        //curTime = millis();
    }

    repeatFor(triac,200);
        uint8_t curStat = digitalRead(PINTRIAC);
        uint8_t newStat = tempToLow || humToHigh ? 0 : 1;
        if (curStat != newStat) {
            digitalWrite(PINTRIAC,newStat);
            tone(PINTONE,2500-500*newStat,150);
        }
    }    
        

    
#ifdef SSD1306_h

    repeatFor(clickView,200);
        if (lastPressOf(Button::K1)) {
            viewMenu();
            oled.showLines(selLines,topline,invline);
            curTime = millis();
        }
    }
    
    
    
    repeatForOledView(MENU,200);
        if (lastPressed) {
            switch(lastPress()) {
                case BUTINT(K2):
                    {
                        invline++;
                        if (invline == arraySize(selLines)) {
                            invline = 0;
                            topline= 0;
                        }
                        if (topline+3 < invline)
                            topline++;
                        oled.showLines(selLines,topline,invline);
                        curTime = millis();
                    }
                    break;
                case BUTINT(K3):
                    {
                        invline--;
                        if (invline == 0xff) {
                            invline = arraySize(selLines)-1;
                            topline= invline-3;
                        }
                        if (topline > invline)
                            topline--;
                        oled.showLines(selLines,topline,invline);
                        curTime = millis();
                    }
                    break;
            
                case BUTINT(K4):
                    switch(invline) {
                        case OVINT(CLOCK):
                            viewClock();
                            break;
                        case OVINT(DHTTEMP):
                            viewDHTTemp();
                            break;
                        case OVINT(HUM):
                            viewHum();
                            break;
                        case OVINT(NTCTEMP):
                            viewNTCTemp();
                            break;
                        case OVINT(TEMPLEVEL):
                            viewTempLevel();
                            break;
                        case OVINT(HUMLEVEL):
                            viewHumLevel();
                            break;
                        case OVINT(REST):
                            viewRest();
                            break;
                        case OVINT(SAMPLE):
                            viewSample();
                            break;
                    }
                    break;
                    
            }
        } // endif pressed
    }

    repeatForOledView(TEMPLEVEL,200);
        if (lastPressed) {
            switch(lastPress()) {
                case BUTINT(K2):
                    tempLevel++;
                    break;
                case BUTINT(K3):
                    tempLevel--;
            }
        }
        itoa(tempLevel,commonbuf,10);
        oled.puts(commonbuf,0,4,4);
        curTime = millis();

    }
  
    repeatForOledView(HUMLEVEL,200);
        if (lastPressed) {
            switch(lastPress()) {
                case BUTINT(K2):
                    humLevel++;
                    break;
                case BUTINT(K3):
                    humLevel--;
            }
        }
        itoa(humLevel,commonbuf,10);
        oled.puts(commonbuf,0,4,4);
        curTime = millis();
    }
        
    
    mDelay(clockblink,1000);
    if ( oledView == OledView::CLOCK  && gone(clockblink) ) {
        newtick(clockblink);
        
        static char *clockdelim[] = {":"," "};
        static uint8_t clockdelimPos=0;
        oled.puts(clockdelim[clockdelimPos],48,3,4);
        clockdelimPos ^= 1;
        curTime = millis();
    }

    repeatForOledView(CLOCK,60000);
        oled.fill();
        oled.puts(eeTimer.clocktime(),0,3,4);
        curTime = millis();
    }

    repeatForOledView(INPUTPIN,1000);
        strcpy(commonbuf,"000");
        itoa(lastPress(),commonbuf+3,2);
        char *p=commonbuf+3;
        while(*p++);
        oled.puts(p-5,0,4,4);
        curTime = millis();
    }


    #ifdef DHT_H
        repeatFor(readDHT22sensor,10000);
            float read = dht.readTemperature();
            if (!isnanf(read))
                dht22Temp=read;    
            read = dht.readHumidity();
            if (!isnanf(read))
                dht22Hum=read;
            
            // for triacOnByHum        
            if (dht22Hum > (float)humLevel+0.5) 
                humToHigh = true;
            if (dht22Hum <(float)humLevel-0.5)
                humToHigh = false;
            
        }

        repeatForOledView(DHTTEMP,10000);
            //snprintf(commonbuf,bufsize,"%lu",dht.cycleCnt);
            //oled.puts(commonbuf,0,0,2);
            //snprintf(commonbuf,bufsize,"ups: %03hu, failCode: %u",dht.powerOnCnt,toInt(dht.failCause));
            //oled.puts(commonbuf,0,7,1);
            showtemp(dht22Temp,"dht22-temp");
            curTime = millis();
        }

        repeatForOledView(HUM,10000);
            //float hum = dht.readHumidity();
            //snprintf(commonbuf,bufsize,"%lu",dht.cycleCnt);
            //oled.puts(commonbuf,0,0,2);
            //snprintf(commonbuf,bufsize,"ups: %03hu, failCode: %u",dht.powerOnCnt,toInt(dht.failCause));
            //oled.puts(commonbuf,0,7,1);
            oled.puts("DHT22-hum",0,0,2);
            oled.puts(dtostr(dht22Hum,0,3),0,3,4);
            oled.puts("%",73,3,4);
            curTime = millis();
        }
    #endif  // DHT_H
    #ifdef Thermistor_h
            repeatFor(ntcfortriac,10000);
                float temp = avarageNtcTemp();
                if (temp + 0.5 < tempLevel) 
                    tempToLow = true;
                if (temp -0.5 > tempLevel)
                    tempToLow = false;

            }
            
            repeatForOledView(NTCTEMP,500);
                showtemp(avarageNtcTemp(),"ntc-temp");
                curTime = millis();
            }


            
            repeatForOledView(REST,500);
                int16_t sa = 1023- avarageNccSample();
                double rest = (1-(double(sa)/1023))*10000*1023/sa;
                oled.puts(dtostr(rest,0,5),0,3,4);
                
                curTime = millis();
            }


            repeatForOledView(SAMPLE,500);
                uint16_t sa = avarageNccSample();
                strcpy(commonbuf,"    ");
                itoa(sa,&commonbuf[4],10);
                char *p = &commonbuf[4];
                while(*p++);
                oled.puts(p-6,0,3,4);
                    
                curTime = millis();
            }

    #endif // Thermistor_h

#endif      // SSD1306_h

}
void TaskRunner::viewClock(){
    oled.fill();
    oled.puts(eeTimer.clocktime(),0,3,4);
    oledView = OledView::CLOCK;
}
void TaskRunner::viewDHTTemp(){
    oled.fill();
    oledView = OledView::DHTTEMP;
}
void TaskRunner::viewDS18B20Temp(){
    oled.fill();
    oledView = OledView::DS18B20TEMP;
}
void TaskRunner::viewHum(){
    oled.fill();
    oledView = OledView::HUM;
}
void TaskRunner::viewInputPins(){
    oledView = OledView::INPUTPIN;
    oled.fill();
    
}
void TaskRunner::viewNothing() {
    oledView = OledView::NOP; 
    oled.fill();
}
void TaskRunner::viewMenu() {
    oledView = OledView::MENU; 
    oled.fill();
}
void TaskRunner::viewNTCTemp(){
    oled.fill();
    oledView = OledView::NTCTEMP;
}
void TaskRunner::viewRest(){
    oled.fill();
    oledView = OledView::REST;
}
void TaskRunner::viewSample(){
    oled.fill();
    oledView = OledView::SAMPLE;
}

void TaskRunner::viewTempLevel(){
    oled.fill();
    oled.puts("switch level",0,0,1);
    oled.puts("temp in  C",0,1,2);
    oled.puts("o",96,1,1);
    oledView = OledView::TEMPLEVEL;
}
void TaskRunner::viewHumLevel(){
    oled.fill();
    oled.puts("switch level",0,0,1);
    oled.puts("humidity %",0,1,2);
    
    oledView = OledView::HUMLEVEL;
}

bool TaskRunner::toogleDebug(){
    debug ^=1;
    return debug;
}

int16_t TaskRunner::avarageNccSample() {
    static uint8_t sampleCnt=4;
    while(sampleCnt--) {
        ntcSamples[putSampleIndex]= analogRead(A0);
        putSampleIndex = (putSampleIndex+1) & 3;
    }
    sampleCnt=1;
    int16_t sum=0;
    for (int i=0; i<4; i++)
        sum+= ntcSamples[i];
    return sum >> 2;
}

float TaskRunner::avarageNtcTemp() {
    uint16_t sa = avarageNccSample(); 
    return sa >= 800 
        ? 0.0
        : 15.345+(630 - sa)/11.1;
}


void TaskRunner::showtemp(float temp, const char* head) {
    oled.puts(head,0,0,2);
    uint8_t intPart = (uint8_t)temp;
    uint8_t deci = (temp-intPart)*10;  
    oled.puts(dtostr(temp,0,2),0,4,4);
    itoa(deci,commonbuf,10);
    oled.puts(commonbuf,60,4,4);
    oled.puts(".",46,5,2);
    oled.puts("C",100,4,4);
    oled.puts("o",85,4,2);
}

TaskRunner taskRunner;


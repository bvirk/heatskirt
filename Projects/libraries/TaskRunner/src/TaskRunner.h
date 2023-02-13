#ifndef TaskRunner_h
#define TaskRunner_h
#include <stdint.h>
#include "DHT22.h"

#define PINTONE 9
#define PINFLASHLED 8
#define PINTRIAC 6

#define arraySize(x) sizeof(x)/sizeof(x[0])


enum class OledView {CLOCK,DHTTEMP,HUM,NTCTEMP,TEMPLEVEL,HUMLEVEL,REST,SAMPLE,INPUTPIN,DS18B20TEMP,NOP,MENU,};
enum class Button {K1=8,K2=4,K3=2,K4=1};
#define OVINT(x) static_cast<int>(OledView::x)
#define BUTINT(x) static_cast<int>(Button::x)

const uint8_t bufsize=29;

class TaskRunner {
    OledView oledView;
    char commonbuf[bufsize+1];
    uint16_t ntcSamples[4];
    uint8_t putSampleIndex;
    bool debug;
    int8_t previous;
    int8_t confirmed;
    uint8_t invline;
    uint8_t topline;
    #ifdef DHT_H
        float dht22Temp;
        float dht22Hum;
    #endif
    uint8_t tempLevel;
    uint8_t humLevel;
    bool humToHigh;   // dht22 sense humidity higher than humLevel
    bool tempToLow;   // ntc sense temperature below tempLevel

    
    
    
    // present after button release, but overwritten by new button press  
    // meant to be cleared by reading - like a one button press buffer.
    // lastPressed==0 means no pending button press.
    
    // given by port    wanted         shift
    // read
    // k1 ~ 0x40        0x08            >> 3
    // k2 ~ 0x04        0x04            ok
    // k3 ~ 0x08        0x02            >> 2
    // k4 ~ 0x02        0x01            >> 1
    //
    // (read & 0x40) >> 3 | (read & 0x04) | (read & 0x08) >> 2 | (read & 0x02) >> 1 
    int8_t lastPressed; 
    
    

    int16_t avarageNccSample();
    void showtemp(float temp, const char* head);
    float avarageNtcTemp();

public:
    TaskRunner();
    
    void viewClock();
    void viewHum();
    void viewDHTTemp();
    void viewNTCTemp();
    void viewNothing();
    void viewMenu();
    void viewRest();
    void viewSample();
    void viewDS18B20Temp();
    void viewInputPins();
    void viewTempLevel();
    void viewHumLevel();
    void timedSnippets();
    void printButtons();
    bool toogleDebug();
    bool hasButtonPress();
    uint8_t lastPress();
    bool lastPressOf(Button b);
    void setPinModes();
};



#endif
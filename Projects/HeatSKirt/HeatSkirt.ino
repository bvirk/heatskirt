#include <Arduino.h>
#define __ASSERT_USE_STDERR
#include <assert.h>
#include <Sendf.h>
#include <EETimer.h>
#include <RiExec.h>
#include <TaskRunner.h>
#include <SSD1306.h>
#include <DHT22.h>


extern EETimer& eeTimer;
extern RiExec cmds;
extern TaskRunner taskRunner;

#ifdef SSD1306_h
	extern SSD1306 oled;
#endif

#ifdef  DHT_H
    DHT dht(/*pin*/7, /*vccpin*/5);
#endif



void setup() {
 pinMode(PINTONE,OUTPUT);
 pinMode(PINFLASHLED,OUTPUT);
 pinMode(PINTRIAC,OUTPUT);
 digitalWrite(PINTRIAC,1);
 taskRunner.setPinModes();
 
 Serial.begin(115200);

  
  cmds.loop();

  // leaved program
  for (int i=4; i; i--) {
    tone(PINTONE,3000,50);
		delay(150);
  }
	
}
/*
pins to use
/home/bvirk/documents/devel/myArduinoCore/.vscode/c_cpp_properties.json
reset
sda      : 2
clc      : 3
ntc      : a0
dht vcc  : 5
dht data : 7
tone     : 8
blink    : 9
triac    : 6

5v on j1: nearest usb connector


*/

void loop() {
  
  digitalWrite(PINFLASHLED,!digitalRead(PINFLASHLED));
		delay(100);
}

void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
  char *ls = __file+strlen(__file);
  while (*(ls-1) != '/' && ls > __file)
    ls--;
 	eeTimer.setError(ls,__lineno);
}

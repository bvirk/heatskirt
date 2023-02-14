# heatskirt hardware
Combined pro miro (ATMEGA32U4) app, terminal program and prebuild C++ files deployment.

and

A this piece of hardware used to turn on a heat element as function of temperature, moisture and time.
![heatcontroller](https://github.com/bvirk/heatskirt/blob/main/img/heatcontroller.png)

The heatskirt is made of a heating wire embedded in an adhesive at the bottom of the walls, an given a airflow fender.

![fender](https://github.com/bvirk/heatskirt/blob/main/img/fender.png)

The beauty that gots the job af controlling this, is a 8 Mhtz ATMEGA32U4 in its  mount on a tiny pro micro board in this box.

![promicro](https://github.com/bvirk/heatskirt/blob/main/img/promicro.png)

Besides USB, the sensors is a DHT22 humidity and temperature sensor, a NTC resistor and a SSD1306 OLED display. The power supply delivers 180ma at 5v. The separation to high voltage can be seen where removed copper makes the veroboard shine lighter. The white brick that is sticking out under the BT138 triac is a moc3041 zero crossing detect optocoupler.

# heatskirt software suite

The heatskirt software consists of three programs
- consoleshell terminal
- consoleshellDeploy prebuild task
- the AVR program

## consoleshell terminal

consoleshell expose AVR commands the way console programs is exposed to a shell and is made to be invoked with arguments. It uses gnu readline to offer history, command completion and editing facilities. 

benefits:

- verbosity don't fills flash
- ease of use
- automated dialog (time, calculating drift)

## consoleshellDeploy prebuild task

consoleshellDeploy lifts off some boilerplate coding with repspect to C++ project management in AVR programming, and exposes from avr code, settings needed for consoleshell.
- which commands exist
- what are the commands syntax (argument)
- which source files exists i the AVR program (for trace alerts byond reset)

## The AVR program

### circuit
![circuit](https://github.com/bvirk/heatskirt/blob/main/img/heatskirtCircuit.jpg)

### pins used in source
![promicropins](https://github.com/bvirk/heatskirt/blob/main/img/promicropins.png)
- 2 og 3 : not in source because sda og scl is bound direct to AVR processor registers (display)
- 5 og 7 : DHT22 constructor
- 6      : optocoupler
- 8	     : blink led
- 9      : piezo sounder - not shown on drawing
- 10,16,14,15: portB
- a0	 : ntc thermistor

### Followed principles

#### heap use
No use of heap mean no use of

- string
- strdup
- returnering anything from function besides simple types.

#### dialog
dialog is Serial dialog with a terminal.

setup() and loop() is called from main.cpp (~/arduino15/packages/arduino/hardware/avr/1.8.6/cores/arduino/main.cpp perhaps)
```
int main(void)
{
...
	setup();
    
	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}
        
	return 0;
}
```
If we define serialEventRun() it will be runned concurrently - and this is where you execute Serial.available() conditionally.

serialEventRun() is blocking - you notice it when the data recieved results in running a command. Not before the command returns, the loop() is executed again.


#### The concept of time

The program we create is in a way in a thread, even if we do not use any thread library such as RTOS or anything else.

millis(), tone(...) and Serial.available() is non blocking - in other words, something more is running than what we program - driven by timer interrupts.

[uint32_t millis()](https://www.arduino.cc/reference/en/language/functions/time/millis/) gives, as indicated, milliseconds after reset, and it runs fairly accurately if you can avoid routines that disables interrupts.

So precise is, the crystal behind the timercount behind millis(), that you have, not just a clock - but a time. The only thing you need is a uint32_t offset in seconds, to a fixed point of time - epoch y2k, 2000-1-1 00:00:00 is an obvious choice.

The AVR must of course retrieve this current time from the outside, in order to save an offset, named secSReset. This is why a terminal program that can do more than pass on entries is a must.

Offset secSReset is in the singleton object eeTimer, which is constructed from an EEPROM area, which all non const methods also update. It has another offset, secSResetAtError, for entering an error condition. In addition, the file name id and line number that triggered the error condition are stored in the eeTimer object. After reset, the time of the error condition and its triggering location in the source code are thus still available.

Signalled error condition can be used for both programming errors and situations beyond what the device is made for being able to control- the motor in the water pump runs periodically, but the water sensor detects that the barrel is empty, the sensor on the motorized door detects that something is stuck, etc.

uint32_t counting milliseconds wraps around exactly (double)UINT32_MAX/1000/60/60/24 = 49,7103 days after reset

If it is not possible to avoid rutines that disables interrupt, and it has an unacceptable impact, the solution is:
- Ensure the interrupt disabling has a constant impact over time.
- Make a drift compensation event.
- Calculate the drift by time differens observation over a period.

#### alerts
```
#include <assert.h>
```
In all cpp files which should be able to
```
alert(condition)
```

I Heatskirt.ino

```
...
#include <assert.h>
...
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
  //basefile
  char *ls = __file+strlen(__file);
  while (*(ls-1) != '/' && ls > __file)
    ls--;
  eeTimer.setError(ls,__lineno);
}
```

#### structure
By structure is meant which files are used by others and in what way.

Almost everything is placed in libraries - according to their type. Heatskirt.ino does not contain much. For every .cpp there is .h file.

Strictly speaking, there is no class structure - just classes and exactly one global object of each - except for I2c which is inside the SSD1306. Classes have access to each other through the declaration of an external variable for the one instance and the inclusion of the .h file.


#### loops as runlevels
Two runlevels: normal and error:

```
//runlevel normal, conceptually illustrated
while(true) {
	...
	if (error)
		break;
}

//runlevel error, yes, that's how short loop() really is - a quick blink
void loop() {
	digitalWrite(FLASHLED,!digitalRead(FLASHLED));
	delay(150);
}
```
Runlevel normal is called in setup() and is just

```
cmds.loop();

```
And that is how the loop looks

```
void RiExec::loop() {
    cmdNPos=0;
    while (true) {
        if (eeTimer.hasSettedError())
            break;
        taskRunner.timedSnippets();
        if (serialEvent()) {
            runCmd();
            cmdNPos=0;
        }
    }
}    
```

and eeTimer.hasSettedError() becomes true when one of the previously mentioned alert's arguments is false.

#### commands
commands is, in the AVR source, a list of function pointers.
```
uint8_t (*cmds[])(uint8_t,char**) = {myname1,myname2,...};
```
The functions is, like C++'s main in a program for a pc: int main(int argc, char*argv[]), only there is no default argument 0 representing the program itself.

Everything I have developed has started with being tested in a command - the first has probably been:
```
/*!
 * @details syntax\\n  showargs [arg [ arg]..]...\\n
 */
uint8_t showargs(uint8_t argc, char *argv[]) {
	for (int i=0; i<argc; i++)
		sendf("%d: %s",i,argv[i]);
	return 0;
}
```
sendf is a macro that wraps printf with a const __FlashStringHelper * for the formatstring argument.

Commands have no text identification in the AVR - the terminal sends, space separated, the number which the command is in the list and the parameters. Put simply, terminal has seen the above source code and if you type 'showargs -h', terminal responds with:


```
syntax
  showargs [arg [ arg]..]...

```
__without__ any communication with the AVR'en

Terminal is called consoleshell and avoids a lot of repeated typing with gnu readlines command completion, command editing, and history.

#### timedSnippets, LEDFLASH

When terminal does not send anything, or a command is not in progress, taskRunner.timedSnippets() is the only thing that is called repeatable. 

Method timedSnippets() is a bit ugly because it is not divided into functions according to their type - it would cost a function call and return for each snippet. 

It's also ugly because it uses macros so unorthodox that confuses vscode syntax highlighting.

We need FLASHLED to flash slowly - change every two seconds indicating that everything is running (quietly)

```
void TaskRunner::timedSnippets() {
	int32_t curTime = millis();
	
	const uint32_t flashLedMillisDelay=2000;
	static uint32_t flashLedLastMillis=0;
	if ( curTime-flashLedLastMillis > flashLedMillisDelay) { 
		flashLedLastMillis=curTime;
		digitalWrite(FLASHLED,!digitalread(FLASHLED));
	}
	...
}
```
And more follows. sensors and buttons are read, the display is updated, the heater is switched on and off - it could quickly become an unmanageable function.

Note how the flashLed repeats in constant and variable. There is a small trick in it - search 'macro concatenation' to understand what the following does

```
#define repeatFor(varname,val) static uint32_t varname ## LastMillis=0; \
        if ( curTime-varname ## LastMillis > val) { varname ## LastMillis=curTime
```

and the above snippet is reduced to

```
void TaskRunner::timedSnippets() {
	int32_t curTime = millis();
	
	repeatFor(flashLed,2000);
            digitalWrite(PINFLASHLED,!digitalRead(PINFLASHLED));
            //curTime = millis();
	} // NOTICE THIS CURLY - its matching { is inside the macro 
	...
}
```
Note the outcommented curTime=millis(). Many similar snippets follow below and the time a snippet takes to execute offsets millis() from curTime. Wait times are not a multiple of each other, so for each run of timedSnippets() different snippets are serviced. On the other hand, a call is saved - so if the current snippet is not considered to have taken a long time, it can be deemed redundant with a millis() call.

#### timedSnippets, buttons

![buttons](https://github.com/bvirk/heatskirt/blob/main/img/buttons.png)

The four buttons k1,k2,k3,k4 grounds by press respectively pin 10,16,14,15 which has pinMode INPUT_PULLUP.

The microcontroller does not have pins - it is the board's numbers at the pins that are conveniently referred to. The microcontroller has ports and some bits in port B are connected to 10,16,14,15. We must tackle them together or it will get complicated.
Two similar samples over 50ms is what we use as debounce free value.

The TaskRunner object, taskRunner has 

```
int8_t previous;
int8_t confirmed;
int8_t lastPressed;
```

```
void TaskRunner::timedSnippets() {
	...
	
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
            // curTime = millis();
    }
	...
}
```

The bits we deals with is 5,3,2  og 1 - they are  masked out with  0x4e. We want them actively represented as 1 (high) therefore reversed with ^0x4e; (input is drawn low by press). We change the position of the bits so that the key position in the image above represents high for a depressed key - In other words: k1 ~ 1000, k2 ~ 0100, k3~ 0010 og k4 ~ 0001

the keystroke is buffered in lastPressed. It is read with

```
uint8_t TaskRunner::lastPress() {
    uint8_t butbits = lastPressed;
    lastPressed=0;
    return butbits;
}
```
which then 'empties the buffer'

With

```
bool TaskRunner::lastPressOf(Button b) {
    if (static_cast<uint8_t>(b) == lastPressed) {
        lastPressed=0;
        return true;
    }
    return false;
}
```
the 'buffer' is only 'emptied' if contains what matches argument b (enum class type defined i TaskRunner.h)

and finally, perhaps more illustrative than necessary as an independent method.

```
bool TaskRunner::hasButtonPress() {
    return lastPressed != 0;
}
```

#### timedSnippets, CLOCK

taskRunner has

```
OledView  oledView;
```
basered on, from TaskRunner.h 
```
enum class OledView {CLOCK,DHTTEMP,HUM,NTCTEMP,TEMPLEVEL,HUMLEVEL,REST,SAMPLE,INPUTPIN,DS18B20TEMP,NOP,MENU};
```
and initialised to, in TaskRunner::TaskRunner()
```
oledView(OledView::CLOCK)
```
With yet another macro

```
#define repeatForOledView(varname,val) static uint32_t varname ## LastMillis=0; \
        if ( oledView == OledView::varname && curTime-varname ## LastMillis > val) { varname ## LastMillis=curTime
```

Is the clock updated every minute - however not at second 0 for the sake of source code simplicity.

```
void TaskRunner::timedSnippets() {
	...
	repeatForOledView(CLOCK,60000);
            oled.fill();
            oled.puts(eeTimer.clocktime(),0,3,4);
            curTime = millis();
	}
	...
}
```
We want the ':' between hours and minutes to flash with the second. You can't just repeatForOledView(CLOCK,1000) because it's a macro that declares and defines a CLOCKLastMillis, and if you could reuse it would destroy the previous use.

With another serie of macros
```
#define mDelay(varname,val) const uint32_t varname ## MillisDelay=val; \
                            static uint32_t varname ## LastMillis=0

#define gone(varname) curTime-varname ## LastMillis > varname ## MillisDelay
#define newtick(varname) varname ## LastMillis=curTime
```
the flashing colon is implemented like this:
```
void TaskRunner::timedSnippets() {
	...
	mDelay(clockblink,1000);
	if ( oledView == OledView::CLOCK  && gone(clockblink) ) {
		newtick(clockblink);
		
		static char *clockdelim[] = {":"," "};
		static uint8_t clockdelimPos=0;
		oled.puts(clockdelim[clockdelimPos],48,3,4);
		clockdelimPos ^= 1;
		curTime = millis();
	}
	...
}
```

#### timedSnippets, more buttons

The buttons get these uses:

- k1: menu
- k2: select next or count up
- k2: select previous or count down
- k4: confirm (where necessary)

taskRunner has a lot of viewSOMETHINGONDISPLAY() public methods, e.g. viewDHTTemp() and viewCLOCK(). They can be called from commands or internally. The oledView member data variable gets the right value thereby.

Regardless of what oledView is and thus the display shows, a menu must be able to be called up:

```
void TaskRunner::timedSnippets() {
	...
    repeatFor(clickView,200);
            if (lastPressOf(Button::K1)) {
                viewMenu();
                oled.showLines(selLines,topline,invline);
            }
    }
	...
}
```
selLines is the PROGMEM flash version of a global const char* array[], topline the one that appears on display line 1 and selected item is displayed inverted.

Key presses other than k1 are not 'reset from lastPressed' - therefore they are passed through to

```
void TaskRunner::timedSnippets() {
	...
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
    } // end repeatForOledView
	...
}
```

#### timedSnippets, levels

taskRunner has data members

```
bool humToHigh;    // dht22 sense humidity higher than humLevel
bool tempToLow;    // ntc sense temperature below tempLevel
uint8_t tempLevel; // initial 15 by TaskRunner::TaskRunner()
uint8_t humLevel;  // initial 80 by TaskRunner::TaskRunner()
float dht22Temp;
float dht22Hum;
uint16_t ntcSamples[4];
uint8_t putSampleIndex;
```
There are two temperature sensors, DHT22 and an NTC resistor. The NTC resistor alone is used to switch the triac relay on/off - but the DHT temperature is read and can be shown on the display.

DHT22 has a timed snippet, but 'NTC value', which is analogRead samples of voltage division between ntc resistor and a general resistor is read 'on demand' - well enough noise smoothed by being based on an average of 4 samples.

The triac relay turns on, based on the ntc sensed temperature and the DHT sensed humidity.

If just one of humToHigh or tempToLow is true, then the triac relay is turned on.

```
void TaskRunner::timedSnippets() {
	...
    repeatFor(triac,200);
            uint8_t curStat = digitalRead(PINTRIAC);
            uint8_t newStat = tempToLow || humToHigh ? 0 : 1;
            if (curStat != newStat) {
                digitalWrite(PINTRIAC,newStat);
                tone(PINTONE,2500-500*newStat,150);
            }
    }    
	...
}
```
A small beep to satisfy the creator - a LED also indicates the state.

tempToLow and humToHigh relate levels with a hysteresis of 1 degree or 1 procent

```
void TaskRunner::timedSnippets() {
	...
	repeatFor(ntcfortriac,10000);
            float temp = avarageNtcTemp();
            if (temp + 0.5 < tempLevel) 
                tempToLow = true;
            if (temp -0.5 > tempLevel)
                tempToLow = false;
    }
	...

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
	...
}
```

Levels can be set with the up/down buttons

```
void TaskRunner::timedSnippets() {
	...
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
    ...
}
```

#### sensors, DHT22

The Adafruit driver was not sufficient. It is a well-documented but proprietary interface.

Each read is initiated by the DHT22 'responding' to the initiating read request by setting nivea first low and then high at certain intervals, the periode sizes is not critical.

After ½ day of reading every two seconds, it just doesn't bother anymore! You can do a reset by unplugging it electrically and plugging it back in - then it will run again for a while.

We are faced with the interesting phenomenon that empiricism wins over documentation.

There are no guesswork worth wasting time on.

It seems that you can get a lot of readings after reset. The driver, DHT22.cpp and DHT22.h are adapted to suit reality.

- simplification to get an overview 
	- DHT22 only
	- AVR port reading only because it is an AVR
	- removed unused methods
- vcc pin added as data member and constructor argument
- pinMode INPUT_PULLUP replaced by INPUT - an ekstern resistor to __pin vcc__ (not power vcc, which is not used anymore)
- a readwrapper() enclosures read()
- read() is changed
    - only succesfull reading changes
        - data[5]
        - _lastreadtime

There is no guarantee that a read at any time will not return the NAN. Therefore, it is periodically read as follows

```
void TaskRunner::timedSnippets() {
	...
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
    ...
}
```



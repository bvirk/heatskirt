#include <string.h>
#define __ASSERT_USE_STDERR
#include <assert.h>
#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>

#include "RiExec.h"
#include "Commands.h"
#include "Sendf.h"
#include "EETimer.h"
#include "TaskRunner.h"


#include "SSD1306.h"


#include "FunctionPointerList.h"



#ifdef SSD1306_h
extern SSD1306 oled;
#endif

extern EETimer& eeTimer;
extern TaskRunner taskRunner;

RiExec::RiExec() {}

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

void RiExec::runCmd() {
    if (cmdFuncIndex < sizeofArray(direct)) 
        exitLevel = (direct[cmdFuncIndex])(argc,argv);
    else {
        sendf("number has no function" );    
        exitLevel=0;
    }
    if (exitLevel)
        sendf("exitlevel=%d\n",exitLevel);
}

void RiExec::loop(const char *line) {
    if (strlen(line) < COMMAND_SIZE ) {
        strcpy(command,line);
        if((argc=splittedCount(command," ")) > MAX_ARG_COUNT)
            argc = MAX_ARG_COUNT;
        catchSplits(argv, command, argc,true);
        runCmd();
    }
    loop();
}

bool RiExec::serialEvent() {
    if (Serial.available()) {
        uint8_t inp = Serial.read();
        if (inp != 10 && inp != 13 && cmdNPos < COMMAND_SIZE-1) // 1. bytes left in buffer end
            command[cmdNPos++] = inp;  
        //if ( inp == 10 || inp == 13 || cmdNPos == COMMAND_SIZE-1) { // sending exatly COMMAND_SIZE-2 should not 
                                                                        // auto execute 
        if ( inp == 10 || inp == 13 ) { 
                                
            // we got line with a command with possible arguments
            // cmdNPos is its length
            // cmdNPos > 0 because it is not alowed for sender to send an 'empty line' 
            // - at least a 1 byte code and cmdNPos==1 is present 
                                                                // cmdNPos==1   | cmdNPos=2
            command[cmdNPos] = '\0';                            //              | != ' '
            argc = 0;                                           //              |
            cmdFuncIndex=command[0]-0x30;                            //              |
            command[0]=' '; // avoids ref to command[-1]        //              |
                            // 5 lines below                    //              |
            for (uint8_t cmdPos=0; cmdPos < cmdNPos; cmdPos++)  //  only that   |
                if (command[cmdPos] == ' ')                     //              |
                    command[cmdPos] = '\0';                     //              |
                else                                            //              |
                    if (cmdPos && command[cmdPos-1] == '\0' ) { //  never runned|
                        argv[argc++] = &command[cmdPos];        //              |
                        if (argc == MAX_ARG_COUNT)              //              |
                            break; // for loop
                    }
            return true;
        } // lineshift or buffer full
    } // serial.available
    return false;
}

int RiExec::splittedCount(char str[], const char* delims) {
	strtok(str,delims);
	int cnt=1;
	while (strtok(NULL,delims))
		cnt++;
	return cnt;
}

void RiExec::catchSplits(char *items[], char str[],int cnt, bool ltrim) {
	char *p = items[0] = str;
	int index=1;
	while (index < cnt) {
		while (*p++);
		if (ltrim) 
			while (*p == ' ')
				p++;
		items[index++]=p;
	}
}

RiExec cmds;

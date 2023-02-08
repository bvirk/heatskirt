#include <Arduino.h>
#include <stdint.h>
#include "SSD1306.h"
#include "I2C.h"
//#include "TaskRunner.h"

//extern TaskRunner taskRunner;
    
SSD1306::SSD1306() {
    i2c.init(toInt(SSD1306_::DEFAULT_ADDRESS));
    uint8_t initcmds[] = {
         toInt(SSD1306_::DISPLAYOFF)
        ,toInt(SSD1306_::SETDISPLAYCLOCKDIV)
        ,0x80
        ,toInt(SSD1306_::SETMULTIPLEX)
        ,0x3F
        ,toInt(SSD1306_::SETDISPLAYOFFSET)
        ,0x00
        ,toInt(SSD1306_::SETSTARTLINE)
        ,toInt(SSD1306_::CHARGEPUMP)
        ,0x14
        ,toInt(SSD1306_::MEMORYMODE)
        ,0x01
        ,(toInt(SSD1306_::SEGREMAP)|0x1)
        ,toInt(SSD1306_::COMSCANDEC)
        ,toInt(SSD1306_::SETCOMPINS)
        ,0x12
        ,toInt(SSD1306_::SETCONTRAST)
        ,0xCF
        ,toInt(SSD1306_::SETPRECHARGE)
        ,0xF1
        ,toInt(SSD1306_::SETVCOMDETECT)
        ,0x40
        ,toInt(SSD1306_::DISPLAYALLON_RESUME)
        ,toInt(SSD1306_::NORMALDISPLAY)
        ,toInt(SSD1306_::DISPLAYON)
    };

    for (int i=0; i<sizeof(initcmds); i++)
        command(initcmds[i]);
}

void SSD1306::fillColByte(uint8_t colByte) {
    command(SSD1306_::MEMORYMODE);
    command(0);
    command(SSD1306_::COLUMNADDR);
    command(0x00);
    command(toInt(SSD1306_::WIDTH) - 1);
    command(SSD1306_::PAGEADDR);
    command(0x00);
    command(toInt(SSD1306_::HEIGHT) - 1);
    for (int16_t  page = 0; page < 8; page++) {
        i2c.start();
        i2c.write(0x40);
        for (uint8_t col=0; col < 128 ; col++)
            i2c.write(colByte);
        i2c.stop();
    } 
    
}
void SSD1306::fill(uint8_t ch) {
    command(SSD1306_::COLUMNADDR);
    command(0x00);
    command(toInt(SSD1306_::WIDTH) - 1);
    command(SSD1306_::PAGEADDR);
    command(0x00);
    command(toInt(SSD1306_::HEIGHT) - 1);
    for (int16_t  page = 0; page < 8; page++) {
        i2c.start();
        i2c.write(0x40);
        for (uint8_t chCnt=1; chCnt <= 21 ; chCnt++) {
            for (int8_t chColPos = 0; chColPos < 6; chColPos++) {
                uint8_t chCol = chColPos == 5 ? 0 : pgm_read_byte(&(font[ch][chColPos]));
                i2c.write(chCol);
            }
            i2c.write(0);i2c.write(0);
        }
        i2c.stop();
    }
}

//uint8_t SSD1306::getLine(bool advance) {
//    puts(lines[invline],0,2*invline,2,false);
//    invline += advance ? 1 : 0;
//    if (invline >= arraySize(lines)) {
//        invline=0;
//        topline=0;
//    }
//    if (topline +3 < invline) { //scroll down
//        topline++;
//    }
//    showLines(false);
//    return invline;
//}

uint8_t SSD1306::showLines(PGM_P const items[],uint8_t topline, uint8_t invline,bool clear) {
    if (clear)
        fill();
    for (int i=0;i<4;i++) {
		char buf[11];
		//memset(buf,32,10);
        buf[10]='\0';
		//puts(buf,0,2*i,2);
        strcpy_P(buf, (PGM_P)pgm_read_word(&items[i+topline]));
        puts(buf,0,2*i,2,invline == i+topline);
    }
    //while(!taskRunner.hasButtonPress());
    return 0;

}

#define irange(varname,start,obend) for(int varname=start; i<obend; i++)

void SSD1306::blok() {
    command(SSD1306_::COLUMNADDR);    
    command(40);    
    command(59);    
    command(SSD1306_::PAGEADDR);    
    command(1);
    command(6);
    i2c.start();
    i2c.write(0x40);
    //for (int i=0; i<20*6; i++)
    irange(i,0,20*6)
        i2c.write(255);
    i2c.stop();
}


uint32_t SSD1306::puts(const char* str, uint8_t xStart, uint8_t yStart, uint8_t scale, bool inverse) {
    switch(scale) {
        case 1:
             if (xStart > 127-4 || yStart > 7)
                return 0;
            break;
        case 2:
            if (xStart > 128-12 || yStart > 6)
                return 0;
            break;
        case 4:
            if (xStart > 128-24 || yStart > 4)
                return 0;
            break;
        default:
            return 0;
    }
    //int memmode = scale != 1;

    uint8_t minLength = min(strlen(str),(unsigned)(128-xStart)/6/scale);
    
    // 1:xStart+6*minLength, 2:xStart+12*minLength-1, 4:xStart+24*minLength-1
    const uint8_t xEnd = xStart+6*scale*minLength-(scale != 1);
    
    // 1:yStart+1, 2:yStart+1, 4:yStart+3
    const uint8_t yEnd = yStart+scale-(scale > 1); 
    
    
    command(SSD1306_::MEMORYMODE);
    command(scale != 1);  // Vertical when scale != 1
    command(SSD1306_::COLUMNADDR);    
    command(xStart);    
    command(xEnd); 
    command(SSD1306_::PAGEADDR);    
    command(yStart);
    command(yEnd);
    i2c.start();
    i2c.write(0x40);

    for (int8_t leadVL = 0; leadVL <scale; leadVL++)
        i2c.write(inverse ? 0xff : 0);
    do {  
        uint8_t fontBuf[5];
        for (int8_t fontCol = 0; fontCol < 5; fontCol++) {
            fontBuf[fontCol] = pgm_read_byte(&(font[*str][fontCol]));
            if (inverse)
                    fontBuf[fontCol] ^= 0xff;
        } 
        for (int8_t chCol = 0; chCol < 6*scale; chCol++) {
            uint8_t chRead = chCol < 5*scale 
                ? fontBuf[chCol/scale] 
                : inverse ? 0xff : 0;
            for (int r=0; r <scale; r++) {
                uint8_t lowgt1 = chRead >> (6-scale)*r;
                uint8_t exploded = scale==4 
                    ? 0xf*(lowgt1 & 1)+0x78*(lowgt1 & 2)
                    : scale== 2
                        ? 3*(lowgt1 & 1)+6*(lowgt1 & 2)+12*(lowgt1 & 4)+24*(lowgt1 & 8)
                        : chRead;
                i2c.write(exploded);
            }
        }
    } while (*(str++) && --minLength);
    i2c.stop();   
    return ((uint32_t)xStart) << 24 | ((uint32_t)yStart) << 16 | ((uint32_t)xEnd) << 8 | yEnd;     
}


void SSD1306::command(uint8_t command) {
    i2c.start();
    i2c.write(0x00);
    i2c.write(command);
    i2c.stop();
}

void SSD1306::command(SSD1306_ cmd) {
    command(toInt(cmd));
}

SSD1306 oled;

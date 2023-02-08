
#include <Arduino.h>
#include <stdint-gcc.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <HardwareSerial.h>
#include <WString.h>
#include "Sendf.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

uint8_t sendfCommon(uint8_t callIndex, const char* fmt, va_list args) {
    const int8_t xorCrypt = 0xf5;
    const char* fmtErr = "\004\xf1\004";
    va_list args2;
    va_copy(args2, args);
    int length = vsnprintf(0, 0, fmt, args);
    if (length > 0) {
        char sendf_text[length+4];
        if (vsnprintf(&sendf_text[2], length+1, fmt, args2)==length) {
            *sendf_text=callIndex;
            *(sendf_text+1)=callIndex ^ xorCrypt;
            *(sendf_text+length+2) = 4;
            *(sendf_text+length+3) = 0;
            Serial.print(sendf_text);
        } else
            Serial.print(fmtErr);
    } else
        Serial.print(fmtErr);
    Serial.flush();
    va_end(args2);
    return length > 0 ? length : 3;
}
/*
uint8_t sendf(rcallIndex index,const char* fmt, ...)
{
    va_list args;
	va_start(args, fmt);
    uint8_t retval = sendfCommon(static_cast<uint8_t>(index),fmt,args);
    va_end(args);
    return retval;
}


uint8_t sendf(const char* fmt, ...)
{
    va_list args;
	va_start(args, fmt);
    uint8_t retval = sendfCommon(static_cast<uint8_t>(DEF),fmt,args);
    va_end(args);
    return retval;
}
*/
uint8_t sendflash(rcallIndex index,const __FlashStringHelper *ffmt, ...) {
	va_list args;
	va_start(args, ffmt);
	
	PGM_P p = reinterpret_cast<PGM_P>(ffmt);
	uint8_t flen = strnlen_P(p,0x40);
	char fmt[flen+1];
	strncpy_P(fmt,p,flen+1);
	fmt[flen] = '\0';
	uint8_t retval = sendfCommon(static_cast<uint8_t>(index),fmt,args);
	va_end(args);
	return retval;
}

uint8_t sendflash(const __FlashStringHelper *ffmt, ...) {
	va_list args;
	va_start(args, ffmt);
	
	PGM_P p = reinterpret_cast<PGM_P>(ffmt);
	uint8_t flen = strnlen_P(p,0x40);
	char fmt[flen+1];
	strncpy_P(fmt,p,flen+1);
	fmt[flen] = '\0';
	uint8_t retval = sendfCommon(static_cast<uint8_t>(rcallIndex::DEF),fmt,args);
	va_end(args);
	return retval;
}

char *rearranged_dtostrf(char *buf,double f, int precision=0, int totalwidth=0) {
    return dtostrf(f,totalwidth,precision,buf);
}

















/*

char double_text[0xf];

inline char * dtostre(double d, uint8_t prec) {
	return dtostre(d,double_text,prec > 7 ? 7 : prec,true);
}

inline char * dtostrf(double d, uint8_t prec) {
	return dtostrf(d,0,prec > 7 ? 7 : prec,double_text);
}

char * (*toStr[])(double, uint8_t) = {dtostre,dtostrf};
*/
/**
  * First %[.d]e or &[.d]f flowing point format code
  *
  * @param fmt to be searched for %[.d]e or &[.d]f strings at start
  * @return -1 for not  %[.d]e or &[.d]f at start of fmt or else
  *	bit0-2 : precision (0-7), bit 3-4: length of (2 or 4 depend on eg. %f or %.4f), bit 5: 0=e, 1=f 
  *	scientific vs decimal number
  
inline int8_t typeLengthPrecision(const char * fmt) {
    if (*fmt == '%') {
        if (*(fmt+1) =='e' || *(fmt+1) =='f')
            return DOUBLE_DEFAULT_PRECISION |  (*(fmt+1) =='f' ? 0x50 : 0x10);
        else
            if(*(fmt+1) == '.' && (*(fmt+3) == 'e' || *(fmt+3) == 'f')) {
                char prec = *(fmt+2);
                return ( isdigit(prec) ? (prec < '8' ? int(prec-48) : 7) : DOUBLE_DEFAULT_PRECISION ) | (*(fmt+3) == 'f' ? 0x60 : 0x20);
            }
    }
    return -1;
}

inline int8_t sendffCommon(const char * fmt, va_list args) {
       
    const int8_t sendf_textSize=0x3f;
    char sendf_text[0x43];
    
    // remote call codes
    *sendf_text = 1;  //default print
    *(sendf_text+1) = 0xf4; // 

    const char *beg = fmt;
    const char *end = fmt;
    char *dest = sendf_text+2;
    int8_t precsn;

    while(*end) {
        while (*end && (precsn=typeLengthPrecision(end)) == -1)            
            end++;
        if (end > beg) { // move, not format specifier containing text 
            memmove(dest,beg,end-beg);
            if (*end == '\0')
            	*(dest+(end-beg))= '\0';
        }
        if (*end) {
            dest += end-beg;
            double d = va_arg(args,double);
            double dAbs = abs(d);
            int isFmtSpecF = precsn >> 6;
            uint8_t preci = precsn & 7;
            if (isFmtSpecF && dAbs != 0 && (dAbs > 9999999*pow(10,-preci) || dAbs < pow(10,preci-8)))
            	isFmtSpecF=0;
            char *flt =  (toStr[isFmtSpecF])(d,preci);
            int fltlen = strlen(flt);
            memmove(dest,flt,fltlen);
            dest +=fltlen;
            end += (precsn >> 3) & 6;
            beg=end;
            if (*end == '\0')
            	*dest='\0';
        }
    }

    // eot code
    uint8_t len = strlen(sendf_text);
    sendf_text[len]='\004';
    sendf_text[len+1]='\0';
    Serial.print(sendf_text);
    
    return len-3;
}

int8_t sendffloat(const __FlashStringHelper *ffmt, ...) {
	va_list args;
    va_start(args, ffmt);
    
   	PGM_P p = reinterpret_cast<PGM_P>(ffmt);
	uint8_t flen = strnlen_P(p,0x40);
	char fmt[flen+1];
	strncpy_P(fmt,p,flen+1);
	fmt[flen] = '\0';
    
	int8_t len = sendffCommon(fmt,args);
	va_end(args);
	return len;
}
*/


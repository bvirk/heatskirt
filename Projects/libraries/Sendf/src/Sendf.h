#ifndef Sendf_h
#define Sendf_h

#define DOUBLE_DEFAULT_PRECISION 2


#if defined(DEF) || defined(RED) || defined(YEL)
#include <stophere> // some macros allready defined
#endif

enum class rcallIndex {DEF=1,RED,ALERT,FMTERR,NPOS};

//uint8_t sendf(rcallIndex index,const char* fmt, ...);
//uint8_t sendf(const char* fmt, ...);
uint8_t sendflash(rcallIndex, const __FlashStringHelper *ffmt, ...);
uint8_t sendflash(const __FlashStringHelper *ffmt, ...);
int8_t  sendffloat(const __FlashStringHelper *ffmt, ...);
char *rearranged_dtostrf(char *buf,double f, int precision=0, int totalwidth=0);

#define sln(p)               sendflash(F("%s"),p)
#define sendf(fmt,      ...) sendflash(F(fmt),##__VA_ARGS__)
#define sendfRED(fmt,   ...) sendflash(rcallIndex::RED,F(fmt),##__VA_ARGS__)
#define sendfALERT(fmt, ...) sendflash(rcallIndex::ALERT,F(fmt),##__VA_ARGS__)
#define sendfFMTERR(fmt,...) sendflash(rcallIndex::FMTERR,F(fmt),##__VA_ARGS__)
#define dtostr(...) ({ char buf[30]; rearranged_dtostrf(buf,__VA_ARGS__); })

#endif
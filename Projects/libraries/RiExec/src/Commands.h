#include <Arduino.h>

namespace cmdFuncs {
uint8_t autoexec(uint8_t argc, char *argv[]);
uint8_t humidity(uint8_t argc, char *argv[]);
uint8_t lsdht22(uint8_t argc, char *argv[]);
uint8_t lsinput(uint8_t argc, char *argv[]);
uint8_t oledputs(uint8_t argc, char *argv[]);
uint8_t oledascii(uint8_t argc, char *argv[]);
uint8_t oledview(uint8_t argc, char *argv[]);
uint8_t out(uint8_t argc, char *argv[]);
uint8_t onewiredetect(uint8_t argc, char *argv[]);
uint8_t play(uint8_t argc, char *argv[]);
uint8_t reset(uint8_t argc, char *argv[]);
uint8_t showargs(uint8_t argc, char *argv[]);
uint8_t showerror(uint8_t argc, char *argv[]);
uint8_t seterror(uint8_t argc, char *argv[]);
uint8_t showTime(uint8_t argc, char *argv[]);
uint8_t taskmaxdelay(uint8_t argc, char *argv[]);
uint8_t toogleassertenable(uint8_t argc, char *argv[]);
uint8_t toogledebug(uint8_t argc, char *argv[]);
uint8_t tone(uint8_t argc, char *argv[]);
};

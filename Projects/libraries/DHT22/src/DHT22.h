/*!
 *  @file DHT.h
 *
 *  This is a library for DHT series of low cost temperature/humidity sensors.
 *
 *  You must have Adafruit Unified Sensor Library library installed to use this
 * class.
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 *
 *  Written by Adafruit Industries.
 *
 *  MIT license, all text above must be included in any redistribution
 */

#ifndef DHT_H
#define DHT_H

#include "Arduino.h"

enum class failCode {NOFAIL,INITLOW,INITHIGH,DATA,CHECKSUM};

/* Uncomment to enable printing out nice debug messages. */
//#define DHT_DEBUG

#define DEBUG_PRINTER                                                          \
  Serial /**< Define where debug output will be printed.                       \
          */

/* Setup debug printing macros. */
#ifdef DHT_DEBUG
#define DEBUG_PRINT(...)                                                       \
  { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...)                                                     \
  { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...)                                                       \
  {} /**< Debug Print Placeholder if Debug is disabled */
#define DEBUG_PRINTLN(...)                                                     \
  {} /**< Debug Print Line Placeholder if Debug is disabled */
#endif


#if defined(TARGET_NAME) && (TARGET_NAME == ARDUINO_NANO33BLE)
#ifndef microsecondsToClockCycles
/*!
 * As of 7 Sep 2020 the Arduino Nano 33 BLE boards do not have
 * microsecondsToClockCycles defined.
 */
#define microsecondsToClockCycles(a) ((a) * (SystemCoreClock / 1000000L))
#endif
#endif

#define VCCON 1
#define VCCOFF 0

/*!
 *  @brief  Class that stores state and functions for DHT
 */
class DHT {
public:
  failCode failCause;
  uint8_t powerOnCnt;
  uint32_t cycleCnt;
  DHT(uint8_t pin, uint8_t vccpin, uint8_t count = 6);
  void begin(uint8_t usec = 55);
  float readTemperature(bool force = false);
  float readHumidity(bool force = false);
  bool read(bool force = false);
  bool readWrapper(bool force = false);

private:
  uint8_t data[5];
  uint8_t _pin, vccpin;
  uint8_t _bit, _port;
  uint32_t _lastreadtime, _maxcycles;
  uint32_t vccOnTime;
  bool _lastresult;
  bool dht22IsOff;
  uint8_t pullTime; // Time (in usec) to pull up data line before reading

  uint32_t expectPulse(bool level);
};

/*!
 *  @brief  Class that defines Interrupt Lock Avaiability
 */
struct InterruptLock {
  InterruptLock() {
    noInterrupts();
  }
  ~InterruptLock() {
    interrupts();
  }
};

#endif

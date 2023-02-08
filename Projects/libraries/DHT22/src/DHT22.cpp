/*!
 *  @file DHT.cpp
 *
 *  @mainpage DHT series of low cost temperature/humidity sensors.
 *
 *  @section intro_sec Introduction
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
 *  @section author Author
 *
 *  Written by Adafruit Industries.
 *
 *  @section license License
 *
 *  MIT license, all text above must be included in any redistribution
 */

#include "DHT22.h"

#define MIN_INTERVAL 2000 /**< min interval value */
#define TIMEOUT UINT32_MAX 

/*!
 *  @brief  Instantiates a new DHT class
 *  @param  pin
 *          pin number that sensor is connected
 *  @param  type
 *          type of sensor
 *  @param  count
 *          number of sensors
 */
DHT::DHT(uint8_t pin, uint8_t vccpin, uint8_t count) 
  : failCause(failCode::NOFAIL),powerOnCnt(1),cycleCnt(0),vccpin(vccpin), vccOnTime(0),dht22IsOff(false) {
  (void)count; // Workaround to avoid compiler warning.
  _pin = pin;
  #ifdef __AVR
    _bit = digitalPinToBitMask(pin);
    _port = digitalPinToPort(pin);
  #endif
  _maxcycles =
      microsecondsToClockCycles(1000); // 1 millisecond timeout for
                                       // reading pulses from DHT sensor.
  // Note that count is now ignored as the DHT reading algorithm adjusts itself
  // based on the speed of the processor.
  begin();
}

/*!
 *  @brief  Setup sensor pins and set pull timings
 *  @param  usec
 *          Optionally pass pull-up time (in microseconds) before DHT reading
 *starts. Default is 55 (see function declaration in DHT.h).
 */
void DHT::begin(uint8_t usec) {
  // set up the pins!
  pinMode(vccpin,OUTPUT);
  digitalWrite(vccpin,VCCON);
  pinMode(_pin, INPUT);
  // Using this value makes sure that millis() - lastreadtime will be
  // >= MIN_INTERVAL right away. Note that this assignment wraps around,
  // but so will the subtraction.
  _lastreadtime = millis() - MIN_INTERVAL;
  DEBUG_PRINT("DHT max clock cycles: ");
  DEBUG_PRINTLN(_maxcycles, DEC);
  pullTime = usec;
}

/*!
 *  @brief  Read temperature
 *  @param  S
 *          Scale. Boolean value:
 *					- true = Fahrenheit
 *					- false = Celcius
 *  @param  force
 *          true if in force mode
 *	@return Temperature value in selected scale
 */
float DHT::readTemperature(bool force) {
  float f = NAN;

  if (readWrapper(force)) {
      f = ((word)(data[2] & 0x7F)) << 8 | data[3];
      f *= 0.1;
      if (data[2] & 0x80) {
        f *= -1;
      }
  }
  return f;
}

/*!
 *  @brief  Read Humidity
 *  @param  force
 *					force read mode
 *	@return float value - humidity in percent
 */
float DHT::readHumidity(bool force) {
  float f = NAN;
  if (readWrapper(force)) {
      f = ((word)data[0]) << 8 | data[1];
      f *= 0.1;
  }
  return f;
}

bool DHT::readWrapper(bool force) {
  int32_t currenttime = millis();
  
  // could be long time after _lastreadtime 
  if (dht22IsOff && currenttime -_lastreadtime > 2 * MIN_INTERVAL && digitalRead(vccpin)==VCCOFF) {
  // But when at least MIN_INTERVAL after dht22 WAS POWERED OFF 
  
    digitalWrite(vccpin,VCCON); // dht22 power on
    powerOnCnt++;
    
    // it is undefined how much currenttime is above _lastreadtime - because of that 
    // we have to save the time of powering dht22 again.
    vccOnTime = currenttime;
    return currenttime - _lastreadtime < 3 * MIN_INTERVAL; 
  }
  
  if (dht22IsOff) {
    // vccOntime both used af flag and time of dht22 powered on
    if (vccOnTime && millis() - vccOnTime > MIN_INTERVAL) {
      // MIN_INTERVAL after dht22 power on, we states it in the variable 
      dht22IsOff=false;
      vccOnTime=0; // as flag cleared
      // now we are back to normal state - hopefully dht22 reboot has fixed it.
    } else
      return currenttime - _lastreadtime < 3 * MIN_INTERVAL;
  }
  
  
  
  // normal functionality
  if (read(force))
    return true;
  

  digitalWrite(vccpin,VCCOFF);
  dht22IsOff = true;
  
  // the read()==false has not spoiled data[] or changed _lastreadtime of last read()==true
  // within 3*MIN_INTERVAL we accepts latest succes reading
  return currenttime - _lastreadtime < 3* MIN_INTERVAL;

}

/*!
 *  @brief  Read value from sensor or return last one from less than two
 *seconds.
 *  @param  force
 *          true if using force mode
 *	@return float value
 */
bool DHT::read(bool force) {
  // Check if sensor was read less than two seconds ago and return early
  // to use last reading.
  uint32_t currenttime = millis();
  if (!force && ((currenttime - _lastreadtime) < MIN_INTERVAL)) {
    return _lastresult; // return last correct measurement
  }
  //_lastreadtime = currenttime;

  // Reset 40 bits of received data to zero.
  //data[0] = data[1] = data[2] = data[3] = data[4] = 0;


  // Send start signal.  See DHT datasheet for full signal diagram:
  //   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

  // Go into high impedence state to let pull-up raise data line level and
  // start the reading process.
  pinMode(_pin, INPUT);
  delay(1);

  // First set data line low for a period according to sensor type
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);

  delayMicroseconds(1100); // data sheet says "at least 1ms"

  uint32_t cycles[80];
  {
    // End the start signal by setting data line high for 40 microseconds.
    pinMode(_pin, INPUT);

    // Delay a moment to let sensor pull data line low.
    delayMicroseconds(pullTime);

    // Now start reading the data line to get the value from the DHT sensor.

    // Turn off interrupts temporarily because the next sections
    // are timing critical and we don't want any interruptions.
    InterruptLock lock;

    // First expect a low signal for ~80 microseconds followed by a high signal
    // for ~80 microseconds again.
    if (expectPulse(LOW) == TIMEOUT) {
      DEBUG_PRINTLN(F("DHT timeout waiting for start signal low pulse."));
      _lastresult = false;
      failCause = failCode::INITLOW;
      return _lastresult;
    }
    if (expectPulse(HIGH) == TIMEOUT) {
      DEBUG_PRINTLN(F("DHT timeout waiting for start signal high pulse."));
      _lastresult = false;
      failCause = failCode::INITHIGH;
      return _lastresult;
    }

    // Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
    // microsecond low pulse followed by a variable length high pulse.  If the
    // high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
    // then it's a 1.  We measure the cycle count of the initial 50us low pulse
    // and use that to compare to the cycle count of the high pulse to determine
    // if the bit is a 0 (high state cycle count < low state cycle count), or a
    // 1 (high state cycle count > low state cycle count). Note that for speed
    // all the pulses are read into a array and then examined in a later step.
    for (int i = 0; i < 80; i += 2) {
      cycles[i] = expectPulse(LOW);
      cycles[i + 1] = expectPulse(HIGH);
    }
  } // Timing critical code is now complete.

  // Inspect pulses and determine which ones are 0 (high state cycle count < low
  // state cycle count), or 1 (high state cycle count > low state cycle count).
  uint8_t tempdata[5];
  for (int i = 0; i < 40; ++i) {
    uint32_t lowCycles = cycles[2 * i];
    uint32_t highCycles = cycles[2 * i + 1];
    if ((lowCycles == TIMEOUT) || (highCycles == TIMEOUT)) {
      DEBUG_PRINTLN(F("DHT timeout waiting for pulse."));
      _lastresult = false;
      failCause = failCode::DATA;
      return _lastresult;
    }
    tempdata[i / 8] <<= 1;
    // Now compare the low and high cycle times to see if the bit is a 0 or 1.
    if (highCycles > lowCycles) {
      // High cycles are greater than 50us low cycle count, must be a 1.
      tempdata[i / 8] |= 1;
    }
    // Else high cycles are less than (or equal to, a weird case) the 50us low
    // cycle count so this must be a zero.  Nothing needs to be changed in the
    // stored data.
  }

  DEBUG_PRINTLN(F("Received from DHT:"));
  DEBUG_PRINT(tempdata[0], HEX);
  DEBUG_PRINT(F(", "));
  DEBUG_PRINT(tempdata[1], HEX);
  DEBUG_PRINT(F(", "));
  DEBUG_PRINT(tempdata[2], HEX);
  DEBUG_PRINT(F(", "));
  DEBUG_PRINT(tempdata[3], HEX);
  DEBUG_PRINT(F(", "));
  DEBUG_PRINT(tempdata[4], HEX);
  DEBUG_PRINT(F(" =? "));
  DEBUG_PRINTLN((tempdata[0] + tempdata[1] + tempdata[2] + tempdata[3]) & 0xFF, HEX);

  // Check we read 40 bits and that the checksum matches.
  if (tempdata[4] == ((tempdata[0] + tempdata[1] + tempdata[2] + tempdata[3]) & 0xFF)) {
    for (int i=0; i < 5; i++)
      data[i]=tempdata[i];
    _lastreadtime = millis();
    _lastresult = true;
    return _lastresult;
  } else {
    DEBUG_PRINTLN(F("DHT checksum failure!"));
    _lastresult = false;
    failCause = failCode::CHECKSUM;
    return _lastresult;
  }
}

//// Expect the signal line to be at the specified level for a period of time and
  // return a count of loop cycles spent at that level (this cycle count can be
  // used to compare the relative time of two pulses).  If more than a millisecond
  // ellapses without the level changing then the call fails with a 0 response.
  // This is adapted from Arduino's pulseInLong function (which is only available
  // in the very latest IDE versions):
  //   https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring_pulse.c
uint32_t DHT::expectPulse(bool level) {
  uint32_t count = 0;
  uint8_t portState = level ? _bit : 0;
  while ((*portInputRegister(_port) & _bit) == portState) {
    cycleCnt++;
    if (count++ >= _maxcycles)
      return TIMEOUT;
  }
  return count;
}

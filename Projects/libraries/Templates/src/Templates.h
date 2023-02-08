#ifndef Templates_h
#define Templates_h

// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.

#include <EEPROM.h>
#include <string.h>

/*!
  It seems function template definitions has to be contained in
  every compilation unit - makes sense it uses defintion to compile 
  the actual types.
  
  Therefore definitions in this header file in a library for common use
  */

/*! Initilalize object from EEPROM memory
  \param address of objects persistent data
  \param object the be initialized
  */
template<typename T>void EEPROMRead(uint16_t address, T & object) {
uint8_t* p = (uint8_t*)(void*)&object;
	for (uint8_t i = 0; i < sizeof(object); i++) 
		*p++ = EEPROM.read(address++);
}                                         


/*! Save value to EEPROM memory
  \param address
  \param value to be saved
  */
template<typename T>void EEPROMWrite(uint16_t address, T value) {
	uint8_t* p = (uint8_t*)(void*)&value;
	for (uint8_t i = 0; i < sizeof(value); i++) {
		if (EEPROM.read(address) != *p) 
			EEPROM.write(address, *p);
		address++,
		p++;
	}
}


/*! Read value from EEPROM memory 
  \param address of least significant byte of value
  \return value
  */
template<typename T>T EEPROMValue(uint16_t address) {
	T value;
	uint8_t* p = (uint8_t*)(void*)&value;
	for (uint8_t i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(address++);
	return value;
}

template< typename T>struct enumStr {
	T d;
	const char *name;
	uint32_t val;
	static int quantity(int advance) { static int length=0; length += advance; return length;}
	enumStr(T d, const char* name) : d(d),name(name),val(0) {
		enumStr::quantity(1);
	}
	enumStr(T d, const char* name, uint32_t val) : d(d),name(name),val(val) {
		enumStr::quantity(1);
	}
	enumStr(T d) : d(d),name(""),val(0) {
		enumStr::quantity(1);
	}
}; 

template<typename T,typename U> T findEnum(const char* dyrName, U myv[]) {
	for (int i=0; i < U::quantity(0) ; i++ ) 
//	for (int i=0; i < 3 ; i++ ) 
		if (!strcasecmp(myv[i].name,dyrName))
			return myv[i].d;
	return static_cast<T>(0);
}

template<typename T,typename U>const char *findEnumStr(T e, U myv[]) {
	for (int i=0; i < U::quantity(0) ; i++ )
		if (e == myv[i].d)
			return myv[i].name;
	return ""; //dummy
}

template<typename T,typename U>uint32_t findEnumUInt32(T e, U myv[]) {
	for (int i=0; i < U::quantity(0) ; i++ )
		if (e == myv[i].d)
			return myv[i].val;
	return 0; //dummy
}


#endif
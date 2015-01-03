/*
 * adruino_pins2.h
 *
 *  Created on: 11. 2. 2014
 *      Author: Jan Dolinay
 *
 *  Alternate version of digital input/output for Arduino.
 *  This is file for Arduino Standard (Uno).
 *
 *  Howto port this file to another Arduino variant:
 *  1) Define GPIO_pin_t enum with pin codes
 *  2) Set the number of pins to proper value in GPIO_PINS_NUMBER 
 *  3) Check/modify the macros for getting addresses of I/O registers
 *  4) Define the gpio_pins_progmem array with the values from GPIO_pin_t enum
 *  
 *  Notes:
 *  Step 1) you need the datasheet of the AVR MCU used in your variant. Check the
 *  addresses of the registers for controlling GPIO ports (typically there is
 *  "Register summary" chapter in the datasheet with the addresses. 
 *  See the definition below for Atmega328 as an example.
 *  Step 3) if the addresses of all the GPIO registers are lower than 0xFF, you 
 *  can use the simple macros as defined here.
 *  If there are some registers with higher address (such as the case in 
 *  Atmega 2560 used in Arduino Mega), we need to encode the address into
 *  single byte or use a different approach. Use the macros defined in 
 *  pins2_arduino.h for Arduino mega.
 *
 *
  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
 *
 */

#ifndef ADRUINO_PINS2_H_
#define ADRUINO_PINS2_H_


// ===========================================================================
// Configuration
// ===========================================================================
// GPIO2_PREFER_SPEED - User can define this value before including arduino2.h
// or this default value is used.
// Set to 1 to make all digital I/O functions inline.
// This will make them work faster but the resulting code will be bigger.
// In general, you can start with 1 and if you are running out of program memory
// switch to 0.
//
#ifndef GPIO2_PREFER_SPEED
	#define		GPIO2_PREFER_SPEED	1
#endif


// GPIO2_IOREGS_ABOVEFF - set to one if the Atmega MCU used for this Arduino 
// variant has some I/O registers at address higher than 0xff 
// (e.g. Atmega2560 in Arduino Mega).
// In this case we must always disable interrupts in digitalWrite and pinMode,
// because the registers will not be manipulated by single instruction (SBI, CBI),
// because these instructions only work on locations below 0xFF.
//
// For Arduino Standard (ATmega328) set to 0
// For Arduino Mega (ATmega1280 or ATmega2560) set to 1
// If not sure, set to 1 (this is always safe option)
//
#define	  GPIO2_IOREGS_ABOVEFF	0


// Helper macro to create the pin code from port register address and pin number.
// Note that this is not used in runtime; just in compile time to create the
// enum with pin codes.
#define	GPIO_MAKE_PINCODE(port, pin)  (((uint16_t)port & 0x00FF) | ((1<<pin) << 8))

// Unfortunately we cannot use the PORTA etc. definitions from avr/io.h in the
// GPIO_MAKE_PINCODE macro, so I define the port register addresses here to make
// it more comfortable to define the pins
// This is the address of the port register, e.g. PORTA or PORTB, from the datasheet.
#define		MYPORTB		(0x25)
#define		MYPORTC		(0x28)
#define		MYPORTD		(0x2B)


/*
 * GPIO_pin_t
 * Define the type for digital I/O pin.
 * We will not use simple integer (int) to identify a pin.
 * Instead we use special code, which contains the address of the I/O register
 * for given pin (lower byte) together with its bit mask (upper byte).
 * For this code we create our special data type (GPIO_pin_t) which will prevent
 * the user from calling our digitalRead/Write with invalid pin numbers.
 *
*/
enum GPIO_pin_enum
{
	// Note: The invalid value can be 0 which means digitalWrite will write to
    // reserved address on Atmega 328 used in Arduino Uno,
	// or it can be any valid port register - as long as the bit mask in upper 
    // byte is 0, the operation on this register will have no effect.
	DP_INVALID = 0x0025,
	DP0 = GPIO_MAKE_PINCODE(MYPORTD,0),
	DP1 = GPIO_MAKE_PINCODE(MYPORTD,1),
	DP2 = GPIO_MAKE_PINCODE(MYPORTD,2),
	DP3 = GPIO_MAKE_PINCODE(MYPORTD,3),
	DP4 = GPIO_MAKE_PINCODE(MYPORTD,4),
	DP5 = GPIO_MAKE_PINCODE(MYPORTD,5),
	DP6 = GPIO_MAKE_PINCODE(MYPORTD,6),
	DP7 = GPIO_MAKE_PINCODE(MYPORTD,7),
	DP8 = GPIO_MAKE_PINCODE(MYPORTB,0),
	DP9 = GPIO_MAKE_PINCODE(MYPORTB,1),
	DP10 = GPIO_MAKE_PINCODE(MYPORTB,2),
	DP11 = GPIO_MAKE_PINCODE(MYPORTB,3),
	DP12 = GPIO_MAKE_PINCODE(MYPORTB,4),
	DP13 = GPIO_MAKE_PINCODE(MYPORTB,5),
	DP14 = GPIO_MAKE_PINCODE(MYPORTC,0),
	DP15 = GPIO_MAKE_PINCODE(MYPORTC,1),
	DP16 = GPIO_MAKE_PINCODE(MYPORTC,2),
	DP17 = GPIO_MAKE_PINCODE(MYPORTC,3),
	DP18 = GPIO_MAKE_PINCODE(MYPORTC,4),
	DP19 = GPIO_MAKE_PINCODE(MYPORTC,5),
};

typedef	enum GPIO_pin_enum  GPIO_pin_t;

// Number of GPIO pins.
// Used in Arduino_to_GPIO_pin function
#define		GPIO_PINS_NUMBER		(20)

/*   
// "raw" version of the pin definition using port address and mask directly
enum GPIO_pin_t
{
	// Note: The invalid value can be 0 which means digitalWrite will write to reserved address
	// on Atmega 328 used in Arduino Uno,
	// or it can be any valid port register - as long as the bit mask in upper byte is 0, the
	// operation on this register will have no effect.
	DPINVALID = 0x0025,
	DP0 = 0x012B,
	DP1 = 0x022B,
	DP2 = 0x042B,
	DP3 = 0x082B,
	DP4 = 0x102B,
	DP5 = 0x202B,
	DP6 = 0x402B,
	DP7 = 0x802B,
	DP8 = 0x0125,
	DP9 = 0x0225,
	DP10 = 0x0425,
	DP11 = 0x0825,
	DP12 = 0x1025,
	DP13 = 0x2025,
	DP14 = 0x0128,
	DP15 = 0x0228,
	DP16 = 0x0428,
	DP17 = 0x0828,
	DP18 = 0x1028,
	DP19 = 0x2028,
};
*/


// Macro to obtain bit mask of a pin from its code
#define		GPIO_PIN_MASK(pin)		((uint8_t)((uint16_t)pin >> 8))

// Macros to obtain the addresses of various I/O registers from pin code
#define		GET_PORT_REG_ADR(pin)		((volatile uint8_t*)((pin) & 0x00FF))
#define		GET_PIN_REG_ADR(pin)		(GET_PORT_REG_ADR(pin)-2)
#define		GET_DDR_REG_ADR(pin)		(GET_PORT_REG_ADR(pin)-1)

// Macros which allow us to refer to the I/O registers directly
#define   GPIO_PIN_REG(pin)    (*(volatile uint8_t*)GET_PIN_REG_ADR(pin) )
#define   GPIO_PORT_REG(pin)    (*(volatile uint8_t*)GET_PORT_REG_ADR(pin) )
#define   GPIO_DDR_REG(pin)    (*(volatile uint8_t*)GET_DDR_REG_ADR(pin) )


// ARDUINO2_MAIN should be defined only once in the program so that the
// gpio_pins_progmem is not duplicated. This is done in digital2.c
#ifdef ARDUINO2_MAIN

// This array in program memory (FLASH) maps Arduino pin number (simple integer)
// to our GPIO pin definition.
// The Arduino pin number is used as index into this array. Value at given index
// N is the pin code for the Arduino pin N.
const GPIO_pin_t PROGMEM gpio_pins_progmem[] = {
		DP0, DP1, DP2, DP3,
		DP4, DP5, DP6, DP7,
		DP8, DP9, DP10, DP11,
		DP12, DP13, DP14, DP15,
		DP16, DP17, DP18, DP19,
};
#else
	const extern GPIO_pin_t PROGMEM gpio_pins_progmem[];
#endif


#endif /* ADRUINO_PINS2_H_ */

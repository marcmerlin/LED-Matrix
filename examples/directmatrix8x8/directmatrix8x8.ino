/*************************************************** 
    This is a library to address LED matrices that requires
    constant column/row rescans.

    It uses code from the Adafruit I2C LED backpack library designed for
    ----> http://www.adafruit.com/products/881
    ----> http://www.adafruit.com/products/880
    ----> http://www.adafruit.com/products/879
    ----> http://www.adafruit.com/products/878

    Adafruit invests time and resources providing this open source code, 
    please support Adafruit and open-source hardware by purchasing 
    products from Adafruit!

    Original code written by Limor Fried/Ladyada for Adafruit Industries.  
    BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "LED_Matrix.h"

// I shouldn't have to re-include these libs included in LED_Matrix.h
// but I get
// LED_Matrix.h:10:19: fatal error: Wire.h: No such file or directory  #include <Wire.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <TimerOne.h>

#define DEBUG 0

// ----------------------------------------------------------------------------
#ifndef FASTIO
#define DATA_PIN DINV
#define CLK_PIN DINV
#define LATCH1_PIN DINV
#define LATCH2_PIN DINV
#define LATCH3_PIN DINV

// These go to ground:
GPIO_pin_t gnd_line_pins[] = { 5, 6, 7, 8, 12, 11, 10, 9 };
// Those go to V+
// A6 and A7 do NOT work as digital pins on Arduino Nano
// Red LEDs are directly connected.
// Green LEDs are connected via shift register
GPIO_pin_t column_pins[] = {  0,  4, A5, A4, A3, A2, A1, A0,
                              DINV, DINV, DINV, DINV, DINV, DINV, DINV, DINV,
                              DINV, DINV, DINV, DINV, DINV, DINV, DINV, DINV, };

// ----------------------------------------------------------------------------
#else
#define DATA_PIN DINV
#define CLK_PIN DINV
#define LATCH1_PIN DINV
#define LATCH2_PIN DINV
#define LATCH3_PIN DINV

GPIO_pin_t gnd_line_pins[] = { DP5, DP6, DP7, DP8, DP12, DP11, DP10, DP9 };

GPIO_pin_t column_pins[] = {  DP0,  DP4, DP19, DP18, DP17, DP16, DP15, DP14,
                              DINV, DINV, DINV, DINV, DINV, DINV, DINV, DINV,
                              DINV, DINV, DINV, DINV, DINV, DINV, DINV, DINV, };
#endif
// ----------------------------------------------------------------------------

// no shift register in single color test, all latches are set to invalid pin
GPIO_pin_t sr_pins[] = { DINV, DINV, DINV, DATA_PIN, CLK_PIN };

PWMDirectMatrix *matrix;

void setup() {
    // Initializing serial breaks one row (shared pin)
    if (DEBUG) Serial.begin(57600);
    if (DEBUG) while (!Serial);
    if (DEBUG) Serial.println("DirectMatrix Test");

    matrix = new PWMDirectMatrix(8, 8, 1);
    // The ISR frequency is doubled 3 times to create 4 PWM values
    // and will run at x, x*2, x*4, x*16 to simulate 16 levels of
    // intensity without causing 16 interrupts at x, leaving more
    // time for the main loop and causing less intensity loss.
    // 200 flickers a bit for me due to the 1600us 4th scan, 150 removes
    // the flicker for my eyes.
    matrix->begin(gnd_line_pins, column_pins, sr_pins, 150);
}

static const uint8_t PROGMEM
    smile_bmp[] =
    { B00111100,
        B01000010,
        B10100101,
        B10000001,
        B10100101,
        B10011001,
        B01000010,
        B00111100 },
    neutral_bmp[] =
    { B00111100,
        B01000010,
        B10100101,
        B10000001,
        B10111101,
        B10000001,
        B01000010,
        B00111100 },
    frown_bmp[] =
    { B00111100,
        B01000010,
        B10100101,
        B10000001,
        B10011001,
        B10100101,
        B01000010,
        B00111100 };

static const uint16_t PROGMEM
    RGB_bmp[][64] = {
      // 16 shades repeated 4 times for 64 LEDs
      { 0x000, 0x000, 0x000, 0x000, 0x001, 0x001, 0x001, 0x001, 
        0x002, 0x002, 0x002, 0x002, 0x003, 0x003, 0x003, 0x003, 
	0x004, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x005, 
	0x006, 0x006, 0x006, 0x006, 0x007, 0x007, 0x007, 0x007, 
	0x008, 0x008, 0x008, 0x008, 0x009, 0x009, 0x009, 0x009, 
	0x00A, 0x00A, 0x00A, 0x00A, 0x00B, 0x00B, 0x00B, 0x00B, 
	0x00C, 0x00C, 0x00C, 0x00C, 0x00D, 0x00D, 0x00D, 0x00D, 
	0x00E, 0x00E, 0x00E, 0x00E, 0x00F, 0x00F, 0x00F, 0x00F, } };

void show_isr() {
    if (DEBUG) Serial.print  (F("ISR runtime: "));
    if (DEBUG) Serial.print  (matrix->ISR_runtime());
    if (DEBUG) Serial.print  (F(" and latency: "));
    if (DEBUG) Serial.println(matrix->ISR_latency());
}

void loop() {
    for (uint8_t i=0; i<=0; i++)
    {
	show_isr();
	matrix->clear();
	matrix->drawRGBBitmap(0, 0, RGB_bmp[i], 8, 8);
	matrix->writeDisplay();
	delay(4000);
    }

    show_isr();
    matrix->clear();
    matrix->drawBitmap(0, 0, smile_bmp, 8, 8, LED_RED_HIGH);
    matrix->writeDisplay();
    delay(1000);

    show_isr();
    matrix->clear();
    matrix->drawBitmap(0, 0, neutral_bmp, 8, 8, LED_RED_MEDIUM);
    matrix->writeDisplay();
    delay(1000);

    show_isr();
    matrix->clear();
    matrix->drawBitmap(0, 0, frown_bmp, 8, 8, LED_RED_LOW);
    matrix->writeDisplay();
    delay(1000);

    show_isr();
    matrix->clear();
    matrix->drawCircle(3,3, 3, LED_RED_MEDIUM);
    matrix->writeDisplay();
    delay(500);

    matrix->setTextWrap(false);  // we don't wrap text so it scrolls nicely
    matrix->setTextSize(1);
    matrix->setTextColor(LED_RED_HIGH);
    matrix->setRotation(3);
    for (int8_t x=7; x>=-36; x--) {
        matrix->clear();
        matrix->setCursor(x,0);
        matrix->print("Hello");
        matrix->writeDisplay();
	delay(50);
    }
    delay(100);
    matrix->setRotation(0);
    matrix->setTextColor(LED_RED_LOW);
    for (int8_t x=7; x>=-36; x--) {
        matrix->clear();
        matrix->setCursor(x,0);
        matrix->print("World");
        matrix->writeDisplay();
	delay(50);
    }
    matrix->setRotation(0);
}

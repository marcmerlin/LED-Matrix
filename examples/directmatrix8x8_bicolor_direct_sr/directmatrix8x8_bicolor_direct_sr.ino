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

#include <Wire.h>
#include "LED_Matrix.h"
#include "Adafruit_GFX.h"
#include "TimerOne.h"

#define DATA_PIN 3
#define CLK_PIN 2
#define LATCH1_PIN -1
#define LATCH2_PIN 13
#define LATCH3_PIN -1

// These go to ground:
uint8_t gnd_line_pins[] = { 5, 6, 7, 8, 12, 11, 10, 9 };

// Those go to V+
// A6 and A7 do NOT work as digital pins on Arduino Nano
// Red LEDs are directly connected.
// Green LEDs are connected via shift register
uint8_t column_pins[] = {  0,  4, A5, A4, A3, A2, A1, A0,
                          -1, -1, -1, -1, -1, -1, -1, -1,
                          -1, -1, -1, -1, -1, -1, -1, -1, };

// -1 -> Red is directly connected
// LATCH2_PIN -> Latch pin for green
// -1 -> no blue
uint8_t sr_pins[] = { -1, LATCH2_PIN, -1, DATA_PIN, CLK_PIN };

PWMDirectMatrix *matrix;

void setup() {
#ifndef __AVR_ATtiny85__
    // Initializing serial breaks one row (shared pin)
    //Serial.begin(57600);
    //Serial.println("DirectMatrix Test");
#endif
    matrix = new PWMDirectMatrix(8, 8, 2);
    matrix->begin(gnd_line_pins, column_pins, sr_pins);
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


void loop() {

    matrix->clear();
    matrix->drawBitmap(0, 0, smile_bmp, 8, 8, LED_RED_HIGH);
    matrix->writeDisplay();
    delay(1000);

    matrix->clear();
    matrix->drawBitmap(0, 0, neutral_bmp, 8, 8, LED_GREEN_HIGH);
    matrix->writeDisplay();
    delay(1000);

    matrix->clear();
    matrix->drawBitmap(0, 0, frown_bmp, 8, 8, LED_ORANGE_HIGH);
    matrix->writeDisplay();
    delay(1000);

    matrix->clear();
    matrix->drawLine(0,0, 7,7, LED_GREEN_LOW);
    matrix->writeDisplay();  // write the changes we just made to the display
    delay(500);

    matrix->clear();
    matrix->drawRect(0,0, 8,8, LED_ORANGE_HIGH);
    matrix->drawRect(1,1, 6,6, LED_GREEN_MEDIUM);
    matrix->fillRect(2,2, 4,4, LED_RED_HIGH);
    matrix->writeDisplay();  // write the changes we just made to the display
    delay(3000);

    matrix->clear();
    matrix->drawCircle(3,3, 3, LED_RED_MEDIUM);
    matrix->writeDisplay();  // write the changes we just made to the display
    delay(500);

    matrix->setTextWrap(false);  // we don't want text to wrap so it scrolls nicely
    matrix->setTextSize(1);
    matrix->setTextColor(LED_GREEN_HIGH);
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
    matrix->setTextColor(LED_ORANGE_HIGH);
    for (int8_t x=7; x>=-36; x--) {
        matrix->clear();
        matrix->setCursor(x,0);
        matrix->print("World");
        matrix->writeDisplay();
	delay(50);
    }
    matrix->setRotation(0);
}

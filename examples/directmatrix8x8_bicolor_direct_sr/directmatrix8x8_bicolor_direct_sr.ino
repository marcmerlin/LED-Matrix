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
#define DATA_PIN 3
#define CLK_PIN 2
#define LATCH1_PIN DINV
#define LATCH2_PIN 13
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
#define DATA_PIN DP3
#define CLK_PIN DP2
#define LATCH1_PIN DINV
#define LATCH2_PIN DP13
#define LATCH3_PIN DINV

GPIO_pin_t gnd_line_pins[] = { DP5, DP6, DP7, DP8, DP12, DP11, DP10, DP9 };

GPIO_pin_t column_pins[] = {  DP0,  DP4, DP19, DP18, DP17, DP16, DP15, DP14,
                              DINV, DINV, DINV, DINV, DINV, DINV, DINV, DINV,
                              DINV, DINV, DINV, DINV, DINV, DINV, DINV, DINV, };
#endif
// ----------------------------------------------------------------------------

// DINV -> Red is directly connected
// LATCH2_PIN -> Latch pin for green
// DINV -> no blue
GPIO_pin_t sr_pins[] = { DINV, LATCH2_PIN, DINV, DATA_PIN, CLK_PIN };

PWMDirectMatrix *matrix;

void setup() {
    // Initializing serial breaks one row (shared pin)
    if (DEBUG) Serial.begin(57600);
    if (DEBUG) while (!Serial);
    if (DEBUG) Serial.println("DirectMatrix Test");

    matrix = new PWMDirectMatrix(8, 8, 2);
    // The ISR frequency is doubled 3 times to create 4 PWM values
    // and will run at x, x*2, x*4, x*16 to simulate 16 levels of
    // intensity without causing 16 interrupts at x, leaving more
    // time for the main loop and causing less intensity loss.
    matrix->begin(gnd_line_pins, column_pins, sr_pins, 200);
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


void show_isr() {
    if (DEBUG) Serial.print  (F("ISR runtime: "));
    if (DEBUG) Serial.print  (matrix->ISR_runtime());
    if (DEBUG) Serial.print  (F(" and latency: "));
    if (DEBUG) Serial.println(matrix->ISR_latency());
}

void loop() {
    show_isr();
    matrix->clear();
    matrix->drawLine(0,2, 7,2, LED_RED_VERYLOW);
    matrix->drawLine(0,3, 7,3, LED_RED_LOW);
    matrix->drawLine(0,4, 7,4, LED_RED_MEDIUM);
    matrix->drawLine(0,5, 7,5, LED_RED_HIGH);
    matrix->drawLine(2,0, 2,7, LED_GREEN_VERYLOW);
    matrix->drawLine(3,0, 3,7, LED_GREEN_LOW);
    matrix->drawLine(4,0, 4,7, LED_GREEN_MEDIUM);
    matrix->drawLine(5,0, 5,7, LED_GREEN_HIGH);
    matrix->writeDisplay();
    delay(4000);

    show_isr();
    matrix->clear();
    matrix->drawBitmap(0, 0, smile_bmp, 8, 8, LED_RED_HIGH);
    matrix->writeDisplay();
    delay(1000);

    show_isr();
    matrix->clear();
    matrix->drawBitmap(0, 0, neutral_bmp, 8, 8, LED_GREEN_HIGH);
    matrix->writeDisplay();
    delay(1000);

    show_isr();
    matrix->clear();
    matrix->drawBitmap(0, 0, frown_bmp, 8, 8, LED_ORANGE_HIGH);
    matrix->writeDisplay();
    delay(1000);

    show_isr();
    matrix->clear();
    matrix->drawRect(0,0, 8,8, LED_ORANGE_HIGH);
    matrix->drawRect(1,1, 6,6, LED_GREEN_MEDIUM);
    matrix->fillRect(2,2, 4,4, LED_RED_HIGH);
    matrix->writeDisplay();
    delay(3000);

    show_isr();
    matrix->clear();
    matrix->drawCircle(3,3, 3, LED_RED_MEDIUM);
    matrix->writeDisplay();
    delay(500);

    matrix->setTextWrap(false);  // we don't wrap text so it scrolls nicely
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

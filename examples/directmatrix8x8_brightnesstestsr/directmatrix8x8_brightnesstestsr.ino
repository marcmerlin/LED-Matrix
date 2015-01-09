/*************************************************** 
This code tests the brightness of the matrix when all the LEDs
are left on without scanning vs the brightness under scan
 ****************************************************/

#include "LED_Matrix.h"

// I shouldn't have to re-include these libs included in LED_Matrix.h
// but I get
// LED_Matrix.h:10:19: fatal error: Wire.h: No such file or directory  #include <Wire.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <TimerOne.h>

#define DEBUG 0

#define DATA_PIN DP3
#define CLK_PIN DP2
#define LATCH1_PIN DINV
#define LATCH2_PIN DP13
#define LATCH3_PIN DINV

GPIO_pin_t line_pins[] = { DP5, DP6, DP7, DP8, DP12, DP11, DP10, DP9 };

GPIO_pin_t column_pins[] = {  DP0,  DP4, DP19, DP18, DP17, DP16, DP15, DP14,
                              DINV, DINV, DINV, DINV, DINV, DINV, DINV, DINV,
                              DINV, DINV, DINV, DINV, DINV, DINV, DINV, DINV, };

// DINV -> Red is directly connected
// LATCH2_PIN -> Latch pin for green
// DINV -> no blue
GPIO_pin_t sr_pins[] = { DINV, LATCH2_PIN, DINV, DATA_PIN, CLK_PIN };

PWMDirectMatrix *matrix;

void setup() {
    // Turn on all the LEDs
    // I need to set the RED LEDs to output or they can prevent the
    // greens from displaying
    for (uint8_t i = 0; i <= 8; i++)
    {
	pinMode(column_pins[i], OUTPUT);
	digitalWrite(column_pins[i], LOW);
    }
    for (uint8_t i = 0; i <= 8; i++)
    {
	pinMode(line_pins[i], OUTPUT);
	digitalWrite(line_pins[i], LOW);
    }
    pinMode(sr_pins[LATCH2_PIN], OUTPUT);
    pinMode(sr_pins[DATA_PIN], OUTPUT);
    pinMode(sr_pins[CLK_PIN], OUTPUT);
    digitalWrite(sr_pins[LATCH2_PIN], LOW);
    for (uint8_t i = 0; i <= 8; i++)
    {
	digitalWrite(sr_pins[CLK_PIN], LOW);
	digitalWrite(sr_pins[DATA_PIN], 1);
	digitalWrite(sr_pins[CLK_PIN], HIGH);
    }
    digitalWrite(sr_pins[LATCH2_PIN], HIGH);

    delay(1000);

    matrix = new PWMDirectMatrix(8, 8, 2);
    matrix->begin(line_pins, column_pins, sr_pins, 200);

    matrix->clear();
    matrix->fillRect(0,0, 8,8, LED_GREEN_HIGH);
    matrix->writeDisplay();
}

void loop() { }

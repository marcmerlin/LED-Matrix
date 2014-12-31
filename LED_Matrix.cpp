/*************************************************** 
  This library was heavily inspired from Adafruit's LED-Backpack-Library

  It was originally designed for these products:
  ----> http://www.adafruit.com/products/
  ----> http://www.adafruit.com/products/

  I used the library shell to write my own library to support LED Matrices
  directly addressed with an X/Y pin matrix.

  By Marc MERLIN <marc_soft@merlins.org>

  Original library written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifdef __AVR_ATtiny85__
 #include <TinyWireM.h>
 #define Wire TinyWireM
#else
 #include <Wire.h>
#endif
#include "LED_Matrix.h"
#include "Adafruit_GFX.h"

// Direct addressing PWM classes use a timer to scan pixels
#include "TimerOne.h"

static const uint16_t alphafonttable[] PROGMEM =  {

0b0000000000000001,
0b0000000000000010,
0b0000000000000100,
0b0000000000001000,
0b0000000000010000,
0b0000000000100000,
0b0000000001000000,
0b0000000010000000,
0b0000000100000000,
0b0000001000000000,
0b0000010000000000,
0b0000100000000000,
0b0001000000000000,
0b0010000000000000,
0b0100000000000000,
0b1000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0001001011001001,
0b0001010111000000,
0b0001001011111001,
0b0000000011100011,
0b0000010100110000,
0b0001001011001000,
0b0011101000000000,
0b0001011100000000,
0b0000000000000000, //  
0b0000000000000110, // !
0b0000001000100000, // "
0b0001001011001110, // #
0b0001001011101101, // $
0b0000110000100100, // %
0b0010001101011101, // &
0b0000010000000000, // '
0b0010010000000000, // (
0b0000100100000000, // )
0b0011111111000000, // *
0b0001001011000000, // +
0b0000100000000000, // ,
0b0000000011000000, // -
0b0000000000000000, // .
0b0000110000000000, // /
0b0000110000111111, // 0
0b0000000000000110, // 1
0b0000000011011011, // 2
0b0000000010001111, // 3
0b0000000011100110, // 4
0b0010000001101001, // 5
0b0000000011111101, // 6
0b0000000000000111, // 7
0b0000000011111111, // 8
0b0000000011101111, // 9
0b0001001000000000, // :
0b0000101000000000, // ;
0b0010010000000000, // <
0b0000000011001000, // =
0b0000100100000000, // >
0b0001000010000011, // ?
0b0000001010111011, // @
0b0000000011110111, // A
0b0001001010001111, // B
0b0000000000111001, // C
0b0001001000001111, // D
0b0000000011111001, // E
0b0000000001110001, // F
0b0000000010111101, // G
0b0000000011110110, // H
0b0001001000000000, // I
0b0000000000011110, // J
0b0010010001110000, // K
0b0000000000111000, // L
0b0000010100110110, // M
0b0010000100110110, // N
0b0000000000111111, // O
0b0000000011110011, // P
0b0010000000111111, // Q
0b0010000011110011, // R
0b0000000011101101, // S
0b0001001000000001, // T
0b0000000000111110, // U
0b0000110000110000, // V
0b0010100000110110, // W
0b0010110100000000, // X
0b0001010100000000, // Y
0b0000110000001001, // Z
0b0000000000111001, // [
0b0010000100000000, // 
0b0000000000001111, // ]
0b0000110000000011, // ^
0b0000000000001000, // _
0b0000000100000000, // `
0b0001000001011000, // a
0b0010000001111000, // b
0b0000000011011000, // c
0b0000100010001110, // d
0b0000100001011000, // e
0b0000000001110001, // f
0b0000010010001110, // g
0b0001000001110000, // h
0b0001000000000000, // i
0b0000000000001110, // j
0b0011011000000000, // k
0b0000000000110000, // l
0b0001000011010100, // m
0b0001000001010000, // n
0b0000000011011100, // o
0b0000000101110000, // p
0b0000010010000110, // q
0b0000000001010000, // r
0b0010000010001000, // s
0b0000000001111000, // t
0b0000000000011100, // u
0b0010000000000100, // v
0b0010100000010100, // w
0b0010100011000000, // x
0b0010000000001100, // y
0b0000100001001000, // z
0b0000100101001001, // {
0b0001001000000000, // |
0b0010010010001001, // }
0b0000010100100000, // ~
0b0011111111111111,

};




/*********************** PWM DIRECT MATRIX OBJECT */

// Globals required to pass matrix data into the ISR.
// (volatile is required for ISRs)
volatile uint8_t DirectMatrix_ARRAY_ROWS;
volatile uint8_t DirectMatrix_ARRAY_COLS;
volatile uint8_t *DirectMatrix_MATRIX;
// These go to ground:
volatile uint8_t *DirectMatrix_ROW_PINS;
// Those go to V+
volatile uint8_t *DirectMatrix_COL_PINS;

// ISR to refresh one matrix line
// This must be fast since it blocks interrupts and can only use globals.
void DirectMatrix_RefreshPWMLine(void) {
    static uint8_t row = 0;
    static uint8_t pwm = 1;
    int8_t oldrow = row - 1;

    if (row == 0) oldrow = DirectMatrix_ARRAY_ROWS - 1;
    // Before setting the columns, shut off the previous row
    digitalWrite(DirectMatrix_ROW_PINS[oldrow], HIGH);

    // TODO: detect that pin0 is -1 and switch to SR mode
    // TODO: scan matrix 2nd sets of colors and populate 2nd array
    for (int8_t col = DirectMatrix_ARRAY_COLS - 1; col >= 0; col--)
    {
	if (DirectMatrix_MATRIX[row * DirectMatrix_ARRAY_COLS + col] >= pwm)
	{
	    digitalWrite(DirectMatrix_COL_PINS[col], HIGH);
	}
	else
	{
	    digitalWrite(DirectMatrix_COL_PINS[col], LOW);
	}
    }
    // Now that the colums are set, turn the row on
    digitalWrite(DirectMatrix_ROW_PINS[row], LOW);

    row++;
    if (row >= DirectMatrix_ARRAY_ROWS)
    {
	row = 0;
	pwm++;
	if (pwm > DirectMatrix_PWM_LEVELS) pwm = 1;
    }
}

DirectMatrix::DirectMatrix(uint8_t num_rows, uint8_t num_cols) {
    _num_rows = num_rows;
    _num_cols = num_cols;

    // These need to be global so that the ISR can get to them.
    DirectMatrix_ARRAY_ROWS = num_rows;
    DirectMatrix_ARRAY_COLS = num_cols;

    if (! (_matrix = (uint8_t *) malloc(num_rows * num_cols)))
    {
	while (1) {
	    Serial.println(F("Malloc failed in DirectMatrix::DirectMatrix"));
	}
    }
    DirectMatrix_MATRIX = _matrix;
}

// Array of of pins for vertical lines, and columns.
void DirectMatrix::begin(uint8_t __row_pins[], uint8_t __col_pins[]) {
    _row_pins = __row_pins;
    _col_pins = __col_pins;

    // These need to be global so that the ISR can get to them
    DirectMatrix_ROW_PINS = _row_pins;
    DirectMatrix_COL_PINS = _col_pins;

    // Init the lines and cols with the opposite voltage to turn them off.
    for (uint8_t i = 0; i < _num_rows; i++)
    {
	pinMode(_row_pins[i], OUTPUT);
	digitalWrite(_row_pins[i], HIGH);
    }
    for (uint8_t i = 0; i < _num_cols; i++)
    {
	pinMode(_col_pins[i], OUTPUT);
	digitalWrite(_col_pins[i], LOW);
    }

    // We want 40Hz refresh at lowest intensity  
    // x 8 rows x 7 levels of intensity -> 2240Hz or 446us
    // TODO: dynamically calculate the ISR frequency based
    // on the matrix size (and number of colors).
    Timer1.initialize(400);
    Timer1.attachInterrupt(DirectMatrix_RefreshPWMLine);
}

void DirectMatrix::writeDisplay(void) {
    // DirectMatrix uses a timer to keep the display updated
}

void DirectMatrix::clear(void) {
  for (uint8_t i=0; i<_num_rows * _num_cols; i++) {
    DirectMatrix_MATRIX[i] = 0;
  }
}

PWMDirectMatrix::PWMDirectMatrix(uint8_t rows, uint8_t cols) : 
    DirectMatrix(rows, cols), Adafruit_GFX(rows, cols) {
}

void PWMDirectMatrix::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // TODO: we should support more than 8x8, so change this
  if ((y < 0) || (y >= 8)) return;
  if ((x < 0) || (x >= 8)) return;

  switch (getRotation()) {
  case 1:
    swap(x, y);
    x = 8 - x - 1;
    break;
  case 2:
    x = 8 - x - 1;
    y = 8 - y - 1;
    break;
  case 3:
    swap(x, y);
    y = 8 - y - 1;
    break;
  }

  // FIXME? this is reversed, I'd expect y * _num_cols + x, but if I do this
  // Character drawings from the underlying lib are backwards.
  DirectMatrix_MATRIX[y * _num_cols + x] = color;
}

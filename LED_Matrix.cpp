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
volatile uint16_t *DirectMatrix_MATRIX;
// These go to ground:
volatile GPIO_pin_t *DirectMatrix_ROW_PINS;
// Those go to V+
volatile GPIO_pin_t *DirectMatrix_COL_PINS;
// Shift Register Pins that also go to V+
volatile GPIO_pin_t *DirectMatrix_SR_PINS;
// How many colors in the array
volatile uint8_t DirectMatrix_NUM_COLORS;
// 4 frequencies for the ISR to make PWM colors
volatile uint32_t DirectMatrix_ISR_FREQ[4];

// profiling
volatile uint32_t DirectMatrix_ISR_runtime;
volatile uint32_t DirectMatrix_ISR_latency;


// ISR to refresh one matrix line
// This must be fast since it blocks interrupts and can only use globals.
// runtime: 
// - 268ns with 8 direct and 8 via SR (92 + 176) (arduino digitalwrite)
// - 136ns with 8 direct and 8 via SR (56 +  80) (digitalwrite2)
// - 108ns with 8 direct and 8 via SR (48 +  60) (digitalwrite2f)
// TODO:
// redo PWM by 
// 1) for 4 bits of intensity, be called at 400ns 800ns 1600ns and 3200ns (4 times) and turn on or off
// 2) to save some time with changing rows do each line first before switching to the next line?
// (allows leaving the line on while turning row pixels on or off)
void DirectMatrix_RefreshPWMLine(void) {
    static uint32_t time = micros();
    static uint8_t row = 0;
    static uint8_t pwm = 1;
    // we use 4 ISR frequencies for 16 bits of PWM and keep track of which
    // next interval (powers of 2) we set for next time this ISR should run
    static uint8_t isr_freq_offset = 0;
    int8_t oldrow;
    int8_t col_pin_offset = 0;
    uint16_t pwm_shifted = pwm;

    // Record latency between 2 calls
    DirectMatrix_ISR_latency = micros() - time;
    time = micros();

    // We play a speed trick and only turn off the row before setting the 
    // columns if we change rows. We however get called several times for
    // different PWM values and adjust the row intensity for PWM without
    // turning the column off and on.
    if (pwm == 1)
    {
	if (row == 0) oldrow = DirectMatrix_ARRAY_ROWS - 1; else oldrow = row-1;
	// Before setting the columns, shut off the previous row
	digitalWrite(DirectMatrix_ROW_PINS[oldrow], HIGH);
    }

    for (int8_t color = 0; color < DirectMatrix_NUM_COLORS; color++)
    {
	// If no SR is defined for this color, direct color mapping
	if (DirectMatrix_SR_PINS[color] == DINV)
	{
	    for (int8_t col = 0; col <= DirectMatrix_ARRAY_COLS - 1; col++)
	    {
		digitalWrite(DirectMatrix_COL_PINS[col + col_pin_offset],
		    (DirectMatrix_MATRIX[row * DirectMatrix_ARRAY_COLS + col] &
		     pwm_shifted)?HIGH:LOW);
	    }
	}
	else
	{
	    digitalWrite(DirectMatrix_SR_PINS[color], LOW);
	    for (int8_t col = 0; col <= DirectMatrix_ARRAY_COLS - 1; col++)
	    {
		digitalWrite(DirectMatrix_SR_PINS[CLK], LOW);
		digitalWrite(DirectMatrix_SR_PINS[DATA], 
		    (DirectMatrix_MATRIX[row * DirectMatrix_ARRAY_COLS + col] &
		     pwm_shifted)?HIGH:LOW);
		digitalWrite(DirectMatrix_SR_PINS[CLK], HIGH);
	    }
	    digitalWrite(DirectMatrix_SR_PINS[color], HIGH);
	}
	pwm_shifted <<= 4;
	col_pin_offset += DirectMatrix_ARRAY_COLS;
    }

    // Now that the colums are set, turn the row on
    if (pwm == 1) digitalWrite(DirectMatrix_ROW_PINS[row], LOW);

    pwm <<= 1;
    isr_freq_offset++;
    if (pwm >= DirectMatrix_PWM_LEVELS) 
    {
	pwm = 1;
	isr_freq_offset = 0;
	row++;
	if (row >= DirectMatrix_ARRAY_ROWS) row = 0;
    }
    // for 4 bits of PWM, only have 4 interrupts for 16 shades by having
    // each following interrupt be twice as long.
    Timer1.setPeriod(DirectMatrix_ISR_FREQ[isr_freq_offset]);

    // Record how long the function took
    DirectMatrix_ISR_runtime = micros() - time;
    time = micros();
}

DirectMatrix::DirectMatrix(uint8_t num_rows, uint8_t num_cols, 
	uint8_t num_colors) {
    _num_rows = num_rows;
    _num_cols = num_cols;

    // These need to be global so that the ISR can get to them.
    DirectMatrix_ARRAY_ROWS = num_rows;
    DirectMatrix_ARRAY_COLS = num_cols;
    DirectMatrix_NUM_COLORS = num_colors;

    if (! (_matrix = (uint16_t *) malloc(num_rows * num_cols * 2)))
    {
	while (1) {
	    Serial.println(F("Malloc failed in DirectMatrix::DirectMatrix"));
	}
    }
    DirectMatrix_MATRIX = _matrix;
}

// Array of of pins for vertical lines, and columns.
void DirectMatrix::begin(GPIO_pin_t __row_pins[], GPIO_pin_t __col_pins[], 
	GPIO_pin_t __sr_pins[], uint32_t __ISR_freq) {
    _row_pins = __row_pins;
    _col_pins = __col_pins;
    _sr_pins = __sr_pins;

    // These need to be global so that the ISR can get to them
    DirectMatrix_ROW_PINS = _row_pins;
    DirectMatrix_COL_PINS = _col_pins;
    DirectMatrix_SR_PINS = _sr_pins;
    DirectMatrix_ISR_FREQ[0] = __ISR_freq;
    DirectMatrix_ISR_FREQ[1] = __ISR_freq << 1;
    DirectMatrix_ISR_FREQ[2] = __ISR_freq << 2;
    DirectMatrix_ISR_FREQ[3] = __ISR_freq << 3;

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
    
    // Setup SR pins if any.
    for (uint8_t pin = 0; pin < 3; pin++)
    {
	if (_sr_pins[pin] == 255) continue;
	pinMode(_sr_pins[pin], OUTPUT);
	pinMode(_sr_pins[DATA], OUTPUT);
	pinMode(_sr_pins[CLK], OUTPUT);
	digitalWrite(_sr_pins[pin], LOW);
	for (uint8_t i = 0; i <= _num_rows; i++)
	{
	    digitalWrite(_sr_pins[CLK], LOW);
	    digitalWrite(_sr_pins[DATA], i & 1);
	    digitalWrite(_sr_pins[CLK], HIGH);
	}
	digitalWrite(_sr_pins[pin], HIGH);
    }

    // We want 40Hz refresh at lowest intensity  
    // x 8 rows x 7 levels of intensity -> 2240Hz or 446us
    // TODO: dynamically calculate the ISR frequency based
    // on the matrix size)
    // 400 isn't long enough to make full colors, 1000+ is better
    // but it makes PWM colors blink
    Timer1.initialize(DirectMatrix_ISR_FREQ[0]);
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

uint32_t DirectMatrix::ISR_runtime(void) {
  return DirectMatrix_ISR_runtime;
}
uint32_t DirectMatrix::ISR_latency(void) {
  return DirectMatrix_ISR_latency;
}

PWMDirectMatrix::PWMDirectMatrix(uint8_t rows, uint8_t cols, uint8_t colors) : 
    DirectMatrix(rows, cols, colors), Adafruit_GFX(rows, cols) {
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

  DirectMatrix_MATRIX[y * _num_cols + x] = color;
}

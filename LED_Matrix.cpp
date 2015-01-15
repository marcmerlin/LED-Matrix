/*************************************************** 
  This library was heavily inspired from Adafruit's LED-Backpack-Library
  written by Limor Fried/Ladyada for Adafruit Industries.  

  It was originally designed for products like these:
  ----> http://www.adafruit.com/products/902

  I used the library shell to write my own library to support LED Matrices
  directly addressed with an X/Y pin matrix like this one:
  https://www.sparkfun.com/products/682
  or even a tri color like this one:
  https://www.sparkfun.com/products/683

  Rows are addressed directly and columns either directly or by Shift Register.

  By Marc MERLIN <marc_soft@merlins.org>

  License: Apache 2.0 or MIT, at your choice.

  Required libraries:
  - TimerOne: https://www.pjrc.com/teensy/td_libs_TimerOne.html
  - Adafruit-GFX: https://github.com/marcmerlin/Adafruit-GFX-Library
  - http://www.codeproject.com/Articles/732646/Fast-digital-I-O-for-Arduino
    (this is not required, but makes things 3x faster)

 ****************************************************/

#ifdef __AVR_ATtiny85__
 #include <TinyWireM.h>
 #define Wire TinyWireM
#else
 #include <Wire.h>
#endif
#include "LED_Matrix.h"
#include "Adafruit_GFX.h"

// Globals required to pass matrix data into the ISR.
// (volatile is required for ISRs)
volatile uint8_t ROW_OFF;
volatile uint8_t ROW_ON;
volatile uint8_t COL_ON;
volatile uint8_t COL_OFF;
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


// ISR to refresh one matrix row
// This must be fast since it blocks interrupts and can only use globals.
// runtime. On Nano V3, for 2 colors:
// - 268ns with 8 direct and 8 via SR (92 + 176) (arduino digitalwrite)
// - 136ns with 8 direct and 8 via SR (56 +  80) (digitalwrite2)
// - 104ns with 8 direct and 8 via SR (48 +  56) (digitalwrite2f)
//
// For 3 colors (nano v3):
// - 160ns with 1x direct, 2x SR with digitalwrite2f
//
// PWM is done with binary code modulation as per 
// http://www.batsocks.co.uk/readme/art_bcm_1.htm
// 
// I tried to do all 4 bits of PWM on each row before going to the next row
// in an attempt to limit the amount of time rows are turned off, but the ISR
// takes too long and when multipled by 4, it takes too long before a full
// display refresh.
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

    if (row == 0) 
    {
	// When scanning a new row, set the new timer frequency for this run.
	Timer1.setPeriod(DirectMatrix_ISR_FREQ[isr_freq_offset]);
	oldrow = DirectMatrix_ARRAY_ROWS - 1;
    }
    else 
    {
	oldrow = row - 1;
    }
    // Before setting the columns, shut off the previous row
    digitalWrite(DirectMatrix_ROW_PINS[oldrow], ROW_OFF);

    for (int8_t color = 0; color < DirectMatrix_NUM_COLORS; color++)
    {
	// If no SR is defined for this color, direct color mapping
	if (DirectMatrix_SR_PINS[color] == DINV)
	{
	    for (int8_t col = 0; col <= DirectMatrix_ARRAY_COLS - 1; col++)
	    {
		digitalWrite(DirectMatrix_COL_PINS[col + col_pin_offset],
		    (DirectMatrix_MATRIX[row * DirectMatrix_ARRAY_COLS + col] &
		     pwm_shifted)?COL_ON:COL_OFF);
	    }
	}
	else if (DirectMatrix_SR_PINS[color] > 32768)
	{
	    digitalWrite((GPIO_pin_t) -DirectMatrix_SR_PINS[color], LOW);
	    for (int8_t col = DirectMatrix_ARRAY_COLS - 1; col >= 0; col--)
	    {
		digitalWrite(DirectMatrix_SR_PINS[CLK], LOW);
		digitalWrite(DirectMatrix_SR_PINS[DATA], 
		    (DirectMatrix_MATRIX[row * DirectMatrix_ARRAY_COLS + col] &
		     pwm_shifted)?COL_ON:COL_OFF);
		digitalWrite(DirectMatrix_SR_PINS[CLK], HIGH);
	    }
	    digitalWrite((GPIO_pin_t) -DirectMatrix_SR_PINS[color], HIGH);
	}
	else
	{
	    digitalWrite(DirectMatrix_SR_PINS[color], LOW);
	    for (int8_t col = 0; col <= DirectMatrix_ARRAY_COLS - 1; col++)
	    {
		digitalWrite(DirectMatrix_SR_PINS[CLK], LOW);
		digitalWrite(DirectMatrix_SR_PINS[DATA], 
		    (DirectMatrix_MATRIX[row * DirectMatrix_ARRAY_COLS + col] &
		     pwm_shifted)?COL_ON:COL_OFF);
		digitalWrite(DirectMatrix_SR_PINS[CLK], HIGH);
	    }
	    digitalWrite(DirectMatrix_SR_PINS[color], HIGH);
	}
	pwm_shifted <<= 4;
	col_pin_offset += DirectMatrix_ARRAY_COLS;
    }

    // Now that the colums are set, turn the row on
    digitalWrite(DirectMatrix_ROW_PINS[row], ROW_ON);

    row++;
    if (row >= DirectMatrix_ARRAY_ROWS)
    {
	row = 0;
	pwm <<= 1;
	isr_freq_offset++;
	if (pwm >= DirectMatrix_PWM_LEVELS) 
	{
	    pwm = 1;
	    isr_freq_offset = 0;
	}
    }

    // Record how long the function took
    DirectMatrix_ISR_runtime = micros() - time;
    time = micros();
}

DirectMatrix::DirectMatrix(uint8_t num_rows, uint8_t num_cols, 
	uint8_t num_colors, uint8_t common) {
    _num_rows = num_rows;
    _num_cols = num_cols;
    _num_colors = num_colors;

    // These need to be global so that the ISR can get to them.
    DirectMatrix_ARRAY_ROWS = num_rows;
    DirectMatrix_ARRAY_COLS = num_cols;
    DirectMatrix_NUM_COLORS = num_colors;

    if (not common)
    {
	ROW_OFF = HIGH;
	ROW_ON = LOW;
	COL_OFF = LOW;
	COL_ON = HIGH;
    }
    else
    {
	ROW_OFF = LOW;
	ROW_ON = HIGH;
	COL_OFF = HIGH;
	COL_ON = LOW;
    }

    if (! (_matrix = (uint16_t *) malloc(num_rows * num_cols * 2)))
    {
	while (1) {
	    Serial.println(F("Malloc failed in DirectMatrix::DirectMatrix"));
	}
    }
    DirectMatrix_MATRIX = _matrix;
}

// Array of of pins for vertical rows, and columns.
// __sr_pins can have negative values to fill rows backwards if you wired
// in that order.
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

    // Init the rows and cols with the opposite voltage to turn them off.
    for (uint8_t i = 0; i < _num_rows; i++)
    {
	pinMode(_row_pins[i], OUTPUT);
	digitalWrite(_row_pins[i], ROW_OFF);
    }
    
    // Setup output pins.
    for (uint8_t color = 0; color < _num_colors; color++)
    {
	if (_sr_pins[color] == DINV) 
	{
	    for (uint8_t i = 0; i < _num_cols; i++)
	    {
		pinMode(_col_pins[color * _num_rows + i], OUTPUT);
		digitalWrite(_col_pins[color * _num_rows + i], COL_OFF);
	    }
	}
	else if (_sr_pins[color] > 32768)
	{
	    pinMode((GPIO_pin_t) -_sr_pins[color], OUTPUT);
	    pinMode(_sr_pins[DATA], OUTPUT);
	    pinMode(_sr_pins[CLK], OUTPUT);
	    digitalWrite((GPIO_pin_t) -_sr_pins[color], LOW);
	    for (uint8_t i = 0; i <= _num_rows; i++)
	    {
		digitalWrite(_sr_pins[CLK], LOW);
		digitalWrite(_sr_pins[DATA], COL_OFF);
		digitalWrite(_sr_pins[CLK], HIGH);
	    }
	    digitalWrite((GPIO_pin_t) -_sr_pins[color], HIGH);
	}
	else
	{
	    pinMode(_sr_pins[color], OUTPUT);
	    pinMode(_sr_pins[DATA], OUTPUT);
	    pinMode(_sr_pins[CLK], OUTPUT);
	    digitalWrite(_sr_pins[color], LOW);
	    for (uint8_t i = 0; i <= _num_rows; i++)
	    {
		digitalWrite(_sr_pins[CLK], LOW);
		digitalWrite(_sr_pins[DATA], COL_OFF);
		digitalWrite(_sr_pins[CLK], HIGH);
	    }
	    digitalWrite(_sr_pins[color], HIGH);
	}
    }

    // We want at least 40Hz refresh at lowest intensity  
    // x 8 rows x 16 levels of intensity -> 5120Hz or 195us
    // I get good results by making the quickest interrupt be
    // 150us, and 300, 600, 1200us for the other ones.
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

// If common pins are cathode, set common to 0, otherwise 1.
PWMDirectMatrix::PWMDirectMatrix(uint8_t rows, uint8_t cols, uint8_t colors, 
		uint8_t common) : 
    DirectMatrix(rows, cols, colors, common), Adafruit_GFX(rows, cols) {
}

// Default is common cathode.
PWMDirectMatrix::PWMDirectMatrix(uint8_t rows, uint8_t cols, uint8_t colors) : 
    DirectMatrix(rows, cols, colors, 0), Adafruit_GFX(rows, cols) {
}

void PWMDirectMatrix::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((y < 0) || (y >= _num_rows)) return;
  if ((x < 0) || (x >= _num_cols)) return;

  switch (getRotation()) {
  case 1:
    swap(x, y);
    x = _num_cols - x - 1;
    break;
  case 2:
    x = _num_cols - x - 1;
    y = _num_rows - y - 1;
    break;
  case 3:
    swap(x, y);
    y = _num_rows - y - 1;
    break;
  }

  DirectMatrix_MATRIX[y * _num_cols + x] = color;
}

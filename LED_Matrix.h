#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#ifdef __AVR_ATtiny85__
 #include <TinyWireM.h>
#else
 #include <Wire.h>
#endif
#include <Adafruit_GFX.h>

// Include before the pinmode/digitalwrite below due to incompatible definitions
#include "TimerOne.h"

#define FASTIO

#ifdef FASTIO
//include the fast I/O 2 functions 
// http://www.codeproject.com/Articles/732646/Fast-digital-I-O-for-Arduino
#include "arduino2.h"
#define pinMode pinMode2f
#define digitalWrite digitalWrite2f
#define DINV DP_INVALID
#else
#define GPIO_pin_t uint8_t 
#define DINV 255 
#endif

#define DirectMatrix_PWM_LEVELS 7  // 3 bits -> 400 ns refresh with 324ns free
#define LED_RED_VERYLOW 	1
#define LED_RED_LOW 		2
#define LED_RED_MEDIUM 		5
#define LED_RED_HIGH 		7

#define LED_GREEN_VERYLOW	(1 << 4)
#define LED_GREEN_LOW 		(2 << 4)
#define LED_GREEN_MEDIUM 	(5 << 4)
#define LED_GREEN_HIGH 		(7 << 4)

#define LED_ORANGE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW		(LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH)

// Which slots are used for which pins in the SR array
#define LATCH1 0
#define LATCH2 1
#define LATCH3 2
#define DATA 3
#define CLK 4

class DirectMatrix {
 public:
  DirectMatrix(uint8_t, uint8_t, uint8_t);
  void begin(GPIO_pin_t [], GPIO_pin_t [], GPIO_pin_t []);
  void writeDisplay(void);
  void clear(void);
  uint32_t ISR_runtime(void);
  uint32_t ISR_latency(void);
  void init(uint8_t a);

 protected:
  uint8_t _num_rows;
  uint8_t _num_cols;
 
 private:
  GPIO_pin_t *_row_pins;
  GPIO_pin_t *_col_pins;
  GPIO_pin_t *_sr_pins;
  uint16_t *_matrix;
};

class PWMDirectMatrix : public DirectMatrix, public Adafruit_GFX {
 public:
  PWMDirectMatrix(uint8_t, uint8_t, uint8_t);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

 private:
};



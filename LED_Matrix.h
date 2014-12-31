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
#include "Adafruit_GFX.h"

#define DirectMatrix_PWM_LEVELS 7  // 3 bits -> 400 ns refresh with 324ns free
#define LED_VERYLOW 1
#define LED_LOW 2
#define LED_MEDIUM 5
#define LED_HIGH 7

class DirectMatrix {
 public:
  DirectMatrix(uint8_t, uint8_t);
  void begin(uint8_t [], uint8_t []);
  void writeDisplay(void);
  void clear(void);
  void init(uint8_t a);

 protected:
  uint8_t _num_rows;
  uint8_t _num_cols;
 
 private:
  uint8_t *_row_pins;
  uint8_t *_col_pins;
  uint8_t *_matrix;
};

class PWMDirectMatrix : public DirectMatrix, public Adafruit_GFX {
 public:
  PWMDirectMatrix(uint8_t, uint8_t);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

 private:
};



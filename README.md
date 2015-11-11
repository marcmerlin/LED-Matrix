LED-Matrix
==========

Single/bi/tri-color LED Matrix PWM driver for arduino (direct IO or Shift Register connected)

Blog post with video: http://marc.merlins.org/perso/arduino/post_2015-01-06_Driver-for-direct-driving-single-to-3-color-LED-Matrices-with-software-PWM.html

![](/screenshots/snap3.jpg?raw=true)

![](/screenshots/snap3_2.jpg?raw=true)

![](/screenshots/snap3_1.jpg?raw=true)

This code implements equivalent and additional functionality from what you get from Adafruit's 
https://github.com/adafruit/Adafruit-LED-Backpack-Library

Adafruit sells bi color 8x8 LED matrices with an I2C interface. They are easy to program with 
the adafruit library, but doing per pixel PWM is virtually impossible, which means you only
get 3 colors: Red, Green, and Orange (red+green).
See: http://www.adafruit.com/product/902

I wrote this driver to control raw LED matrices which is a lot more work, but provides more 
flexibility. My code allows you to connect the matrices directly to any arduino IO pins or
via shift registers for the columns (adding shift register support for rows is easy, but I 
haven't done it yet since it makes the code slower and I didn't need to save extra IO pins)

Here is how my approach differs from Adafruit's:
Summary: if you don't need to mix color intensities (3 colors is enough vs 256), buy adafruit's
solutions, they are much simpler for you and leave CPU time and IO pins on the arduino.
But if you already have raw LED matrices with no driver chip, this code is for you.
Note that if you buy a MAX7219 driver chip, you'll have the same problem than the adafruit solution
and will not be able to mix colors intensities.
I've found that this newer chip 
http://www.maximintegrated.com/en/products/power/display-power-control/MAX6961.html
can do bi color and 4 step PWM (not as good as my code, but not bad). You may still want to
look at a MAX6960.

Pluses for my solution:
- supports single/bi/tri-color LED matrices
- supports 16 or more levels of intensity per LED dot per color, allowing for
  16 shades, 256 colors, or 4096 colors on mono/bi/tri color arrays
- if you don't value your time, it's cheaper :)
- works with any raw LED matrix, including
  - https://www.sparkfun.com/products/682 (bi-color)
  - https://www.sparkfun.com/products/683 (tri-color) (note that the tricolor
    matrix can be had for just $5 from hobbyking: 
    http://www.hobbyking.com/hobbyking/preserve/__37852__Arduino_Tri_Color_RGB_LED_Dot_Matrix.html )

Minuses for my solution:
- my driver does all the work in software, and requires sequencing interrupts every 
  150, 300, 600, 1200 microseconds on the arduino (I use binary code modulation to
  avoid having 16 interrupts at 150us, and only have 4 interrupts at increasing
  frequencies. See http://www.batsocks.co.uk/readme/art_bcm_1.htm)
- obviously my solution requires more IO pins than adafruits (at least 5 pins for 2 colors
  and 6 pins for 3 colors, or many more if you don't use shift registers for extra speed)
- my driver also uses Timer1 (which means you can't easily use PWM 9 and 10)

Required libraries:
- TimerOne: https://www.pjrc.com/teensy/td_libs_TimerOne.html
- My fork of Adafruit-GFX: https://github.com/marcmerlin/Adafruit-GFX-Library because of
  this small patch https://github.com/adafruit/Adafruit-GFX-Library/pull/39 
- http://www.codeproject.com/Articles/732646/Fast-digital-I-O-for-Arduino
  (this is not required, but makes things 3x faster)

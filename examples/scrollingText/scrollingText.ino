/**********************************************************
 * @file    simpleAnimation.ino
 * @author  M.Brugman (mattb@linux.com)
 * @license MIT (see license.txt)
 **********************************************************
 * Example sketch showing how to use the Minimal Uno HUB75
 * library
**********************************************************/
#include <Uno_HUB75_Driver.h>

// instantiate the panel object
Uno_HUB75_Driver panel;

// coordinate mapper prototype
void origin(int16_t& x, int16_t& y);

// one time setup - do library init stuff
void setup()
{
  // call the begin method.  Parameters set to 'False' to say 
  // we're not using interrupts for timing the 'update()' method,
  // and a reference to the X/Y remapping method.  Origin point
  // (0, 0) point is the lower left corner
  panel.begin(false, origin);
}

void loop() 
{
  static uint32_t offset = millis();  // keep track of time
  static const char* msg = "Hello World!";
  static int16_t left = MAXCOLS;

  // Do ths every 50 milliseconds or so
  if (millis() - offset >= 50)
  {
    // update offset
    offset = millis();

    // start with a black panel
    panel.fillAll(Uno_HUB75_Driver::BLACK);

    // draw the text string
    panel.drawString(left, 11, msg, Uno_HUB75_Driver::YELLOW);

    // scroll one pixel left
    --left;

    // if the message has scrolled all the way off the left edge,
    // start it again at the right edge
    if (left < -(int)((strlen(msg) * 6)))
    {
      left = MAXCOLS;
    }

    // border around outer edge of panel
    panel.outlineBox(0, 0, MAXCOLS, MAXROWS, Uno_HUB75_Driver::BLUE);

    // Important!! Call this last after drawing everything else to update
    // the "active" framebuffer.  Miss this and the display will always
    // be blank
    panel.draw();
  }

  // not using interrupts/ISR, so we have to update manually
  panel.update();
}

/******************************************
* origin()
*******************************************
* This method defines the coordinate trans-
* lation.  The "default" origin is the upper
* left corner of the display.  This method
* translates so that origin is the lower
* left.
*
* It will be passed the Uno_HUB75_Driver class through
* the init() method - see below
******************************************/
void origin(int16_t& x, int16_t& y)
{
  y = MAXROWS - y;
}

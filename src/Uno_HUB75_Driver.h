/**********************************************************
 * @file    panel.h
 * @author  M.Brugman (mattb@linux.com)
 * @license MIT (see license.txt)
 **********************************************************
 * This is an Arduino library for driving a 32 X 16 HUB75
 * panel using an Arduino Uno type board.
 * 
 * The MCU is an 8-bit AVR running at 16MHz; it only has
 * 2K of RAM of 32K of Flash.  The challenge, obviously,
 * is to drive a the panel using the limited resources.
 * 
 * This library displays 3-bit colors (8 colors) using a
 * double buffered framebuffer to prevent flickering.  
 * Ideally, it will be using hardware timer 2 for updates,
 * but can optionally be updated in the caller's main
 * loop.
**********************************************************/
#ifndef PANEL_H_
#define PANEL_H_

// make sure of architecture, create a compiler error
// if wrong type of board is selected
#ifndef ARDUINO_ARCH_AVR
#error This library is specific to the AVR architecture
#endif

#include "Arduino.h"

// This is for the a 32x16 HUB75 display
#define COLS 32
#define ROWS 16

#define MAXCOLS (COLS - 1)
#define MAXROWS (ROWS - 1)
#define HALFROW (ROWS / 2)

// main class for this library
class Uno_HUB75_Driver
{
public:
  // all panel colors are defined by this enum
  enum Colors
  {
    BLACK = 0,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
  };

  // helper struct to define a single pixel on the panel
  typedef struct
  {
    int16_t x;
    int16_t y;
  } Point;

  // helper struct to define a rectangular region on the panel
  typedef struct
  {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
  } Rect;

  Uno_HUB75_Driver() {}
  ~Uno_HUB75_Driver() {}

  void begin(bool useISR = true, void(*xlater)(int16_t& x, int16_t& y) = NULL);
  
  void draw();
  void update();

  void clear();
  void fillAll(Uno_HUB75_Driver::Colors c);

  // polymorphic - pixel is defined by X/Y coordinates or a Point struct
  void setPixel(int16_t x, int16_t y, Uno_HUB75_Driver::Colors c);
  void setPixel(Uno_HUB75_Driver::Point& p, Uno_HUB75_Driver::Colors c)
        { setPixel(p.x, p.y, c); }

  // polymorphic - line is defined by a pair of X/Y coordinates or Point structs
  void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Uno_HUB75_Driver::Colors c);
  void line(Uno_HUB75_Driver::Point& start, Uno_HUB75_Driver::Point& end, Uno_HUB75_Driver::Colors c)  
        { line(start.x, start.y, end.x, end.y, c); }

  // polymorphic - rectangle is defined by a pair of X/Y coordinates or 2 Point structs or
  // a Rect struct
  void rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Uno_HUB75_Driver::Colors c, bool fill = true);
  void rectangle(Uno_HUB75_Driver::Point& topLeft, Uno_HUB75_Driver::Point& btmRight, Uno_HUB75_Driver::Colors c, bool fill = true)
        { rectangle (topLeft.x, topLeft.y, btmRight.x, btmRight.y, c, fill); }
  void rectangle(Uno_HUB75_Driver::Rect& rect, Uno_HUB75_Driver::Colors c, bool fill = true)
        { rectangle(rect.x1, rect.y1, rect.x2, rect.y2, c, fill); }

  // according to Arduino library specifications, it is better to provide the user with multiple
  // methods instead of using a bool to select functionality.
  void filledBox(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Uno_HUB75_Driver::Colors c)
        { rectangle(x1, y1, x2, y2, c, true); }
  void filledBox(Uno_HUB75_Driver::Point& topLeft, Uno_HUB75_Driver::Point& btmRight, Uno_HUB75_Driver::Colors c)
        { rectangle (topLeft.x, topLeft.y, btmRight.x, btmRight.y, c, true); }
  void filledBox(Uno_HUB75_Driver::Rect& rect, Uno_HUB75_Driver::Colors c, bool fill = true)
        { rectangle(rect.x1, rect.y1, rect.x2, rect.y2, c, true); }

  void outlineBox(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Uno_HUB75_Driver::Colors c)
        { rectangle(x1, y1, x2, y2, c, false); }
  void outlineBox(Uno_HUB75_Driver::Point& topLeft, Uno_HUB75_Driver::Point& btmRight, Uno_HUB75_Driver::Colors c)
        { rectangle (topLeft.x, topLeft.y, btmRight.x, btmRight.y, c, false); }
  void outlineBox(Uno_HUB75_Driver::Rect& rect, Uno_HUB75_Driver::Colors c)
        { rectangle(rect.x1, rect.y1, rect.x2, rect.y2, c, false); }

  // polymorphic - copy a pixel from one location to another
  void copyPixel(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  void copyPixel(Uno_HUB75_Driver::Point& src, Uno_HUB75_Driver::Point& dst)
        { copyPixel(src.x, src.y, dst.x, dst.y); }  
  
  // copy a rectangular region.  This method is only offered using the Rect struct, otherwise
  // the parameter list is unwieldy
  void copyRegion(Uno_HUB75_Driver::Rect& src, Uno_HUB75_Driver::Rect& dst);

  // text methods
  void drawChar(int16_t x, int16_t y, char chr, Uno_HUB75_Driver::Colors c);
  void drawString(int16_t x, int16_t y, const char* str, Uno_HUB75_Driver::Colors c);

  // return the color of the selected pixel
  Uno_HUB75_Driver::Colors getPixel(int16_t x, int16_t y);

private:
  // double-buffered to prevent flickering
  uint8_t pixBuff[HALFROW][COLS];         // "drawing" framebuffer
  uint8_t updBuff[HALFROW][COLS];         // "output" framebuffer
  bool usingISR;

  // pointer to the translator method provided by begin().  If none
  // provided, begin() will set this to NULL
  void (*xlatFunc)(int16_t& x, int16_t& y);

  // internal method to set the display buffer.  Does all sanity checking, 
  // so all drawing should come down to this instead of directly writing to
  // any buffer
  void setBuff(int16_t x, int16_t y, Uno_HUB75_Driver::Colors c);
};
#endif // PANEL_H_
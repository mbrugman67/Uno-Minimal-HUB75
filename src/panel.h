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

#include "Arduino.h"

// This is for the a 32x16 HUB75 display
#define COLS 32
#define ROWS 16

#define MAXCOLS (COLS - 1)
#define MAXROWS (ROWS - 1)
#define HALFROW (ROWS / 2)

class Panel
{
public:
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

  typedef struct
  {
    int16_t x;
    int16_t y;
  } Point;

  typedef struct
  {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
  } Rect;

  Panel() {}
  ~Panel() {}

  void begin(bool useISR = true, void(*xlater)(int16_t& x, int16_t& y) = NULL);
  
  void draw();
  void update();

  void clear();
  void fillAll(Panel::Colors c);

  // polymorphic - pixel is defined by X/Y coordinates or a Point struct
  void setPixel(int16_t x, int16_t y, Panel::Colors c);
  void setPixel(Panel::Point& p, Panel::Colors c)
        { setPixel(p.x, p.y, c); }

  // polymorphic - line is defined by a pair of X/Y coordinates or Point structs
  void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Panel::Colors c);
  void line(Panel::Point& start, Panel::Point& end, Panel::Colors c)  
        { line(start.x, start.y, end.x, end.y, c); }

  // polymorphic - rectangle is defined by a pair of X/Y coordinates or 2 Point structs or
  // a Rect struct
  void rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Panel::Colors c, bool fill = true);
  void rectangle(Panel::Point& topLeft, Panel::Point& btmRight, Panel::Colors c, bool fill = true)
        { rectangle (topLeft.x, topLeft.y, btmRight.x, btmRight.y, c, fill); }
  void rectangle(Panel::Rect& rect, Panel::Colors c, bool fill = true)
        { rectangle(rect.x1, rect.y1, rect.x2, rect.y2, c, fill); }

  // according to Arduino library specifications, it is better to provide the user with multiple
  // methods instead of using a bool to select functionality.
  void filledBox(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Panel::Colors c)
        { rectangle(x1, y1, x2, y2, c, true); }
  void filledBox(Panel::Point& topLeft, Panel::Point& btmRight, Panel::Colors c)
        { rectangle (topLeft.x, topLeft.y, btmRight.x, btmRight.y, c, true); }
  void filledBox(Panel::Rect& rect, Panel::Colors c, bool fill = true)
        { rectangle(rect.x1, rect.y1, rect.x2, rect.y2, c, true); }

  void outlineBox(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Panel::Colors c)
        { rectangle(x1, y1, x2, y2, c, false); }
  void outlineBox(Panel::Point& topLeft, Panel::Point& btmRight, Panel::Colors c)
        { rectangle (topLeft.x, topLeft.y, btmRight.x, btmRight.y, c, false); }
  void outlineBox(Panel::Rect& rect, Panel::Colors c)
        { rectangle(rect.x1, rect.y1, rect.x2, rect.y2, c, false); }

  void copyPixel(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  void copyPixel(Panel::Point& src, Panel::Point& dst)
        { copyPixel(src.x, src.y, dst.x, dst.y); }  
  void copyRegion(Panel::Rect& src, Panel::Rect& dst);

  void drawChar(int16_t x, int16_t y, char chr, Panel::Colors c);
  void drawString(int16_t x, int16_t y, const char* str, Panel::Colors c);

  Panel::Colors getPixel(int16_t x, int16_t y);

private:
  uint8_t pixBuff[HALFROW][COLS];
  uint8_t updBuff[HALFROW][COLS];
  bool usingISR;

  // pointer to the translator method provided by begin().  If none
  // provided, begin() will set this to NULL
  void (*xlatFunc)(int16_t& x, int16_t& y);

  // internal method to set the display buffer.  Does all sanity checking, 
  // so all drawing should come down to this instead of directly writing to
  // any buffer
  void setBuff(int16_t x, int16_t y, Panel::Colors c);
};
#endif // PANEL_H_
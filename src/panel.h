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
    uint8_t x;
    uint8_t y;
  } Point;

  typedef struct
  {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
  } Rect;

  Panel() {}
  ~Panel() {}

  void begin(bool useISR = true, void(*xlater)(uint8_t& x, uint8_t& y) = NULL);
  
  void draw();
  void update();

  void clear();
  void fillAll(Panel::Colors c);

  void setPoint(uint8_t x, uint8_t y, Panel::Colors color);
  void setPoint(Panel::Point& p, Panel::Colors c)
        { setPoint(p.x, p.y, c); }

  void line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Panel::Colors color);
  void line(Panel::Point& start, Panel::Point& end, Panel::Colors c)  
        { line(start.x, start.y, end.x, end.y, c); }

  void rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Panel::Colors color, bool fill = true);
  void rectangle(Panel::Point& topLeft, Panel::Point& btmRight, Panel::Colors c, bool fill = true)
        { rectangle (topLeft.x, topLeft.y, btmRight.x, btmRight.y, c, fill); }
  void rectagle(Panel::Rect& rect, Panel::Colors c, bool fill = true)
        { rectangle(rect.x1, rect.y1, rect.x2, rect.y2, c, fill); }

  void copyPoint(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
  void copyPoint(Panel::Point& src, Panel::Point& dst)
        { copyPoint(src.x, src.y, dst.x, dst.y); }  
  void copyRegion(Panel::Rect& src, Panel::Rect& dst);

  void drawA(uint8_t x, uint8_t y, Panel::Colors color);
  void drawB(uint8_t x, uint8_t y, Panel::Colors color);
  void drawC(uint8_t x, uint8_t y, Panel::Colors color);

  Panel::Colors getPoint(uint8_t x, uint8_t y);

private:
  uint8_t pixBuff[HALFROW][COLS];
  uint8_t updBuff[HALFROW][COLS];

  void (*xlatFunc)(uint8_t& x, uint8_t& y);

  int16_t m;  // the "m" in y = mx + b
  int16_t b;  // the "b" in y = mx + b

  void setBuff(uint8_t x, uint8_t y, Panel::Colors c);
  void resetLines();
  void restControlLines();
  void setupSlopeIntercept(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
  uint8_t calcSlopeIntercept(uint8_t x);
};
#endif // PANEL_H_
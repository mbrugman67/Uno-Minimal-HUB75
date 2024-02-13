#include "panel.h"
#include "panel_impl.h"
#include "font.h"

#define TOPMASK 0x07
#define BTMMASK 0x70

#define swap(a, b)  {uint8_t t = a; a = b; b = t;}

/********************************************************
* clear()
*********************************************************
* Make the whole display black
********************************************************/
void Panel::clear()
{
  this->fillAll(Panel::BLACK);
}

/********************************************************
* fillAll()
*********************************************************
* Make the whole display one solid color, good for a 
* background
*
* Parameters:
*   Panel::Colors color - a member of the colors enumeration
*     to fill the panel with
* Returns
*   Void
********************************************************/
void Panel::fillAll(Panel::Colors c)
{
  uint8_t val = (uint8_t)c | (uint8_t)c << 4;
  memset(pixBuff, val, HALFROW * COLS);
}

/********************************************************
* setPoint()
*********************************************************
* Set a single pixel on the panel
*
* Parameters:
*   uint8_t x - the x coordinate
*   uint8_t y - the y coordinate
*   Panel::Colors color - color to set point
* Returns
*   Void
********************************************************/
void Panel::setPoint(uint8_t x, uint8_t y, Panel::Colors color)
{
  if (x < COLS && y < ROWS)
  {
    this->setBuff(x, y, color);
  }
}

/********************************************************
* getPoint()
*********************************************************
* Get the color of a single pixel on the panel
*
* Parameters:
*   uint8_t x - the x coordinate
*   uint8_t y - the y coordinate
* Returns
*   Panel::Colors color - color of the specified pixel
********************************************************/
Panel::Colors Panel::getPoint(uint8_t x, uint8_t y)
{
  if (x < COLS && y < ROWS)
  {
    // handle coordinate translation
    if (xlatFunc)
    {
      this->xlatFunc(x, y);
    }

    return ((Panel::Colors)pixBuff[y][x]);
  }
  
  return (Panel::BLACK);
}

/********************************************************
* copyPoint()
*********************************************************
* Set a single pixel on the panel
*
* Parameters:
*   uint8_t x1 - the x coordinate of source pixel
*   uint8_t y1 - the y coordinate of source pixel
*   uint8_t x2 - the x coordinate of destination pixel
*   uint8_t y2 - the y coordinate of destination pixel
* Returns
*   Void
********************************************************/
void Panel::copyPoint(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
  if (x1 < COLS && x2 < COLS && y1 < ROWS && y2 < ROWS)
  {
    this->setPoint(x2, y2, this->getPoint(x1, y2));
  }
}

/********************************************************
* copyRegion()
*********************************************************
* Copy the colors of a rectangle to another rectangle
*
* Parameters:
*   Panel::Rect src - reference to source region
*   Panel::Rect dst - reference to destination region
* Returns
*   Void
********************************************************/
void Panel::copyRegion(Panel::Rect& src, Panel::Rect& dst)
{
  for(uint8_t hgt = 0; hgt <= (dst.y2 - dst.y1); ++hgt)
  {
    for(uint8_t len = 0; len <= (dst.x2 - dst.x1); ++len)
    {
      // all of the (uint8_t) casts are required because of default integer promotion in c
      this->copyPoint((uint8_t)(src.x1 + len), (uint8_t)(src.y1 + hgt), (uint8_t)(dst.x1 + len), (uint8_t)(dst.y1 + hgt));
    }
  }
}

/********************************************************
* rectangle()
*********************************************************
* Draw a rectangle somewhere on the display and 
* optionally fill it with a color
*
* Parameters:
*   uint8_t top - the top row (inclusive)
*   uint8_t left - the left hand side (inclusive)
*   uint8_t bottom - the bottom row (inclusive)
*   uint8_t right - the right hand side (inclusive)
*   Panel::Colors color - line and fill color
*   bool fill - true to fill with color
* Returns
*   Void
********************************************************/
void Panel::rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Panel::Colors color, bool fill)
{
  // sanity checks
  if (x1 > MAXCOLS)   x1 = MAXCOLS;
  if (x2 > MAXCOLS)   x2 = MAXCOLS;
  if (y1 > MAXROWS)   y1 = MAXROWS;
  if (y2 > MAXROWS)   y2 = MAXROWS;

  // fixup backwards dimensions
  if (x1 > x2)     swap(x1, x2)
  if (y1 > y1)     swap(y1, y2)

  if (fill == false)
  {
    this->line(x1, y1, x2, y1, color);
    this->line(x2, y1, x2, y2, color);
    this->line(x1, y2, x2, y2, color);
    this->line(x1, y1, x1, y2, color);
  }
  else
  {
    // fill in the buffer
    for (uint8_t col = x1; col <= x2; ++col)
    {
      for (uint8_t row = y1; row <= y2; ++row)
      {
        this->setBuff(col, row, color);
      }
    }
  }
}

/********************************************************
* line()
*********************************************************
* Draw a line somewhere on the display.  In general, uses
* slope-intercept formula
*
* Parameters:
*   uint8_t x1 - the starting x point
*   uint8_t y1 - the starting y point
*   uint8_t x2 - the ending x point
*   uint8_t y2 - the ending y poing
*   Panel::Colors color - line and fill color
*   bool fill - true to fill with color
* Returns
*   Void
********************************************************/
void Panel::line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Panel::Colors color)
{
  if (x1 > MAXCOLS)    x1 = MAXCOLS;
  if (x2 > MAXCOLS)    x2 = MAXCOLS;
  if (y1 > MAXROWS)    y1 = MAXROWS;
  if (y2 > MAXROWS)    y2 = MAXROWS;

  // special case of horizontal line
  if (y1 == y2)
  {
    if (x1 > x2)  swap(x1, x2);

    for (uint8_t col = x1; col <= x2; ++col)
    {
      this->setBuff(col, y1, color);
    }
  }
  // special case of vertical line
  else if (x1 == x2)
  {
    if (y1 > y2)  swap(y1, y2);

    for (uint8_t row = y1; row <= y2; ++row)
    {
      this->setBuff(x1, row, color);
    }
  }
  else
  // slope-intercept time
  {
    this->setupSlopeIntercept(x1, y1, x2, y2);

    if (x1 > x2)    swap(x1, x2);
    for (uint8_t col = x1; col <= x2; ++col)
    {
      this->setBuff(col, this->calcSlopeIntercept(col), color);
    }
  }  
}

void Panel::drawA(uint8_t x, uint8_t y, Panel::Colors color)
{
  for (uint8_t col = 0; col < 5; ++col)
  {
    for (uint8_t row = 0; row < 7; ++row)
    {
      if (((figA[col] >> row) & 0x01))
      {
        this->setPoint(col + x, y - row, color);
      }
    }
  }
}

void Panel::drawB(uint8_t x, uint8_t y, Panel::Colors color)
{
  for (uint8_t col = 0; col < 5; ++col)
  {
    for (uint8_t row = 0; row < 7; ++row)
    {
      if (((figB[col] >> row) & 0x01))
      {
        this->setPoint(col + x, y - row, color);
      }
    }
  }
}

void Panel::drawC(uint8_t x, uint8_t y, Panel::Colors color)
{
  for (uint8_t col = 0; col < 5; ++col)
  {
    for (uint8_t row = 0; row < 7; ++row)
    {
      if (((figC[col] >> row) & 0x01))
      {
        this->setPoint(col + x, y - row, color);
      }
    }
  }
}

/********************************************************
* setBuff()
*********************************************************
* Translate coordinate system.  By convention (and 
* imagination), the memory buffer is kind of seen with 
* (0, 0) being the top left corner.  
*
* Maybe you wanna have the origin be somewhere else? 
* Do that by passing a translation method to this classes
* init() method
*
* Paramters:
*   uint8_t x - the X coordinate
*   uint8_t y - the Y coordinate
*   Panel::Colors c - the color
* Returns
*   void
********************************************************/
void Panel::setBuff(uint8_t x, uint8_t y, Panel::Colors c)
{
  // if a translation method was specified in the init()
  // method, then do that translation
  if (xlatFunc)
  {
    this->xlatFunc(x, y);
  }

  // memory buffer is set up as [row][column], or 
  // think of it as [Y][X].  Maybe backwards, but
  // it makes looping through the drive more clean

  // We're using 8 colors on the panel (black, red, green, blue,
  // yellow, cyan, magenta).  That only takes 3 bits.  To save
  // RAM on this small micro, we'll use the lower 4 bits of each
  // buffer byte to be the "top" half of the physical display
  // and the upper 4 bytes for the "bottom" half of the physical
  // display
  if (y < HALFROW)
  {
    pixBuff[y][x] = (uint8_t)((pixBuff[y][x] & BTMMASK) | c) & 0xff;
  }
  else
  {
    pixBuff[y - HALFROW][x] = (uint8_t)((pixBuff[y - HALFROW][x] & TOPMASK) | (uint8_t)(c << 4)) & 0xff; 
  }
}

void Panel::draw()
{
  TIMSK2 &= ~bit(OCIE2A);
  memcpy(updBuff, pixBuff, HALFROW * COLS);
  TIMSK2 |= bit(OCIE2A);
}

/********************************************************
* update()
*********************************************************
* Do the thing to shift the buffer out to the HUB75
* display.
*
* This should be called around every 1 or 2 milliseconds
* for decent results.
*
* With the clock at 16 MHz, this routine was measured to 
* take a total of 568 us (microseconds).  If you call the
* routine every 1 ms, you will be using just over half the
* horsepower of a ATMega 328.
********************************************************/
void Panel::update()
{
  for (uint8_t thisRow = 0; thisRow < HALFROW; ++thisRow)
  {
    this->restControlLines();

    // get a pointer to the first column of this row
    uint8_t* row = updBuff[thisRow];
    
    // fill in all of the columns for this row and the 
    // corresponding "lower" row.
    for (uint8_t ii = 0; ii < COLS; ++ii)
    {
      CLRBIT_CTL(PIN_CLK);

      // get the lower 2 bits - we don't want to change them; they are
      // the RX/TX pins of the UART which may be used for something else
      PORTD &= 0x03;

      // The panel framebuffer is storing the top physical half in the lower
      // 4 bits of each byte, while the lower physical half is in the upper
      // 4 bits of each byte - hence the masking and shifting of bits
      PORTD |= (((*row & TOPMASK) << 2) | ((*row & BTMMASK) << 1)) & 0xfc;

      ++row;

      // clock this column in
      SETBIT_CTL(PIN_CLK);
    }

    // turn off output, set row, then re-enable output
    SETBIT_CTL(PIN_LAT);
    CLRBIT_CTL(PIN_LAT);

    SETBIT_CTL(PIN_OE);
    PORTB &= ~0x07;
    PORTB |= thisRow & 0x07;
    CLRBIT_CTL(PIN_OE);

    // Delay a bit for added PoV brightness of the display.  Could be
    // longer at the expense of processing bandwidth.  Without the 
    // _NOP(), the compiler will optimize this loop away
    for (uint8_t ii = 0; ii < 60; ++ii) _NOP();
  }
  
  SETBIT_CTL(PIN_OE);
}

/********************************************************
* begin()
*********************************************************
* Set up the display, and optionally the timer interrupt
*
* Parameters:
*   bool useISR - true to use interrupt timing
*   (void)(*xlate)(uint8_t& x, uint8_t& y) - translation function
* Returns
*   Void
********************************************************/
void Panel::begin(bool useISR, void(*xlater)(uint8_t& x, uint8_t& y))
{
  // set up I/O pins
  for (uint8_t ii = 2; ii < 14; ++ii)
  {
    pinMode(ii, OUTPUT);
    digitalWrite(ii, LOW);
  }

  // clear pixel buffer
  this->clear();

  // clear the update buffer
  memset(updBuff, 0x00, HALFROW * COLS);

  // tranlation function
  xlatFunc = xlater;

  // using ISR for timing??
  if (useISR)
  {
    TCCR2A = 0;
    bitSet(TCCR2A, WGM21);  // WGM mode CTC, auto reset
    
    // For a time period of 2ms (500Hz):
    // prescaler = 256, count = 125
    // error = 0.0%
    OCR2A = 125;
    TCCR2B = bit(CS22) | bit(CS21);
    
    // enable interrupt on A
    TIMSK2 = bit(OCIE2A);  
  }
}


/********************************************************
* setupSlopeIntercept()
*********************************************************
* Set up y = mx + b slope/intercept equation for m and b
*
* Call once before drawing a line that is not completely
* horizontal or vertical
*
* Parameters:
*   uint8_t x1 - the starting x point
*   uint8_t y1 - the starting y point
*   uint8_t x2 - the ending x point
*   uint8_t y2 - the ending y poing
* Returns
*   Void
********************************************************/
void Panel::setupSlopeIntercept(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
  m = ((((int16_t)y2) * 100) - (((int16_t)y1) * 100)) / ((int16_t)x2 - (int16_t)x1);
  b = (int16_t)y1 - (((int16_t)x1 * m) / 100);
}


/********************************************************
* calcSlopeIntercept()
*********************************************************
* Calculate the Y for a given X with known m & b
*
* Parameters:
*   uint8_t x - the x coordinate of a line
* Returns
*   uint8_t y - the corresponding Y coordinate of the line
********************************************************/
uint8_t Panel::calcSlopeIntercept(uint8_t x)
{
  int16_t y = (m * (int16_t)x) / 100 + b;

  // sanity check
  if (y < 0)        y = 1;
  if (y > MAXROWS)  y = MAXROWS;

  return ((uint8_t)y);
}

void Panel::restControlLines()
{
  // clear the control lines
  SETBIT_CTL(PIN_OE);   // Output enable is active low
  CLRBIT_CTL(PIN_CLK);
  CLRBIT_CTL(PIN_LAT);
  CLRBIT_CTL(PIN_RA);
  CLRBIT_CTL(PIN_RB);
  CLRBIT_CTL(PIN_RC);
}

void Panel::resetLines()
{
  // set all control lines (except OE to low)  CLRBIT_CTL(PIN_CLK);
  CLRBIT_CTL(PIN_LAT);
  SETBIT_CTL(PIN_OE);   // Output enable is active low
  CLRBIT_CTL(PIN_RA);
  CLRBIT_CTL(PIN_RB);
  CLRBIT_CTL(PIN_RC);

  // clear the pixel shift register values
  PORTD &= 0x03;
}

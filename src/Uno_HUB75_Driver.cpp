/**********************************************************
 * @file    panel.cpp
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
#include "Uno_HUB75_Driver.h"
#include "Uno_HUB75_Driver_impl.h"
#include "font.h"

#include "Arduino.h"

#define TOPMASK 0x07
#define BTMMASK 0x70

#define swap(a, b)  {int16_t t = a; a = b; b = t;}

/********************************************************
* clear()
*********************************************************
* Make the whole display black
********************************************************/
void Uno_HUB75_Driver::clear()
{
  this->fillAll(Uno_HUB75_Driver::BLACK);
}

/********************************************************
* fillAll()
*********************************************************
* Make the whole display one solid color, good for a 
* background
*
* Parameters:
*   Uno_HUB75_Driver::Colors color - a member of the colors enumeration
*     to fill the panel with
* Returns
*   Void
********************************************************/
void Uno_HUB75_Driver::fillAll(Uno_HUB75_Driver::Colors c)
{
  uint8_t val = (uint8_t)c << 2 | (uint8_t)c << 5;
  memset(pixBuff, val, HALFROW * COLS);
}

/********************************************************
* setPixel()
*********************************************************
* Set a single pixel on the panel
*
* Parameters:
*   int16_t x - the x coordinate
*   int16_t y - the y coordinate
*   Uno_HUB75_Driver::Colors color - color to set point
* Returns
*   Void
********************************************************/
void Uno_HUB75_Driver::setPixel(int16_t x, int16_t y, Uno_HUB75_Driver::Colors color)
{
  // only set buffer if in the actual drawable region
  if (x >= 0 && x < COLS && y >= 0 && y < ROWS)
  {
    this->setBuff(x, y, color);
  }
}

/********************************************************
* getPixel()
*********************************************************
* Get the color of a single pixel on the panel
*
* Parameters:
*   int16_t x - the x coordinate
*   int16_t y - the y coordinate
* Returns
*   Uno_HUB75_Driver::Colors color - color of the specified pixel
********************************************************/
Uno_HUB75_Driver::Colors Uno_HUB75_Driver::getPixel(int16_t x, int16_t y)
{
  if (x >= 0 && x < COLS && y >= 0 && y < ROWS)
  {
    // handle coordinate translation
    if (xlatFunc)
    {
      this->xlatFunc(x, y);
    }

    return ((Uno_HUB75_Driver::Colors)pixBuff[y][x]);
  }
  
  return (Uno_HUB75_Driver::BLACK);
}

/********************************************************
* copyPixel()
*********************************************************
* Set a single pixel on the panel
*
* Parameters:
*   int16_t x1 - the x coordinate of source pixel
*   int16_t y1 - the y coordinate of source pixel
*   int16_t x2 - the x coordinate of destination pixel
*   int16_t y2 - the y coordinate of destination pixel
* Returns
*   Void
********************************************************/
void Uno_HUB75_Driver::copyPixel(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
  this->setPixel(x2, y2, this->getPixel(x1, y2));
}

/********************************************************
* copyRegion()
*********************************************************
* Copy the colors of a rectangle to another rectangle
*
* Parameters:
*   Uno_HUB75_Driver::Rect src - reference to source region
*   Uno_HUB75_Driver::Rect dst - reference to destination region
* Returns
*   Void
********************************************************/
void Uno_HUB75_Driver::copyRegion(Uno_HUB75_Driver::Rect& src, Uno_HUB75_Driver::Rect& dst)
{
  for(int16_t hgt = 0; hgt <= (dst.y2 - dst.y1); ++hgt)
  {
    for(int16_t len = 0; len <= (dst.x2 - dst.x1); ++len)
    {
      this->copyPixel(src.x1 + len, src.y1 + hgt, dst.x1 + len, dst.y1 + hgt);
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
*   int16_t top - the top row (inclusive)
*   int16_t left - the left hand side (inclusive)
*   int16_t bottom - the bottom row (inclusive)
*   int16_t right - the right hand side (inclusive)
*   Uno_HUB75_Driver::Colors color - line and fill color
*   bool fill - true to fill with color
* Returns
*   Void
********************************************************/
void Uno_HUB75_Driver::rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Uno_HUB75_Driver::Colors color, bool fill)
{
  // fixup backwards dimensions
  if (x1 > x2)     swap(x1, x2)
  if (y1 > y2)     swap(y1, y2)

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
    for (int16_t col = x1; col <= x2; ++col)
    {
      for (int16_t row = y1; row <= y2; ++row)
      {
        this->setPixel(col, row, color);
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
*   int16_t x1 - the starting x point
*   int16_t y1 - the starting y point
*   int16_t x2 - the ending x point
*   int16_t y2 - the ending y poing
*   Uno_HUB75_Driver::Colors color - line and fill color
*   bool fill - true to fill with color
* Returns
*   Void
********************************************************/
void Uno_HUB75_Driver::line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Uno_HUB75_Driver::Colors color)
{
  // special case of horizontal line
  if (y1 == y2)
  {
    if (x1 > x2)  swap(x1, x2);

    for (int16_t col = x1; col <= x2; ++col)
    {
      this->setPixel(col, y1, color);
    }
  }
  // special case of vertical line
  else if (x1 == x2)
  {
    if (y1 > y2)  swap(y1, y2);

    for (int16_t row = y1; row <= y2; ++row)
    {
      this->setPixel(x1, row, color);
    }
  }
  else
  {
    // slope-intercept time.  Calculate the 'm' and 'b' for
    // the general y = mx + b line equation.  Scaling by
    // 100 instead of using floating point, because fp on 
    // an 8-bit MCU is ridiculously slow
    int16_t m = ((y2 * 100) - (y1 * 100)) / (x2 - x1);
    int16_t b = y1 - ((x1 * m) / 100);

    if (x1 > x2)    swap(x1, x2);

    for (int16_t col = x1; col <= x2; ++col)
    {
      this->setPixel(col, (m * col) / 100 + b, color);
    }
  }  
}


/********************************************************
* drawChar()
*********************************************************
* Draw and ascii character on the panel at the specified
* coordinates using specified color
*
* This method will draw 5x7 pixel bitmapped font 
* characters on the panel
*
* The font characters are stored in flash memory to save
* RAM; otherwise it would consume another 640 bytes - more
* than both framebuffers!
*
* Paramters:
*   int16_t x - the X coordinate
*   int16_t y - the Y coordinate
*   char chr - character to print
*   Uno_HUB75_Driver::Colors c - the color
* Returns
*   void
********************************************************/
void Uno_HUB75_Driver::drawChar(int16_t x, int16_t y, char chr, Uno_HUB75_Driver::Colors c)
{
  // array in RAM to hold character retrieved from FLASH
  uint8_t flashChr[5];

  // cast to unsigned
  uint8_t inx = (uint8_t)chr;

  // limit to the first 128 characters
  if (chr > 0x7f) chr = 0x7f;

  // the font is 5 columns by 7 rows
  for (int16_t ii = 0; ii < 5; ++ii)
  {
    // read this column's byte from FLASH into RAM
    flashChr[ii] = pgm_read_byte(&font5x7[inx][ii]);
  }

  // 5 columns by 7 rows
  for (int16_t col = 0; col < 5; ++col)
  {
    for (int16_t row = 0; row < 7; ++row)
    {
      // each row is a bit in the column byte
      if ((flashChr[col] >> row) & 0x01)
      {
        // if the bit is set, set the corresponding pixel
        this->setPixel(col + x, y - row, c);
      } // is pixel set 
    } // looping through 7 rows (Y) for this column
  } // looping through the 5 columns
}


/********************************************************
* drawString()
*********************************************************
* Draw ascii characters on the panel at the specified
* coordinates using specified color
*
* This method will draw 5x7 pixel bitmapped font 
* characters on the panel
*
* The font characters are stored in flash memory to save
* RAM; otherwise it would consume another 640 bytes - more
* than both framebuffers!
*
* Paramters:
*   int16_t x - the X coordinate
*   int16_t y - the Y coordinate
*   const char* str - string to print
*   Uno_HUB75_Driver::Colors c - the color
* Returns
*   void
********************************************************/
void Uno_HUB75_Driver::drawString(int16_t x, int16_t y, const char* str, Uno_HUB75_Driver::Colors c)
{
  for (uint8_t ii = 0; ii < strlen(str); ++ii)
  {
    this->drawChar(x, y, str[ii], c);
    
    // index 6 pixels right for the next char
    x += 6;
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
*   int16_t x - the X coordinate
*   int16_t y - the Y coordinate
*   Uno_HUB75_Driver::Colors c - the color
* Returns
*   void
********************************************************/
void Uno_HUB75_Driver::setBuff(int16_t x, int16_t y, Uno_HUB75_Driver::Colors c)
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
    // mask off the top 3 bits (5, 6, and 7) and or it with the color (shifted left 2 bits)
    pixBuff[y][x] = (uint8_t)(((pixBuff[y][x] & 0xe0) | (c << 2)) & 0xff);
  }
  else
  {
    // mask off bits 2, 3, and 4 (the color bits for the upper half), and or it with the
    // 3 color bits for the upper half
    pixBuff[y - HALFROW][x] = (uint8_t)(((pixBuff[y - HALFROW][x] & 0x1c) | (c << 5)) & 0xff); 
  }
}


/********************************************************
* draw()
*********************************************************
* This needs to be called after drawing of the panel is
* complete so that the buffer can be sent out to the panel
********************************************************/
void Uno_HUB75_Driver::draw()
{
  // if using ISR for update, disable it during
  // the memcpy operation to prevent flickering
  if (usingISR)
  {
    TIMSK2 &= ~bit(OCIE2A);
  }
  
  // copy drawing framebuffer to active framebuffer
  memcpy(updBuff, pixBuff, HALFROW * COLS);
  
  // re-enable ISR
  if (usingISR)
  {
    TIMSK2 |= bit(OCIE2A);
  }
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
* take a total of 440 us (microseconds).  If you call the
* routine every 2 ms, you will be using just under a 
* quarter of the horsepower of a ATMega 328.
********************************************************/
void Uno_HUB75_Driver::update()
{
  for (uint8_t thisRow = 0; thisRow < HALFROW; ++thisRow)
  {
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

      // The panel framebuffer is storing the top physical half in bits 2, 3, and 4
      // of each byte, while the lower physical half is in bits 5, 6, and 7
      // of each byte - hence the masking
      PORTD |= *row & 0xfc;

      // next pixel in this row
      ++row;

      // clock this column in
      SETBIT_CTL(PIN_CLK);
    }

    // turn off output
    SETBIT_CTL(PIN_OE);

    // set row
    PORTB &= ~0x07;
    PORTB |= thisRow & 0x07;

    // latch this row
    SETBIT_CTL(PIN_LAT);
    CLRBIT_CTL(PIN_LAT);

    // turn output back on
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
*   (void)(*xlate)(int16_t& x, int16_t& y) - translation function
* Returns
*   Void
********************************************************/
void Uno_HUB75_Driver::begin(bool useISR, void(*xlater)(int16_t& x, int16_t& y))
{
  // set up I/O pins
  for (uint8_t ii = 2; ii < 14; ++ii)
  {
    pinMode(ii, OUTPUT);
    digitalWrite(ii, LOW);
  }

  // clear the control lines
  SETBIT_CTL(PIN_OE);   // Output enable is active low
  CLRBIT_CTL(PIN_CLK);
  CLRBIT_CTL(PIN_LAT);
  CLRBIT_CTL(PIN_RA);
  CLRBIT_CTL(PIN_RB);
  CLRBIT_CTL(PIN_RC);

  // clear pixel buffer
  this->clear();

  // clear the update buffer
  memset(updBuff, 0x00, HALFROW * COLS);

  // tranlation function
  xlatFunc = xlater;

  // using ISR for timing??
  if (useISR)
  {
    usingISR = true;

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
  else
  {
    usingISR = false;
  }
}

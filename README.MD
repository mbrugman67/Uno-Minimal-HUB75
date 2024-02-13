# Minimal UNO HUB75 display driver
This library will control one 32x16 HUB75 display using an Arduino UNO style board.  The libarary will do 8 color (3 bit) display using a double-buffered framebuffer to prevent flickering.  Ideally, the updates will be performed in the Timer 2 ISR, but can optionally be done in the caller's main loop.

### Features
The library provides drawing primitives such as individual pixel write/read, arbitrary lines, and filled and unfilled rectangles in the colors black, red, yellow, green, cyan, magenta, blue, and white.  There are methods to fill the entire display, copy single pixels and even rectangular regions across the display.  There is a bitmapped 5x7 font available for writing text on the display.

Simple animations and scrolling can be accomplished by simply redrawing the buffer at a decent rate.

# Using the library
Instantiate a member of the Panel class.  The `begin()` method takes two arguments (with default values); the first defines if the update method will be called from the Timer 2 ISR, and the second defines a coordinate translation method.

Once instantiated, "draw" on the panel by using the included primitives or even pixel-by-pixel.  When the desired items are drawn, call the `draw()` method to copy the working frambuffer to the output framebuffer; this double-buffering prevents flickering.

The `update()` method needs to be called periodically; either as part of the Timer 2 ISR if desired, or as part of your application's main loop.  For example, if using the ISR:  

```
ISR(TIMER2_COMPA_vect, ISR_BLOCK)
{
  panel.update();
}
```  
### Enums
There is one enumeration for color

### Methods
These are the public methods of the library:  

`begin(bool useISR, void(*xlater)(uint8_t& x, uint8_t& y))` - this method will initialize the buffers and set up the internal state of the library.  Parameters:  
+ `bool useISR` - set to `true` if you want to use the Timer 2 interrupt for updating (this is the recommended method).  Set to `false` if you plan on using Timer 2 for something else, or just prefer to control the timing of the calls to `update()`.  When `true`, Timer 2 will be set up to trigger every 2 milliseconds. 
+ `void(*xlater)(uint8_t& x, uint8_t& y)` - this is a pointer to a function that can be used to translate and modify the X/Y coordinate system.  By default, the `(0,0)` point will be at the upper left of the panel.  It may be more convenient from a math standpoint to have the origin at the lower left corner.  In that case, the `Y` coordinate can be remapped to provide this by something as simple as:   
  ```
  void origin(uint8_t& x, uint8_t& y)
  {
    y = MAXROWS - y;
  }
  ```
This can also be used to translate the coordinate system so the panel can be rotated 90 or 270 degrees.

`update()` - this method needs to be called at a fairly high rate to provide a decent display.  The recommended method as in an ISR, preferably Timer 2 as would be configured in the `begin()` method.  `update()` has been measured to that 568 us (microseconds) to complete.  If using the Timer 2 update rate of 2 ms (milliseconds) as defined by the `begin()` method, about 1/4 of the ATMega's processing time will be taken up by `update()`, which is fairly reasonable.

`draw()` - all "drawing" on the panel done by the library methods are done to the local framebuffer and won't actually be written to the display panel until the `draw()` method is called.  This prevents flickering of the display.  Basically, draw whatever you want on the display, then call `draw()` to copy that to the output framebuffer that the `update()` method uses.
`clear()` - erases the display by setting all pixels to black (remember to call `draw()` for it to take affect)

`fillAll(Panel::Colors c)` - sets all pixels in the display to the specified color `c` (see enums and structs section for colors)

`setPixel(uint8_t x, uint8_t y, Panel::Colors c)`  
`setPixel(Panel::Point& p, Panel::Colors c)` - both of these set the specified pixel to color `c`

`line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Panel::Colors color)`  
`line(Panel::Point& start, Panel::Point& end, Panel::Colors c)` - both of these draw a line on the display, one pixel wide, using color `c` 

`rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Panel::Colors color, bool fill = true)`  
`rectangle(Panel::Point& topLeft, Panel::Point& btmRight, Panel::Colors c, bool fill = true)`  
`rectagle(Panel::Rect& rect, Panel::Colors c, bool fill = true)` - all of these 3 methods will draw a rectangle on the display in color `c`.  If `fill` is set to `true`, the rectangle will be filled with color `c`; otherwise it will be an outline one pixel wide


# Motivation
I was digging through my parts drawers when I came across the display panel.  I bought it from Adafruit some time ago for a project I never finished (sigh).  I went to the products page on Adafruit to remind myself what it was capable of: [Medium 16x32 RGB LED matrix panel - 6mm pitch](https://www.adafruit.com/product/420).

There was a line on the page that really caught my eye:
 
You'll need about 1600 bytes of RAM to buffer the 12-bit color image. **You cannot use this size panel with an Arduino UNO (ATmega328) or ATmega32u4 - you need a chip with more RAM!**   

(emphasis Adafruit, not mine)

Sure, if you want to do full 12-bit color, you'll need that much RAM, but what if you only did 8 colors?  With double buffering and byte packing, that should only be 512 bytes - a quarter of what the ATMega32 family has.  

I've also seen writeups that will say you need high speed microcontrollers or even an FPGA to drive them.  Can I do it with the resources of an ATMega32?  This MCU family are 8-bit AVRs running at 16MHZ with 2KB of RAM and 32KB of Flash memory.  

I happened to have some UNO-style boards in the same drawer, so I decided to give it a try.
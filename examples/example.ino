#include <panel.h>

Panel panel;

void bouncingBlock();

/******************************************
* origin()
*******************************************
* This method defines the coordinate trans-
* lation.  The "default" origin is the upper
* left corner of the display.  This method
* translates so that origin is the lower
* left.
*
* It will be passed the Panel class through
* the init() method - see below
******************************************/
void origin(int16_t& x, int16_t& y)
{
  y = MAXROWS - y;
}

void setup()
{

  panel.begin(true, &origin);

  // Pin 14 (PORT C, Pin 0) is for ISR timing
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
}



// kinda main()
void loop() 
{
  static uint32_t offset = millis();  // keep track of time
  char msg[10];                       // string for seconds.ms display
  static uint16_t msec = 0;           // 100 millisecond counter
  static uint16_t sec = 0;            // 1 second counter

  // wait for 100 ms or so to elapse.  Not an exact timer
  if (millis() - offset >= 100)
  {
    offset = millis();
    ++msec;

    // count hundred milliseconds to 10 (one second)
    if (msec == 10)
    {
      msec = 0;

      // increment seconds
      ++sec;

      // rollover seconds at 1000 (so it fits on panel)
      if (sec > 999)
      {
        sec = 0;
      }      
    }

    // write time to string in the format of X.X seconds
    snprintf(msg, 9, "%d.%d", sec, msec);
    msg[9] = '\0';

    // right-justify where the string goes on the panel
    int16_t timeXPosn = COLS - (strlen(msg) * 6) - 1;

    // start by clearing the panel
    panel.fillAll(Panel::BLACK);

    // draw the bouncing yellow block next (it will be behind everything else)
    bouncingBlock();

    // draw a green line next; the length of the line will grow as the tens of
    // seconds goes from zero to 9
    uint8_t len = (uint8_t)(sec % 10);
    panel.line(2, 2, len * 3 + 2, 2, Panel::GREEN);

    // draw the time text string
    panel.drawString(timeXPosn, 12, msg, Panel::CYAN);
    
    // draw a border around the outside of the panel
    panel.rectangle(0, 0, MAXCOLS, MAXROWS, Panel::MAGENTA, false);

    // Important!! Call this last after drawing everything else to update
    // the "active" framebuffer.  Miss this and the display will always
    // be blank
    panel.draw();
  }
}

/************************************************
* bouncingBlock()
*************************************************
* Simple animation of a 2x2 pixel yellow block 
* bouncing around the panel
************************************************/
void bouncingBlock()
{
  static int16_t x = 1;     // starting X coordinate
  static int16_t y = 11;    // starting Y coordinate
  static bool vdir = true;  // starting vertical direction (up)
  static bool hdir = true;  // starting horizontal direction (right)

  // draw the block at specified X/Y
  panel.rectangle(x, y, x + 1, y + 1, Panel::YELLOW);

  // Handle vertical move
  if (vdir)
  {
    // move positive (up) the panel
    ++y;

    // if we hit the border at the top, reverse direction
    if ((y + 2) >= MAXROWS)
    {
      vdir = false;
    }
  }
  else
  {
    // moving "down"
    --y;

    // if we hit the bottom border, reverse direction
    if (y == 1) 
    {
      vdir = true;
    }
  }

  // handle horizontal move
  if (hdir)
  {
    // moving right
    ++x;

    // if at the right border, reverse direction
    if ((x + 2) > MAXCOLS) 
    {
      hdir = false;
    }
  }
  else
  {
    // moving left
    --x;

    // if at the left border, reverse direction
    if (x == 1) 
    {
      hdir = true;
    }
  }  
}

/**********************************************
* TIMER 2 ISR
***********************************************
* This example is using the hardware Timer 2
* for updating the panel, so we need to include
* this ISR (Interrupt Service Routine).
*
* If you choose the "interrupt" method and forget
* to include the ISR, your Arduino board will 
* reset because there is no entry in the vector
* table!
************************************************/
ISR(TIMER2_COMPA_vect, ISR_BLOCK)
{
  // PORTC, Pin 0 (Arduino pin A0) is being used
  // to time the hardware update here.  Turn it
  // on before starting the update:
  bitSet(PORTC, 0);

  // output the contents of the active framebuffer
  // to the physical panel using GPIO pins
  panel.update();

  // Turn off PORTC, Pin 0.  Using a scope or logic
  // analyzer we can see how long the physical update
  // process takes (a bit over 1/2 ms)
  bitClear(PORTC, 0);
}

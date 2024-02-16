/**********************************************************
 * @file    panel_impl.h
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
#ifndef PANEL_IMPL_H_
#define PANEL_IMPL_H_

#ifndef SETBIT_CTL
#define SETBIT_CTL(a)  (PORTB |= bit(a))
#endif

#ifndef CLRBIT_CTL
#define CLRBIT_CTL(a)  (PORTB &= ~bit(a))
#endif

#ifndef _NOP
#define _NOP() __asm__ __volatile__("nop")
#endif

// Some definitions for I/O in terms of AVR register
// defined names, not Arduino pin numbers
#define PORT_RF PORTD
#define PORT_GF PORTD
#define PORT_BF PORTD
#define PORT_RS PORTD
#define PORT_GS PORTD
#define PORT_BS PORTD
#define PIN_RF  2
#define PIN_GF  3
#define PIN_BF  4
#define PIN_RS  5
#define PIN_GS  6
#define PIN_BS  7

#define PORT_RA PORTB
#define PORT_RB PORTB
#define PORT_RC PORTB
#define PIN_RA  0
#define PIN_RB  1
#define PIN_RC  2

#define PORT_CLK  PORTB
#define PORT_LAT  PORTB
#define PORT_OE   PORTB
#define PIN_CLK   3
#define PIN_LAT   4
#define PIN_OE    5

#endif // PANEL_IMPL_H_
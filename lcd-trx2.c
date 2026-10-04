/*
 Cheap DSP dsPICradio board LCD routines
 This is polling driver with 8-bit interface

 Juha Niinikoski OH2NLT 26.08.2005

 Modified for Digital RX hardware 12.09.2006
 MPLAB C30 version, OH2GWE, 2006.10.17

 IRQ mask added to LCD I/O port operations. IRQ tone output needs this protection.
 S-meter roll over corrected, interface changed to int 02.12.2006
 LCD HEX output added 09.07.2007
 PA100 temperature meter degrees sign added to the soft fonts #4 16.11.2008
 
 Removed the include statement for the file lcd-trx2.h. As there are only 5 definitions,
 and these definitions are never used anywhere else, it made more sense to include them
 in this file. - 5B4AIY 12-AUG-11
 
 Modified lcd_putchhex() to use conditional, which is more concise - 5B4AIY 28/AUG/11
 Removed lcd_putchhex(). It was only used by rs232_test(), and even there sprintf() and a format
 string was much more concise - 5B4AIY 26/NOV/11
 Modified lcd_putst() to use a more concise form - 5B4AIY 28/AUG/11
 Re-wrote draw_s_meter() to eliminate redundant variables and the use of two arrays that are
 unnecessary if the custom characters are ordered correctly in the CGRAM.
 Re-designed the custom characters so that the graphical meter display more closely resembles
 an edge-wise analogue meter. - 5B4AIY 07/OCT/2012
 Added feature to allow the user to select the type of graphic meter display. - 5B4AIY 08/OCT/2012
 Re-located the LCD hardware definitions from file: lcd-trx2.h to juma-pa100.h - 5B4AIY - 27/DEC/2013
 Optimised initlcd() to use a data array and a loop rather than in-line code. - 5B4AIY - 02/JAN/2014
 Re-wrote disp_meter() considerably simplifying it. - 5B4AIY - 27/JAN/2014
 Modified wait_lcd_ready() to remove a redundant assignment and superfluous mask operation, and reduce
 the second delay to 1uS. The original 2uS is needlessly long as the maximum setup delay time is only
 320nS, thus a 1uS delay is far more than is really required. - 5B4AIY - 17/AUG/2014
 Added display_line() and display_screen() functions, and removed the clear_lcd() function and replaced
 it by a macro. A.Ryan - 5B4AIY - 19/MAR/2015
*/
// Includes
#include <stdio.h>
#include "juma-pa100.h"

// External Functions
extern void us_delay(unsigned int);
extern void ms_delay(unsigned int);

// External Data
extern char lcdpbuff[];		// General purpose LCD display buffer

// Local Data
const char init_lcd[] = {				// Indexed by: local variable i in initlcd() function
						0x38,			// LCD reset, 8-bit interface, 1-line, 5*7 font
						0x38,
						0x38,
						0x0C,			// Cursor off, no blink
						0x01,
						0x06			// Increment, no display shift
						};

const char *scale_fmt[] = {				// Indexed by: Defined manifest constants, except for temperature
						"%5.0f\337F",	// 0 - Fahrenheit - Octal 337 = Degree Symbol. Indexed by: eeprom.defval.Temp_Units
						"%5.0f\337C",	// 1 - Celsius - Octal 337 = Degree Symbol. Indexed by: eeprom.defval.Temp_Units
						"%6.1fW",		// 2 - RF Power, Watts - Index constant: WATTS
						"SWR%4.1f",		// 3 - Index constant: SWR
						"%6.2fV",		// 4 - Volts - Index constant: VOLTS
						"%6.1fA",		// 5 - Amps - Index constant: AMPS
						"%4.1fdBm"		// 6 - RF Power, dBm - Index constant: DBM
						};

const double scale_factor[]	= {			// Indexed by: Defined manifest constants, except for temperature
						1.0,			// 0 - Fahrenheit. Indexed by: eeprom.defval.Temp_Units
						1.0,			// 1 - Celsius. Indexed by: eeprom.defval.Temp_Units
						100000000.0,	// 2 - RF Power - Index constant: WATTS
						100.0,			// 3 - Index constant: SWR
						1000000.0,		// 4 - Voltage, uV - Index constant: VOLTS
						200000.0,		// 5 - Amps - Index constant: AMPS
						0.1				// 6 - RF Power - Index constant: DBM
						};

void clk_lcd(unsigned char ldata)		// Set data & clock both LCD controllers
	{
	__asm__ volatile ("disi #9");		// Protect IRQ tone output to RD1 I/O
	LCD_DATA = (LCD_DATA & ~LCD_MASK) | ((ldata << LCD_SHIFT) & LCD_MASK);	// Set LCD data lines
	us_delay(1);						// Extra delay for filter board latch wires
	LCD_E = 1;
	us_delay(1);
	LCD_E = 0;
	us_delay(1); 						// Some delay
	}

/*
 By inserting a counter in the DO-WHILE loop and saving the accumulated count in a global variable that
 was updated if the accumulated count was greater, the maximum number of loop cycles could be determined.
 The data was displayed with some temporary code in the Serial Test Suite. During the initialisation
 the maximum count was 98. During normal display operation it was 2. Updating the internal custom
 character RAM seemed to take the most time. For the Juma TRX-2, the figures were 100/2. The slight difference
 is caused by the fact that the Juma TRX-2 instruction clock is 7.5MHz, whereas the PA-100D instruction cycle
 clock is 7.3728MHz. (The clock is derived from a 7.3728MHz crystal, and the oscillator is multiplied by 4 in
 an internal PLL, and then divided by 4 to give the basic instruction cycle time.) A.Ryan - 5B4AIY - 17/AUG/2014
 
 The maximum setup delay from LCD_E Lo to Hi to read data valid is 320nS. The minimum delay available is 1uS.
*/
void wait_lcd_rdy(void)					// Wait until LCD module is ready
	{
	unsigned int busy;					// LCD Busy flag

	__asm__ volatile ("disi #4");		// Protect IRQ tone output to RD1 I/O
	LCD_TRIS |= LCD_MASK;				// Make data port input, LCD bus direction bits = 1
	LCD_RW = 1;							// Read
	LCD_RS = 0;							// Command mode

	do	{								// Read port pins, mask, and test for busy.
		us_delay(1);					// On first execution, provides the setup delay from RS. On subsequent
		LCD_E = 1;						// executions, the minimum width delay for LCD_E from Lo to Hi.
		us_delay(1);					// Provides the minimum setup delay from LCD_E Lo to Hi to read data.
		busy = LCD_BUS & LCD_BUSY_FLAG;	// Read data from I/O port pins and extract busy flag
		LCD_E = 0;
		} while (busy);					// Test for busy

	LCD_RW = 0;
	LCD_RS = 1;
	__asm__ volatile ("disi #4");		// Protect IRQ tone output to RD1 I/O
	LCD_TRIS &= (~LCD_MASK);			// Return to write mode
	}

void lcd_cmd(unsigned char cmnd)
	{
	wait_lcd_rdy();
	LCD_RS = 0;							// Switch to command mode
	clk_lcd(cmnd);						// Set cursor command + cursor position
	LCD_RS = 1;							// Switch back to data mode
	}

void initlcd(void)						// Initialise selected LCD controller
	{
	int i = 0;

	LCD_RS = 0;							// Set command mode
	LCD_RW = 0;
	ms_delay(15);						// Start delay

	do	{
		clk_lcd(init_lcd[i]);			// Write LCD initialisation data
		ms_delay(5);					// 5mS - Manufacturer's spec: 4.1mS min
		} while (++i < 6);

	LCD_RS = 1;							// Set data mode
	set_pwm4_dac(DEFAULT_CONTRAST);		// Set default display contrast
	set_pwm3_dac(DEFAULT_BL);			// and default backlighting.
	}

void lcdoutch(unsigned char lcd_char)	// Output character to selected display
	{
	wait_lcd_rdy();
	clk_lcd(lcd_char);
	}

/* HD44780 character generator routines */
/* Modified for SOLOMON LM1125SYLU1 display */
void set_ch_bits(char data, int count)	// Set character generator bits. Data, number of characters.
	{
	while (count--) lcdoutch(data);
	}
/*
 The LCD display has an 8-character area of RAM that can be used for user-defined symbols. This RAM
 (CGRAM) starts at address 0 and uses an 8-byte array as a bitmap of the character to display.
 The bitmap is ordered such that the topmost row of pixels is the lowest address, and the bottom
 row of pixels is the highest address. The bottom row is usually used for the cursor, otherwise a
 blank row of pixels is used to separate the characters between the top and bottom lines of the LCD.
 The character bitmap itself is ordered so that the rightmost pixel is the LSB, and the leftmost
 pixel the MSB. Only 5 columns and 7 rows of pixels are actually used, there is a blank column
 of pixels between each character.
 There are a total of 8x8 = 64 bytes of RAM available.
 Note that the CGRAM address counter will wrap around after 64 bytes.
 By ordering the bar-graph custom characters in ascending sequence, it is possible to use the
 character code itself to indicate an empty block (0), or the partial blocks (0 - 5), or the
 full blocks.

 This version of the function allows for the selection of three types of custom character sets
 corresponding to the original, large, and small scale versions. The selection is made from the
 User Configuration Menu.
*/
void set_chgen(int type)	// Write bar symbols to HD44780 RAM character generator
	{
	lcd_cmd(0 | ADRSET);

	if(type == 2)				// Small scale markers
		{
// CGRAM 00 - Empty block, and part = 0
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x15, 2);	// @.@.@
								// @.@.@
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x00, 2);	// .....
								// .....

// CGRAM 01 (part = 1)
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x18, 1);	// @@...
		set_ch_bits(0x1D, 2);	// @@@.@
								// @@@.@
		set_ch_bits(0x18, 1);	// @@...
		set_ch_bits(0x00, 2);	// .....
								// .....

// CGRAM 02 (part = 2)
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x1C, 1);	// @@@..
		set_ch_bits(0x1D, 2);	// @@@.@
								// @@@.@
		set_ch_bits(0x1C, 1);	// @@@..
		set_ch_bits(0x00, 2);	// .....
								// .....

// CGRAM 03	(part = 3)
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x1E, 1);	// @@@@.
		set_ch_bits(0x1F, 2);	// @@@@@
								// @@@@@
		set_ch_bits(0x1E, 1);	// @@@@.
		set_ch_bits(0x00, 2);	// .....
								// .....

// CGRAM 04 (part = 4)
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x1F, 4);	// @@@@@
								// @@@@@
								// @@@@@
								// @@@@@
		set_ch_bits(0x00, 2);	// .....
								// .....

// CGRAM 05 - (part = 5) Fully-Filled Block
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x1F, 4);	// @@@@@
								// @@@@@
								// @@@@@
								// @@@@@
		set_ch_bits(0x00, 2);	// .....
								// .....

// CGRAM 06 - End-Of-Scale Marker (S_BAR)
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x10, 4);	// @....
								// @....
								// @....
								// @....
		set_ch_bits(0x00, 2);	// .....
								// .....
		}
	else if(type == 1)			// Large Scale Markers
		{
// CGRAM 00 - Empty block, and part = 0
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 2);	// @....
								// @....
		set_ch_bits(0x15, 2);	// @.@.@
								// @.@.@
		set_ch_bits(0x10, 2);	// @....
								// @....
		set_ch_bits(0x00, 1);	// .....

// CGRAM 01 (part = 1)
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x18, 1);	// @@...
		set_ch_bits(0x1D, 2);	// @@@.@
								// @@@.@
		set_ch_bits(0x18, 1);	// @@...
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x00, 1);	// .....

// CGRAM 02 (part = 2)
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x1C, 1);	// @@@..
		set_ch_bits(0x1D, 2);	// @@@.@
								// @@@.@
		set_ch_bits(0x1C, 1);	// @@@..
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x00, 1);	// .....

// CGRAM 03	(part = 3)
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x1E, 1);	// @@@@.
		set_ch_bits(0x1F, 2);	// @@@@@
								// @@@@@
		set_ch_bits(0x1E, 1);	// @@@@.
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x00, 1);	// .....

// CGRAM 04 (part = 4)
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x1F, 4);	// @@@@@
								// @@@@@
								// @@@@@
								// @@@@@
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x00, 1);	// .....

// CGRAM 05 - (part = 5) Fully-Filled Block
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x1F, 4);	// @@@@@
								// @@@@@
								// @@@@@
								// @@@@@
		set_ch_bits(0x10, 1);	// @....
		set_ch_bits(0x00, 1);	// .....

// CGRAM 06 - End-Of-Scale Marker
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 6);	// @....
								// @....
								// @....
								// @....
								// @....
								// @....
		set_ch_bits(0x00, 1);	// .....
		}
	else						// Original Scale
		{
//	CGRAM 00 - Empty Block and part = 0
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x10, 3);	// @....
								// @....
								// @....
		set_ch_bits(0x00, 3);	// .....
								// .....
								// .....

//	CGRAM 01 - part = 1
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 5);	// @....
								// @....
								// @....
								// @....
								// @....
		set_ch_bits(0x00, 2);	// .....
								// .....

//	CGRAM 02 - part = 2
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x10, 5);	// @....
								// @....
								// @....
								// @....
								// @....
		set_ch_bits(0x00, 2);	// .....
								// .....

//	CGRAM 03 - part = 3
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x14, 5);	// @.@..
								// @.@..
								// @.@..
								// @.@..
								// @.@..
		set_ch_bits(0x00, 2);	// .....
								// .....

//	CGRAM 04 - part = 4
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x14, 5);	// @.@..
								// @.@..
								// @.@..
								// @.@..
								// @.@..
		set_ch_bits(0x00, 2);	// .....
								// .....

// CGRAM 05 - Full Block and part = 5
		set_ch_bits(0x00, 1);	// .....
		set_ch_bits(0x15, 5);	// @.@.@
								// @.@.@
								// @.@.@
								// @.@.@
								// @.@.@
		set_ch_bits(0x00, 2);	// .....
								// .....

// CGRAM 06 - End-Of-Scale
		set_ch_bits(0x00, 2);	// .....
								// .....
		set_ch_bits(0x10, 3);	// @....
								// @....
								// @....
		set_ch_bits(0x00, 3);	// .....
								// .....
								// .....
		}

// CGRAM 07 - Degree Symbol (Duplicate of character 0xDF)
	set_ch_bits(0x00, 1);		// .....
	set_ch_bits(0x1C, 1);		// @@@.. 
	set_ch_bits(0x14, 1);		// @.@..
	set_ch_bits(0x1C, 1);		// @@@..
	set_ch_bits(0x00, 4);		// .....
								// .....
								// .....
								// .....

	clear_lcd();				// Back to normal mode (clear LCD)
	}

// Draw 8 character graphic meter scale = 0...48 (24 visible steps)
/*
 The original code copied the various custom characters to an 8-byte buffer and then
 cycled through the buffer to display the symbols. It was claimed that this avoided
 display jitter. The present method, whereby each symbol is displayed immediately, does
 not seem to suffer any obvious jitter, and so the display buffer has been eliminated.
 A.Ryan 5B4AIY - 05/OCT/12
*/
void draw_s_meter(int len)
	{
	int whole, part, cursor = 0;

	if(len > MAX_GRAPH)				// First ensure the display length is correct.
		len = MAX_GRAPH;

	whole = len / FONT_W;			// Calculate number of fully-filled blocks.
	part = len % FONT_W;			// Calculate the size of the partial block.

	do	{
		if(cursor < whole)			// If there are any full blocks to display,
			lcdoutch(FULL_BLOCK);	// then display them,
		else						// otherwise,
			{
			lcdoutch(part);			// display any partial block,
			part = 0;				// and then set it to the empty block symbol,
			}						// and fill the display.
		} while (++cursor < DISP_LEN);

	lcdoutch(S_BAR);				// Finally display the end-of-scale bar.
	}

// General I/O Functions
// Display null terminated string
void lcd_putst(register const char *str)
	{
	while (*str) lcdoutch(*str++);	// A shorter way of writing this. - 5B4AIY
	}

// Display a line of text at a specified cursor position.
void display_line(int command, const char *text)
	{
	lcd_cmd(command);
	lcd_putst(text);
	}

// Display a complete screen of text.
void display_screen(const char *line_1, const char *line_2)
	{
	display_line(CLEAR, line_1);
	display_line(LINE2,line_2);
	}

// Display PA-100D analog meter formats
void disp_meter(double value, int index)
	{
	sprintf(lcdpbuff, scale_fmt[index], value / scale_factor[index]);
	lcd_putst(lcdpbuff);
	}


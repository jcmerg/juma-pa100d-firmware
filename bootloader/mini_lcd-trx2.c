/*
 Minimal LCD interface for JUMA-TRX2 flasher 
 Juha Niinikoski OH2NLT 23.01.2007
 MPLAB C30

*/


// External functions
extern void us_delay( unsigned int);
extern void ms_delay( unsigned int);

// Includes
#include "juma-pa100.h"

// 2 * 16 LCD display memory layout

#define LINE1     0x00         				/* line 1 start address */
#define LINE2     0x40         				/* line 2 start address */

#define CURSET     0x80        				 /* set memory cursor */
#define	CLEAR	   0x01						/* clear display */
#define ADRSET	   0x40						/* set cgram address */

#define TICK_PERIOD 7500					// 1ms tick, PWM clock

const char msg1[] = {"JUMA-PA100 Flash"};
const char msg2[] = {"Writer started.."};

void	clk_lcd(unsigned char ldata)		// set data & clock both lcd controllers
	{
	LCD_DATA = (LCD_DATA & ~LCD_MASK) | ((ldata<<LCD_SHIFT) & LCD_MASK);	// set LCD data lines
	us_delay(1);							// extra delay for filter board latch wires
	LCD_E = 1;
	us_delay(1);
	LCD_E = 0;
	ms_delay(1);
	}

void set_cur_lcd(unsigned char cur)		// set cursor position at selected display
	{
	LCD_RS = 0;							// switch command mode		
	clk_lcd(cur | CURSET);				// set cursor command + cursor position
	LCD_RS = 1;							// back to data mode
	ms_delay(2);
	}

#if 0
void clear_lcd(void)					// clear selected display
	{
	LCD_RS = 0;							// switch command mode		
	clk_lcd(CLEAR);						// clear display. Display going to be busy 1,6 mS
	LCD_RS = 1;							// back to data mode
	ms_delay(3);
	}
#endif

void initlcd(void)						// init selected LCD controller
	{
// init required I/O
	TRISC = INIT_TRISC;					// sideband select & switches
	PORTC = INIT_PORTC;

	TRISD = INIT_TRISD;					// LEDs & LCD pins = output
	PORTD = INIT_PORTD;

//Power switch
	PWR_ON = 1;							// secure power ON

	LCD_RS = 0;							// set command mode
	LCD_RW = 0;
	ms_delay(15);						// start delay
	clk_lcd(0x38);						// write init data with delays
	ms_delay(5);
	clk_lcd(0x38);
	ms_delay(1);
	clk_lcd(0x38);						//8-bit, 2 rows, 5x7 matrix, display on
	ms_delay(1);						//cursor off, no blink
	clk_lcd(0x0C);
	ms_delay(1);
	clk_lcd(0x01);
	ms_delay(2);	
	clk_lcd(0x06);						// increment, no display shift
	ms_delay(1);
	LCD_RS = 1;							// set data mode

// set back light & contrast PWM outputs
	OC3CON = 0;							// first turn off the module if it was on
	OC4CON = 0;

// LCD PWM DAC setup, use TMR2 here, autobaud function uses TMR3
	OC3RS = 300;						// some value for back light
	OC3R = 300;
	OC3CON = 0x0006;					// use TMR2, OC3 = PWM mode

	OC4RS = 2000;						// something for contrast to make display visible
	OC4R = 2000;
	OC4CON = 0x0006;					// use TMR2, OC4 = PWM mode

	PR2 = TICK_PERIOD - 1;				// set PWM cycle, TMR3 period register, 1000Hz/1ms
	T2CONbits.TON = 1;					// start TMR3
	}


void lcd_putst(register const char *str)
	{
	while((*str)!=0)
		{
		clk_lcd(*str);
		str++;
		}
	}

// Init LCD & display Flasher started message

void flasher_message(void)
	{
	initlcd();
	lcd_putst(msg1);
	set_cur_lcd(LINE2);
	lcd_putst(msg2);
	}

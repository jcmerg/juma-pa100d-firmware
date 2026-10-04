// Simple Trap handler for dsPIC30F6014 / JUMA-TRX2
// OH2NLT Juha Niinikoski 07.06.2005

// LCD Error message added 26.12.2006

#include "juma-pa100.h"
#include <stdio.h>			// Added - 5B4AIY

extern void lcd_putst(register const char *);
extern void lcd_cmd(unsigned char);

#define OSC_TRAP	0x06	// Oscillator trap
#define ADR_TRAP	0x08	// Address trap
#define STK_TRAP	0x0A	// Stack trap
#define MAT_TRAP	0x0C	// Math Error trap

int w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,w10,w11,w12,w13,w14,w15;	// Temp save for registers

void lcd_error_message(int err)		// Moved to here to avoid forward references - 5B4AIY
	{
	clear_lcd();
	switch(err)
		{
		case 0:
			lcd_putst("Oscillator Trap!");
		break;

		case 1:
			lcd_putst(" Address Trap!  ");
		break;

		case 2:
			lcd_putst("  Stack Trap!   ");
		break;

		case 3:
			lcd_putst("  Math Error!   ");
		break;

		case 4:
			lcd_putst("Interrupt Error!");
		break;

		default:
			lcd_putst(" Unknown Error! ");
		break;
		}

	for(;;)
		{
		if(PWR_SW == 1)				// Try power-off if PWR button pressed
			PWR_ON = 0;
		}
	}

void __attribute__ ((interrupt)) _OscillatorFail(void)
	{
	int x;
//	DI();
 	U1BRG = ((8000000 / 16) / 9600) - 1;		// Try to set set UART1 baud rate to 9600

	for(x = 0; x < 32000; x++)
		{
		asm("nop");
		}
	printf("\n\rTRAP: Oscillator Error\n\r");	// May not print correctly if clock is gone
	printf("OSCCON 0x%X\n\r", OSCCON);

	lcd_error_message(0);
	for(;;);
	}

void __attribute__ ((interrupt)) _AddressError(void)
	{

//	DI();

	asm("mov w0, _w0");				// Save registers
	asm("mov w1, _w1");
	asm("mov w2, _w2");
	asm("mov w3, _w3");
	asm("mov w4, _w4");
	asm("mov w5, _w5");
	asm("mov w6, _w6");
	asm("mov w7, _w7");
	asm("mov w8, _w8");
	asm("mov w9, _w9");
	asm("mov w10, _w10");
	asm("mov w11, _w11");
	asm("mov w12, _w12");
	asm("mov w13, _w13");
	asm("mov w14, _w14");
	asm("mov w15, _w15");

	printf("\n\rTRAP: Address Error\n\r");

	printf("W0 0x%X\n\r", w0);		// Print registers
	printf("W1 0x%X\n\r", w1);
	printf("W2 0x%X\n\r", w2);
	printf("W3 0x%X\n\r", w3);
	printf("W4 0x%X\n\r", w4);
	printf("W5 0x%X\n\r", w5);
	printf("W6 0x%X\n\r", w6);
	printf("W7 0x%X\n\r", w7);
	printf("W8 0x%X\n\r", w8);
	printf("W9 0x%X\n\r", w9);
	printf("W10 0x%X\n\r", w10);
	printf("W11 0x%X\n\r", w11);
	printf("W12 0x%X\n\r", w12);
	printf("W13 0x%X\n\r", w13);
	printf("W14 0x%X\n\r", w14);
	printf("W15 0x%X\n\r", w15);

	lcd_error_message(1);
	for(;;);
	}

void __attribute__ ((interrupt)) _StackError(void)
	{
//	DI();
	asm("mov w15, _w15");
	asm("mov #0x900, w15");			// New temporary stack from hat

	printf("\n\rTRAP: Stack Error\n\r");
	printf("W15 0x%X\n\r", w15);

	lcd_error_message(2);
	for(;;);
	}

void __attribute__ ((interrupt)) _MathError(void)
	{
//	DI();
	printf("\n\rTRAP: Math Error\n\r");

	lcd_error_message(3);
	for(;;);
	}

void __attribute__ ((interrupt)) _DefaultInterrupt(void)
	{
	_INT4IE = 0;
	_DCIIE = 0;
	printf("\n\rTRAP: Spurious Interrupt Error\n\r");

	lcd_error_message(4);
	for(;;);
	}


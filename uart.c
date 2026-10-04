// Simple UART interface for dsPIC30F6012
// Juha Niinikoski, OH2NLT 18.05.2005

// Below is the code to initialize the USART
// 8 bits, 1 start and 1 stop, no error handling
// kbhit() and getch() added 22.05.2005
// getche() added 02.06.2005
// Overrun error handling added 29.08.2005
// MPLAB C30 conversion, OH2GWE 2006.10.17
// Baud Rate setup & IRQ receiver for fast baud rates 14.01.2008
// Adaptation to JUMA-PA100 board, FCY = 7,3728MHz, 07.07.2008
// UART initialisation sequence changed 30.10.2008
// RX buffer pointer cast corrections 18.11.2008

// Note the changes to cvt_hex() to eliminate a compiler warning message - 5B4AIY 12-AUG-11
// get2hex() re-written to eliminate the possibility of collecting non-HEX characters. 5B4AIY 28/AUG/11

#include "juma-pa100.h"
#include <stdio.h>
#include <ctype.h>		// Added - 5B4AIY

/*
	Baud Rate divisors for 30 MHz osc = 7.5 MHz FCY
	Actual baud rate divisors
	Divisor = (FCY / (Baud Rate * 16)) - 1
 [0] 1200	389.625
 [1] 2400	194.3125
 [2] 4800	96.65625
 [3] 9600	47.828125
 [4] 19200	23.4140625
 [5] 38400	11.20703125
 [6] 57600	7.138020833
 [7] 115200	3.069010417
*/

/*
	Baud Rate divisors for 29,4912 MHz osc = 7.3728 MHz FCY
	Actual baud rate divisors
	Divisor = (FCY / (Baud Rate * 16)) - 1
 [0] 1200	383.0
 [1] 2400	191.0
 [2] 4800	95.0
 [3] 9600	47.0
 [4] 19200	23.0
 [5] 38400	11.0
 [6] 57600	7.0
 [7] 115200	3.0
*/

// Module variables & definitions
static volatile unsigned char rx_buffer[256];		// UART mod(256) RX queue, must be exactly 256 long
static volatile unsigned char rx_buf_out_idx = 0;	// Buffer output index, pointer behind in idx
static volatile unsigned char rx_buf_in_idx = 1;	// Buffer input index, pointer always "leading"

/*
const unsigned int baud_rates[] = {			// 7,5MHz
									390,	// 0 - 1,200 Baud
									194,	// 1 - 2,400 Baud
									97,		// 2 - 4,800 Baud
									48,		// 3 - 9,600 Baud
									23,		// 4 - 19,200 Baud
									11,		// 5 - 38,400 Baud
									7,		// 6 - 57,500 Baud
									3		// 7 - 115,200 Baud
									};
*/

const unsigned int baud_rates[] = {			// Divisor = (7,3728,000 / (Baud Rate * 16)) - 1 
									383,	// 0 - 1,200 Baud
									191,	// 1 - 2,400 Baud
									95,		// 2 - 4,800 Baud
									47,		// 3 - 9,600 Baud
									23,		// 4 - 19,200 Baud
									11,		// 5 - 38,400 Baud
									7,		// 6 - 57,500 Baud
									3		// 7 - 115,200 Baud
									};

// UART1 IRQ service
// Queue character
void __attribute__((interrupt)) _U1RXInterrupt(void)	// Put received character to RX queue
	{
  	unsigned char rx;

  	rx = U1RXREG;						// Data read

  	if((unsigned char)(rx_buf_in_idx + 1) != rx_buf_out_idx)	// Check for overrun
  		{
  		rx_buffer[rx_buf_in_idx++] = rx;// Place just received character in queue
    	}
// Else reject character
// Check & handle possible errors
	if(U1STAbits.OERR == 1)				// OERR is blocking UART, should not happen but...
		U1STAbits.OERR = 0;				// Reset overrun error if occurred

// This is not automatic in 30F6014 UART
	IFS0bits.U1RXIF = 0;				// Reset IRQ flag
	}

// UART1 init
void InitUSART1(void)
	{
	U1BRG = baud_rates[3];				// Set initial RS-232 speed to 9,600 Baud
	U1MODE = 0x0000;
	U1STA = 0x0000;						// IRQ on every character
	IFS0bits.U1RXIF = 0;				// Clear possible pending IRQ
	IEC0bits.U1RXIE = 1;				// Enable RX IRQs
	U1MODEbits.UARTEN = 1;				// Enable UART
	U1STAbits.UTXEN = 1;				// Enable transmitter
	}

// Set Baud rate
void SetUSART1baud(int baud_sel)
	{
	if(baud_sel > 7) baud_sel = 7;		// Stay in the table

	 U1BRG = baud_rates[baud_sel];		// Set Baud rate
	}

// Print character
void putch(unsigned char c)
	{
	while(U1STAbits.UTXBF == 1);		// Wait here if TX FIFO full
	U1TXREG = (unsigned int)c;			// TX register is 16-bits long
	}

// Check if key pressed, data available from receive buffer
// Not pressed = 0, Pressed = number of characters
// IRQ based RX queue check
unsigned char kbhit(void)
	{
  	return(unsigned char)((rx_buf_in_idx - (unsigned char)(rx_buf_out_idx + 1)));	// Return with number of available characters
	}

// Clear serial port, reset input buffer
void ClearUSART1queue(void)
	{
	rx_buf_out_idx = 0;					// Buffer output index
	rx_buf_in_idx = 1;					// Buffer input index
	}

// Get character from RX queue, IRQ RX system
unsigned char getch(void)
	{									// The compiler produces exactly the same code for: while(kbhit() == 0);
	while (!kbhit());					// Wait for data if queue is empty.
  	return rx_buffer[++rx_buf_out_idx];	// Return with data.
	}

// Receive character with echo
unsigned char getche(void)
	{
	unsigned char c;

	putch(c = getch());
	return c;
	}

// Some debug I/O functions
// Get unsigned long
unsigned long getlong(void)
	{
	unsigned long val = 0;
	unsigned char c = 0;

	do {
		c = getch() & 0x7F;				// getch() will wait for a character if buffer is empty.

		if(isdigit(c))
			{
			putch(c);
			val = 10 * val + (c - '0');
			}
		} while (c != 0x0D);

	return val;
	}

// Convert character to HEX nibble
unsigned char cvt_hex(void)
{	// Re-wrote this function to simplify the construction and eliminate a compiler warning - 5B4AIY
	unsigned char c;

	while (!isxdigit(c = getch()));		// Get HEX digit. Non-HEX characters ignored.

	c = toupper(c);						// Convert to upper case, as we don't need lower case
	putch(c);							// Display character
	c -= (c <= '9') ? '0' : '7';
	return c;
}

// Get byte (2 HEX characters)
unsigned char get2hex(void)
{	// This function has been re-written to eliminate collecting non-HEX digits. - 5B4AIY
	unsigned char c;

	c = cvt_hex() * 16;					// Convert character to HEX digit * 16
	c += cvt_hex();						// Combine with first character
	return c;
}


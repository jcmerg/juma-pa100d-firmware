// Delays with dsPIC30 TMR5
// Juha Niinikoski, OH2NLT 24.11.2005

//  Microchip C30 compiler

// Delay routines with TMR5

// Microsecond delay
// Max delay 2000us @ 120MHz sysclock, 1,03us / round
// Max delay 8000us @ 30MHz sysclock, 1,06us / round

#include "juma-pa100.h"

static void tmr5_wait(unsigned int cycles)
	{
	TMR5 = 0;							// Reset timer
	PR5 = cycles;						// Delay in instruction cycles
	IFS1bits.T5IF = 0;					// Clear flags
	T5CONbits.TON = 1;					// Start delay

	while(IFS1bits.T5IF == 0);			// Wait here, do nothing

	T5CONbits.TON = 0;					// Timer off and leave
	}

// DLYCONST is rounded up (8 cycles = 1.085uS), so that the LCD timing is never shorter than specified.
void us_delay(unsigned int dly)
	{
	tmr5_wait(dly * DLYCONST);			// Calculate delay
	}

// Millisecond delay
// Maximum delay 65535ms. Previously us_delay(1000), which with the rounded up DLYCONST took 1.085mS. DL4JC
void ms_delay(unsigned int dly)
	{
	while(dly--) tmr5_wait((unsigned int)(FCY / 1000UL));	// 1ms delay
	}


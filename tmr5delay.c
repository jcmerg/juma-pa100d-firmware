// Delays with dsPIC30 TMR5
// Juha Niinikoski, OH2NLT 24.11.2005

//  Microchip C30 compiler

// Delay routines with TMR5

// Microsecond delay
// Max delay 2000us @ 120MHz sysclock, 1,03us / round
// Max delay 8000us @ 30MHz sysclock, 1,06us / round

#include "juma-pa100.h"

void us_delay(unsigned int dly)
	{
	TMR5 = 0;							// Reset timer
	PR5 = dly * DLYCONST;				// Calculate delay
	IFS1bits.T5IF = 0;					// Clear flags
	T5CONbits.TON = 1;					// Start delay

	while(IFS1bits.T5IF == 0);			// Wait here, do nothing

	T5CONbits.TON = 0;					// Timer off and leave
	}

// Millisecond delay
// Maximum delay 65535ms
void ms_delay(unsigned int dly)
	{
	while(dly--) us_delay(1000);		// 1ms delay
	}


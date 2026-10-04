//
// Delays with dsPIC30 TMR5
// Juha Niinikoski, OH2NLT 24.11.2005
//
//  Microchip C30 compiler
// 
//

// delay routines with TMR5

// microsecond delay
// max delay 2000us @ 120MHz sysclock, 1,03us / round
// max delay 8000us @ 30MHz sysclock, 1,06us / round

#include "juma-pa100.h"

void us_delay( unsigned int dly )
	{
	TMR5 = 0;							// reset timer
	PR5 = dly * DLYCONST;				// calculate delay
	IFS1bits.T5IF = 0;					// clear flags
	T5CONbits.TON = 1;					// start delay

	while(IFS1bits.T5IF == 0);			// wait here, do nothing
	
	T5CONbits.TON = 0;					// timer off and leave
	}

// millisecond delay
// max delay 65535ms
void ms_delay( unsigned int dly )
	{
	unsigned int x;
	for(x=0; x<dly; x++)
		{
		us_delay(1000);			// 1ms delay
		}
	}

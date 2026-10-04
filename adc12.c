// JUMA-TRX2 30F6014A ADC12 routines
// Juha Niinikoski OH2NLT 25.11.2006

// Microchip C30 compiler version

// Includes
#include "juma-pa100.h"

extern void us_delay(unsigned int);		// Added - 5B4AIY

// Init ADC system for JUMA-TRX2 board
// Remember to set ADC pins as port input
// Reference from AVCC

void init_adc12(void)
	{
	ADCSSL	=	0x0000;					// Scan select
//	ADPCFG	=	0x01FF;					// AN9 and AN15 in use
	ADPCFG	=	0x81FF;					// AN9 to AN14 in use
//	ADCHS	=	0x000F;					// Select CH15, Vref- for MUXA
	ADCHS	=	0x000E;					// Select CH14, Vref- for MUXA
	ADCON3	=	0x0080;					// Internal RC clock
	ADCON2	=	0x0000;					// Use MUXA, do not scan inputs
//	ADCON1	=	0x8004;					// ADON, format = integer, manual sampling, auto start
	ADCON1	=	0x8000;					// ADON, format = integer, manual sampling, manual start
	}
/*
 Convert selected channel
 If the delay time is removed, then the amplifier will not exit to boot phase, there has to be some delay.
 A delay of 1uS is sufficient, and all channels convert and display correctly. The 100uS delay is more
 than enough to ensure correct conversion and reasonably fast loop cycle time of around 3mSec when
 sequentially converting all 6 channels. The results were:
 1uS/2.4mS, 25uS/2.7mS, 50uS/2.9mS, 100uS/3.2mS (Display Page 0, RF Power) A.Ryan - 5B4AIY - 08/JUN/2016 
*/
int convert_adc12(unsigned int adchs)
	{
	ADCHS = (adchs & 0x000F);			// Select CH, Vref- for MUXA
	ADCON1bits.SAMP = 1;				// Start sampling
	us_delay(50);						// Changed delay from 500uS to 50uS to improve loop cycle time.
	ADCON1bits.SAMP = 0;				// Start conversion

	while(ADCON1bits.DONE == 0);		// Wait for ready

	return(ADCBUF0);					// Return with buff#0
	}


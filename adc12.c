// JUMA-TRX2 30F6014A ADC12 routines
// Juha Niinikoski OH2NLT 25.11.2006

// Microchip C30 compiler version
/*
 Interrupt driven sampling. All A-D conversions are now made in the 1mS timer interrupt, _T3Interrupt() in timers_pwm.c,
 by adc_tick(). This allows the SWR protection to run in the interrupt, independently of the main loop, which may be
 blocked for long periods (button waits, save prompts, serial test commands). In each 1mS tick:

	REV_PWR	has been sampling since the previous tick, so it is converted immediately,
	FWD_PWR	is then sampled for 20uS and converted, so the forward and reverse values are taken about 40uS apart,
	one of ID_CUR, BATT_CH, Y_817 or TEMP in rotation is sampled for 20uS and converted, so each of these is
			updated every 4mS,
	REV_PWR	is then selected and left sampling until the next tick.

 The latest value of each channel is held in adc_raw[], indexed in the same order as AD_Values[]. convert_adc12() keeps its
 original interface, but now simply returns the latest value of the requested channel, so the A-D converter is never
 accessed from two places at once. The interrupt adds approximately 100uS every 1mS, but the main loop no longer spends
 approximately 430uS per cycle converting the six channels.
*/

// Includes
#include "juma-pa100.h"

#define ADC_CHANNELS	6				// Channels 9 - 14, see the A-D channel definitions in juma-pa100.h
#define SLOW_CHANNELS	4				// Number of channels converted in rotation
#define ADC_GUARD		500				// Maximum polling loops waiting for the end of conversion, approx. 340uS

extern void ms_delay(unsigned int);

volatile unsigned int adc_raw[ADC_CHANNELS];	// Latest A-D values, indexed by: channel - ID_CUR
volatile int adc_running = FALSE;				// Set once the A-D converter has been initialised

static const unsigned int slow_ch[SLOW_CHANNELS] = {ID_CUR, BATT_CH, Y_817, TEMP};
static int slow_idx = 0;

// Init ADC system for JUMA-TRX2 board
// Remember to set ADC pins as port input
// Reference from AVCC

void init_adc12(void)
	{
	ADCSSL	=	0x0000;					// Scan select
//	ADPCFG	=	0x01FF;					// AN9 and AN15 in use
	ADPCFG	=	0x81FF;					// AN9 to AN14 in use
//	ADCHS	=	0x000F;					// Select CH15, Vref- for MUXA
	ADCHS	=	REV_PWR;				// Select reverse power channel, Vref- for MUXA, first channel converted in adc_tick()
	ADCON3	=	0x0080;					// Internal RC clock
	ADCON2	=	0x0000;					// Use MUXA, do not scan inputs
//	ADCON1	=	0x8004;					// ADON, format = integer, manual sampling, auto start
	ADCON1	=	0x8000;					// ADON, format = integer, manual sampling, manual start
	ADCON1bits.SAMP = 1;				// Start sampling the first channel
	adc_running = TRUE;					// Allow adc_tick() to run,
	ms_delay(10);						// and wait until every channel has been converted at least once.
	}

// Sample the selected channel for 20uS. Only used within adc_tick(), the 1mS timer interrupt.
static void adc_sample(unsigned int channel)
	{
	ADCHS = (channel & 0x000F);			// Select CH, Vref- for MUXA
	ADCON1bits.SAMP = 1;				// Start sampling
	__asm__ volatile ("repeat #146\n\tnop");	// 147 cycles = 20uS sampling time at 7.3728MHz. (The original used 50uS,
										// but 1uS was found to be sufficient, see the notes for v3.00b.)
	}

// Convert the channel that is being sampled and store the result. Only used within adc_tick().
static void adc_convert(unsigned int channel)
	{
	unsigned int guard = ADC_GUARD;

	ADCON1bits.DONE = 0;				// Ensure that we do not see the previous conversion,
	ADCON1bits.SAMP = 0;				// and start the conversion.

	while(!ADCON1bits.DONE && --guard);	// Wait for ready, but never hang the interrupt.

	if(guard) adc_raw[channel - ID_CUR] = ADCBUF0;	// If the conversion failed, keep the previous value.
	}

// Called every 1mS from _T3Interrupt() in timers_pwm.c, after the frequency counter has been serviced.
void adc_tick(void)
	{
	if(!adc_running) return;

	adc_convert(REV_PWR);				// Reverse power has been sampling since the previous tick.
	adc_sample(FWD_PWR);
	adc_convert(FWD_PWR);
	adc_sample(slow_ch[slow_idx]);		// One of the slowly changing channels, in rotation.
	adc_convert(slow_ch[slow_idx]);

	if(++slow_idx >= SLOW_CHANNELS) slow_idx = 0;

	ADCHS = REV_PWR;					// Leave the reverse power channel sampling until the next tick.
	ADCON1bits.SAMP = 1;
	}
/*
 Return the latest value of the selected channel. The conversion itself is made in adc_tick().
 Previously the conversion was made here, see the notes for v3.00b concerning the sampling delay.
*/
int convert_adc12(unsigned int adchs)
	{
	if(adchs < ID_CUR || adchs > TEMP) return 0;

	return (int)adc_raw[adchs - ID_CUR];
	}

// dsPIC30 timers PWM DAC for JUMA-PA100
// Juha Niinikoski, OH2NLT 06.07.2008

// Microchip C30 compiler conversion

// JUMA-PA100 Timer & PWM system
/*
 Timer & PWM unit usage
 TMR1 is used for frequency counter
 TMR2 is used for tone generation, IRQ driven system, RD1/OC2 = tone out
 TMR3 is used for 1ms timer tick IRQ and for LCD PWM DAC timebase
 OC3 output is for LCD back light control
 OC4 output is for LCD contrast
 OC3 and OC4 are connected to TMR3
 TMR4 
 TMR5 used for delays (separate module)
 Encoder simulator repeat logic added 13.08.2008
*/
#include "juma-pa100.h"
#include "pa100_eeprom.h"			// Get EEPROM structure definitions

//#define TICK_PERIOD 7500			// 1ms tick
//#define TICK_PERIOD		(FCY / 1000)// 1ms tick (Old setting, prior to frequency counter modification)
#define TICK_PERIOD		7378		// New period, frequency counter

// External Data
extern const unsigned int band_limits[];	// Band limits defined in the main
extern unsigned int alarms;			// Alarm flag
extern int loop_ctr;				// Alarm blink counter
extern int dsp_alarm;				// Alarm message display flag. Set in display_alarms()
extern int pa_state;
extern int beep_flag;
extern int rep_dly;					// UP/DOWN button auto-repeat timer
extern int pa_state;
extern long input_freq;

// Local Data
unsigned int freq;					// Frequency counter
unsigned int cmd_timeout;			// Serial message timer
unsigned int polling_timer;			// Band data query, and remote status and monitoring polling timer
unsigned int button_timer;			// Long push timer, count from set value to zero, global visibility
unsigned int rmt_timeout;			// Remote mode command time-out
int decay_counter;					// Power meter slow decay

// Band selector
int band_bins[10];					// Found samples
int freq_sample_ctr;				// Sample counter
int freq_ctr;						// Input frequency sample averaging counter

static unsigned int tone_counter; 	// Beep tone length (ms)
static unsigned int busy_counter;	// Tone generator busy(ms), busy if != 0
static int last_cycle;				// For click-less end trick

// Encoder Simulator
static unsigned int up;				// UP button shift register
static unsigned int dn;				// DOWN button shift register
static int up_pushed;				// Up pushed status
static int dn_pushed;				// Down pushed status
static int rep_up;					// Repeat timers (ms)
static int rep_dn;
static int rep_up_dly;				// Repeat start delay (ms)
static int rep_dn_dly;
int enc = 0;						// Encoder accumulator

// EEPROM structures
extern struct
	{
	struct defval defval;
	} eeprom;

extern struct
	{
	struct calval calval;
	} cal;

// A-D converter, see adc12.c
extern void adc_tick(void);
extern volatile unsigned int adc_raw[];

// TX protection, see tx_guard()
#define MAIN_TIMEOUT	2000		// mS without a main loop cycle before RF is forced off
#define KEY_OFF_TICKS	2			// KEY must be inactive for this many mS before RF is forced off
#define SWR_BLANK		20			// mS after TX_ON is turned on before the SWR is tested (relay settling)
#define SWR_MS_PER_SAMPLE	4		// SWR averaging window in mS per Power Averaging sample (approx. one main loop cycle)
#define SWR_MIN_WINDOW	8			// Minimum SWR averaging window, mS

volatile unsigned int main_heartbeat = 0;	// mS since the main or service loop last ran, reset by those loops
volatile int isr_swr_trip = FALSE;			// Set here, transferred to the alarms in check_alarms()

static unsigned long fwd_sum, rev_sum;		// SWR measurement sums
static int swr_count;						// Number of samples in the sums
static int key_off_count;					// Number of consecutive mS that KEY has been inactive
static int swr_blank;						// SWR test blanking counter, mS
/*
 TX Protection
 Previously all the protection was in the main loop. Whenever the main loop was blocked, for example waiting for a button
 to be released, a save prompt, or a serial test command, the alarms were not checked and TX_ON stayed in whatever state
 it was. This function is called every 1mS from _T3Interrupt() and forces TX_ON off if:

	KEY has been inactive for KEY_OFF_TICKS mS (the transceiver has stopped transmitting),
	the SWR exceeds the trip limit (only in the OPERATE state, as in check_alarms()),
	there is an active alarm (except the Low-Voltage pre-limit warning),
	the main loop has not run for MAIN_TIMEOUT mS.

 It never turns TX_ON on, that remains the responsibility of the main loop. The main loop turns it on again when the
 condition has cleared, KEY is active, and there are no alarms.

 The SWR test is only made while TX_ON is on, and not for the first SWR_BLANK mS, as the relays are still settling and
 short reflected power peaks are normal. (The SWR alarm in check_alarms() still covers the OPERATE state without TX.)
 The forward and reverse values, with the low power offset added to the forward value as in analog_measurements(), are
 averaged over a window of Power Averaging (cal.calval.samples) * SWR_MS_PER_SAMPLE mS, minimum SWR_MIN_WINDOW mS. In the main
 loop each sample took one loop cycle of approx. 4mS, so this gives about the same sensitivity. Testing every single mS
 without averaging caused nuisance trips, e.g. with a non-resonant antenna at SWR 2.4:1 and the trip limit at 3.0:1.
 Rather than calculate the SWR, the reflection coefficient is compared with that of the trip limit:

	SWR > T  <=>  Ro > (T - 1) / (T + 1)  <=>  rev * (T + 1) > fwd * (T - 1)

 With the SWR scaled by 100, as in SWR_Trip, this becomes: rev * (SWR_Trip + 100) > fwd * (SWR_Trip - 100)
 Since the sums are compared, the division by the number of samples is not required. The maximum product is
 64 * (4095 + 150) * 1000, which fits easily in an unsigned long.
*/
static void tx_guard(void)
	{
	unsigned int trip = SWR_Trip;
	int window = (int)cal.calval.samples;

	if(window < SAMPLE_MIN || window > SAMPLE_MAX) window = SAMPLE_MIN;	// Protect against a corrupted setting.

	window *= SWR_MS_PER_SAMPLE;

	if(window < SWR_MIN_WINDOW) window = SWR_MIN_WINDOW;

// SWR
	if(!TX_ON)									// Not transmitting, so restart the blanking time,
		swr_blank = SWR_BLANK;
	else if(swr_blank)							// or wait until the relays have settled.
		swr_blank--;

	if(swr_blank)
		{
		fwd_sum = rev_sum = 0UL;
		swr_count = 0;
		}
	else
		{
		fwd_sum += (unsigned long)(adc_raw[FWD_PWR - ID_CUR] + cal.calval.lo_pwr_offset);
		rev_sum += (unsigned long)adc_raw[REV_PWR - ID_CUR];
		swr_count++;
		}

	if(swr_count >= window)
		{
		if(pa_state
			&& (fwd_sum >= (unsigned long)PWR_MTR_DEAD_BAND * (unsigned long)swr_count)	// Only if there is some power,
			&& (rev_sum * (unsigned long)(trip + 100) > fwd_sum * (unsigned long)(trip - 100)))
			isr_swr_trip = TRUE;

		fwd_sum = rev_sum = 0UL;
		swr_count = 0;
		}
// KEY
	if(KEY) key_off_count = 0;
	else if(key_off_count < KEY_OFF_TICKS) key_off_count++;
// Main loop watchdog
	if(main_heartbeat < MAIN_TIMEOUT) main_heartbeat++;
// Force RF off if required
	if((key_off_count >= KEY_OFF_TICKS) || isr_swr_trip || (alarms & ALARM_MASK) || (main_heartbeat >= MAIN_TIMEOUT))
		TX_ON = OFF;
	}

// Simulate encoder with UP / DOWN buttons
// Read encoder
int encoder_get(void)
	{
	int temp;

	temp = enc;
	enc = 0;
	return(temp);
	}

// Initialise timer & PWM system
void init_timers_pwm(void)
	{
	freq_sample_ctr = F_SAMPLES;
	freq_ctr = 100;
	OC3CON = 0;						// First turn off the module if it was on
	OC4CON = 0;

// LCD PWM DAC setup
	OC3RS = DEFAULT_BL;				// Set to 300
	OC3R = DEFAULT_BL;				// Set to 300
	OC3CON = 0x000E;				// Use TMR3, OC3 = PWM mode

	OC4RS = DEFAULT_CONTRAST;		// Set to 2000
	OC4R = DEFAULT_CONTRAST;		// Set to 2000
	OC4CON = 0x000E;				// Use TMR3, OC4 = PWM mode

//	PR3 = TICK_PERIOD - 1;			// 7372 Set PWM cycle, TMR3 period register, 1000Hz/1ms
	PR3 = TICK_PERIOD;				// Optimum value for frequency counter. (7378 giving a 1mS timer period = 1000.7053uS)
	T3CONbits.TON = 1;				// Start TMR3
	_T3IE = 1;						// Enable TMR3 tick IRQ

// Timer 1 for frequency counting
	_T1IE = 0;
	T1CON = 0x0002;					// External clock, no sync
	cmd_timeout = 0;
	}

// Tone generator, TMR2
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void)
	{
	switch(last_cycle)				// Tone generator state machine
		{
		case 0: default:			// Tone output off, tri-state
			TONE_TRIS = 1;			// Tri-state output
			_T2IE = 0;				// Disable TMR1 IRQs, Tone generator off
		break;

		case 1:						// Last half cycle
			TONE_OUT = !TONE_OUT;
			PR2 = PR2 >> 1;			// Next = half cycle
			last_cycle = 0;			// Flag for shut down
		break;

		case 2:						// Last full cycle, start shut down
			TONE_OUT = !TONE_OUT;	// Inverse tone out on every cycle

			if(TONE_OUT != 0)		// End with down going half cycle
				last_cycle = 1;		// Next = last full cycle
		break;

		case 3:						// Generate sound
			TONE_OUT = !TONE_OUT;	// Inverse tone out on every cycle
		break;
		}

	_T2IF = 0;
	}

// Tone set functions
void tone_on(int tone)				// Set tone, set value = 7,5MHz / tone(Hz) * 2
	{
	last_cycle = 3;					// Not last cycle, run state
	PR2 = tone - 1;					// Set tone period
	TMR2 = PR2 >> 1;				// Start with half period, reduce click
	TONE_OUT = 0;					// Start always same phase
	TONE_TRIS = 0;					// Enable output
	T2CONbits.TON = 1;				// Start timer
	_T2IF = 0;						// Clear old pending interrupt, start with clean half cycle
	_T2IE = 1;						// Enable TMR1 IRQs, Tone generator on
	}

void beep(int tone, int duration)
	{
	if(busy_counter == 0 && duration > 0)	// Start tone only if tone generator free & duration specified
		{
		tone_counter = duration;	// Set tone length
		tone_on(tone);				// Set tone pitch & start play
		}
	}
/*
 Frequency Counter
 The frequency counter relies on the 1mS timer interrupt being accurate. This interrupt is generated whenever
 the accumulated count in the TMR3 period register is equaled. At this point an interrupt is generated, the
 counter cleared and the count of the main clock cycles re-commences. The crystal oscillator used for the main
 clock uses a 7.3728MHz HC6/U crystal, giving a clock period of: 1/7.3728 = 135.6336806nS. In theory, in order
 to generate an interrupt every 1,000uS the period register would need to be set to:

	1000uS / .1356336806uS = 7372

 This would give an actual period of:

	7372 * 135.6336806 = 999.89149uS, or 999891.49nS

 If this is used for the gate time then there will be a constant error, as too few cycles would be counted.
 For example, for an input frequency of 28.85MHz and a gate time of 999891.49nS the accumulated count in the
 frequency counter would be: 28846 +/-1.

 The actual measured counts versus various register settings were:
 
  I/P FREQ			COUNT		COUNT		COUNT		COUNT
	kHz				7272		7373		7378		7379
 ============================================================
	1900			1897		1897		1899		1899
	3600			3596		3596		3599		3599
	7100			7092		7093		7099		7100
	10125			10114		10116		10124		10125
	14200			14185		14187		14199		14201
	18118			18100		18102		18117		18119
	21300			21278		21281		21299		21302
	24940			24915		24918		24939		24942
	28850			28821		28825		28849		28853
 ============================================================
 
 In addition, servicing the interrupt, stopping the count, count transfer, and re-starting the frequency counter
 also consume machine cycles which further shortens the effective gate time. For example, as the table above shows,
 with a timer counter register setting of 7372, a 28.850MHz input is actually counted as 28,821 events. The period
 of this frequency is:

	1 / 28.850 = 0.0346620451nS

 Since the count is 29 cycles short, this means the gate time was 1.0051993068uS short of 1,000uS or an
 effective time of:
 
	28821 * 0.0346620451 = 998994.800694nS

 The actual timer interrupt is: 999891.49nS, this means that the actual interrupt servicing, stopping,
 counter transfer and re-starting accounts for:

	999891.493059 - 998994.800694 = 896.6923650nS

 This suggests that between 6 or 7 machine cycles are required. Therefore, to account for this, the timer
 register setting needs to be increased by 6 or 7 to 7378 or 7379, and as the table above shows, the best
 choice is 7378.

 To ensure a stable display, the counts are averaged over 100 samples, and the resultant count then divided
 by 100. However, if this count were simply converted to a floating point number and then displayed with the
 correct decimal point, there is a constant -1 count error because the count represents a truncated count.
 To account for this, 1 is added to the count.

 In the Service Module, the calibration constant is nominally 1000,000. This is to ensure sufficient resolution
 to accurately account for the minor timing errors. This calibration factor is actually divided by 10 when
 the final calculation is made in the function get_freq() in the file juma-pa100.c

 This calibration factor is adjustable over the limits +/-250 which gives a frequency correction of about
 +/-72kHz at 30MHz and +/-5kHz at 1.9MHz. (See comments in get_freq() in juma-pa100.c)

 Frequency Sense Mode
 If the AUTO band detection is set to the Frequency Sense mode, then the input signal to the amplifier is
 shaped by a pair of diodes, and coupled to the input of the microprocessor. The gate time is 1mS determined
 by the 1mS timer interrupt. On entry to the interrupt the counter is stopped, and the accumulated count is
 transferred to the variable freq. Then the counter is cleared and re-enabled for the next 1mS. Since the gate
 is open for 1mS, the accumulated count represents kHz. For example, for an input signal of 1.8MHz, the counter
 will have accumulated 1,800 counts. The freq_sample_ctr is TRUE so the accumulated count in freq is
 compared with the values already stored in band_limits[]. The freq count is sequentially compared with the
 limit values pre-defined, and if it is less than the current limit, then the condition is met, and the loop
 terminated by a break instruction. The current loop counter index represents the band, and the count in
 the indexed value of the band_bins[] is incremented. The freq_sample counter is decremented and, if it is
 non-zero, the cycle repeats. If it is zero, this represents the end of the measurement cycle. In the main
 module, in the function: eval_band(), if freq_sample_ctr is 0, then we examine the various frequency count
 'bins'. band_bin[0] contains the count of the number of zero count samples. If it is less than 5 then we have
 accumulated enough samples to form a valid band. Each bin is examined from the highest frequency downwards,
 and the first bin to contain a count is considered to be the correct band. The variable eeprom.defval.band
 is assigned the band number, and then the bins are cleared, and freq_sample_ctr is set to the required number
 of counts, 20.
*/
// IRQ code
// 1ms Tick timer
void __attribute__((interrupt, auto_psv)) _T3Interrupt(void)
	{
	int i = 0;

	_T3IF = 0;
//	IRQ_TEST = 1;						// Timing test

// Frequency counter
	T1CONbits.TON = 0;					// Stop counting
	freq = TMR1;						// Save frequency
	TMR1 = 0;							// Reset counter
	T1CONbits.TON = 1;					// Start counter

// Although the 20 samples used by the Band Select module could be used to average the frequency counter readings,
// I found that this gave a somewhat 'glitchy' display. This longer averaging and subsequent display interval leads
// to a much more stable display. A.Ryan - 13/NOV/2014
	if(freq_ctr)
		{
		input_freq += (long)freq + 1L;	// See note above (Line 220)
		freq_ctr--;
		}
// Band select
	if(freq_sample_ctr)					// Run only when previous measurement has been used
		{
		do	{
			if(freq <= band_limits[i])
				{
				band_bins[i]++;			// Sample found, increment the bin count,
				break;					// and exit. (Under normal circumstances, i will never be greater than 9.)
				}						// If the frequency is higher than 30MHz, the break will not be taken, and i will terminate at 10.
			} while (++i < 10);

		if(i == 10)						// Frequency was higher than 30MHz, causing i to increment to 10,
			{
			pa_state = STANDBY;			// so, force state to Standby and set band to NOT_KNOWN. (This should never occur!)
			eeprom.defval.band = NOT_KNOWN;
			}

		freq_sample_ctr--;
		}
// Tone duration counter
	if(tone_counter)
		{
		tone_counter--;

		if(tone_counter == 0) last_cycle = 2;
		}
// Push button timer
	if(button_timer) button_timer--;		// Decrement button timer if active
// Power meter decay counter
	if(decay_counter) decay_counter--;
// Band data query timer
	if(polling_timer) polling_timer--;
// Elecraft KX-3/Yaesu Comms Timeout
	if(cmd_timeout) cmd_timeout--;
// Alarm blink timer
	if(loop_ctr) loop_ctr--;
// Remote Mode command timer
	if(rmt_timeout)
		{
		rmt_timeout--;
		if(!rmt_timeout) pa_state = STANDBY;// This permits local control until a remote message is received.
		}
/*
 In the PA100 we do not have rotary encoder, therefore the encoder functions are simulated, with UP & DOWN buttons
 As with all the buttons, a zero indicates that the button is pressed.
*/
// Do debounce shift registers
	if(!alarms)							// Ignore buttons if there are any alarms.
		{
		up <<= 1;						// UP button, encoder ++ (Shift the 16-bit debounce shift register left one place.)

		if(UP == 0) up |= 0x0001;		// A '0' indicates that the button is pressed, so set bit zero of the debounce shift register.
		else up &= 0xFFFE;				// otherwise, clear bit zero.

		dn <<= 1;						// DOWN button, encoder -- (Shift the 16-bit debounce shift register left one place.)

		if(DN == 0) dn |= 0x0001;		// Again, if the button is pressed, set bit zero of the 16-bit debounce shift register,
		else dn &= 0xFFFE;				// otherwise, clear bit zero.

// Test pushed condition
// UP
		if(up_pushed == 0)				// Not pushed
			{
			if(up == 0xFFFF)			// Test if debounce shift register is all ones, in other words, the button has been pressed without
				{						// there being a spurious release for 16 repeats of the loop.
				up_pushed = 1;			// Set pushed flag
				rep_up_dly = REP_DLY;	// Set repeat start timer
				enc++;					// Increment encoder
				}
			}
		else							// Pushed
			{
			if(up == 0x0000)			// Test if debounce shift register all zeros, in other words, the button has been released without
				{						// there being a spurious contact closure for 16 repeats of the loop.
				up_pushed = 0;			// Clear pushed flag
				rep_up_dly = 0;			// Clear repeat timer
				rep_up = 0;
				}
			}
// DOWN
		if(dn_pushed == 0)				// Not pushed
			{
			if(dn == 0xFFFF)			// Test if debounce shift register all ones
				{
				dn_pushed = 1;			// Set pushed
				rep_dn_dly = REP_DLY;	// Set repeat start timer
				enc--;					// Decrement encoder
				}
			}
		else							// Pushed
			{
			if(dn == 0x0000)			// Test if debounce shift register all zeros
				{
				dn_pushed = 0;			// Clear pushed flag
				rep_dn_dly = 0;			// Clear repeat timer
				rep_dn = 0;
				}
			}
// Repeat start timers
		if(rep_up_dly)					// Check if UP button start repeat timer is running
			{
			rep_up_dly--;

			if(rep_up_dly == 0)			// Start repeat action
				rep_up = rep_dly;
			}

		if(rep_dn_dly)					// Check if DOWN button start repeat timer is running
			{
			rep_dn_dly--;

			if(rep_dn_dly == 0)			// Start repeat action
				rep_dn = rep_dly;
			}
// Repeat generator timers
		if(rep_up)						// UP button repeat counter going ?
			{
			rep_up--;

			if(rep_up == 0)
				{
				rep_up = rep_dly;		// Reload timer
				enc++;					// Add encoder accumulator
				}
			}

		if(rep_dn)						// DOWN button repeat counter going ?
			{
			rep_dn--;

			if(rep_dn == 0)
				{
				rep_dn = rep_dly;		// Reload timer
				enc--;					// Subtract encoder accumulator
				}
			}
		}
// A-D conversions and TX protection. These are last, so that they do not disturb the frequency counter gate time.
	adc_tick();
	tx_guard();
//	IRQ_TEST = 0;						// Timing test
	}


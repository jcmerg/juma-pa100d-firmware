// JUMA-PA100 service & setup routines
// Juha Niinikoski, OH2NLT, 08.07.2008

// Display clean up 11.11.2008
// Factory setup function corrected 16.12.2008

#include <stdio.h>					// Added - 5B4AIY
#include <math.h>
#include "juma-pa100.h"
#include "pa100_eeprom.h"			// Get EEPROM structure definitions

// External References
// External Functions
extern void beep(int, int);						// Generate beep, period, length
extern void ms_delay(unsigned int);				// Delay
extern int encoder_get(void);					// Read encoder
extern void lcd_cmd(unsigned char);				// Send command to LCD
extern void lcd_putst(register const char *);	// Display character string on LCD
extern void disp_meter(double, int);			// Display meters - new version 27/JAN/2014
extern void disp_fwd_pwr(void);					// Display watt-meter
extern void disp_id(void);						// Display RF Amp total current
extern void check_alarms(void);					// Check alarms
extern void set_relays(void);					// Set gain & filter relays
extern void analog_measurements(void);			// Do analog measurements
extern void fan_control(void);					// Cooling fan control
extern int convert_adc12(unsigned int);			// Added - 5B4AIY
extern void save_settings(int, int);				// Added - 5B4AIY
extern void set_value(int, int *, int, int);	// Added - 5B4AIY
extern void get_one_zero(int *);
extern void eval_band(void);					// Frequency Sense function
extern double get_freq(void);					// Get the input frequency
extern void display_beeps(int);					// Page Increment/Decrement beep tone
extern void display_line(int, const char *);	// Display text string on specified line of LCD
extern void clear_buffer(void);					// Clear Juma PA-100D receive buffer

// External Data
extern volatile int pa_state;				// PA Standby/Operate State
extern int key;						// Copy of TX request input
extern int svc_flag;
extern int adjust_flag;
extern volatile int rep_dly;
extern volatile int enc;
extern volatile unsigned int alarms;			// Alarm bits
extern char lcdpbuff[];				// General purpose buffer for displays, etc
extern const char *on_off[];
extern const char *auto_man[];
extern const char *bs_txt[];
extern unsigned int AD_Values[];	// A-D converter samples
extern volatile unsigned int main_heartbeat;	// Main loop watchdog, see tx_guard() in timers_pwm.c
extern volatile int filter_mismatch;			// Input frequency above the selected filter, see tx_guard() in timers_pwm.c
extern volatile unsigned int relay_settle;		// Relay settling timer, see set_relays()

// Local Data
const char trip_fmt[] = {"Trip:%11s"};
const char trip_voltage[] = {"Limit:%9.2fV"};
const char cal_msg_fmt[] = {"Cal%7s:%5i"};
const char freq_cal_fmt[] = {"Freq:%7.3f MHz"};
const char factor[] = {"Factor"};
const char offset[] = {"Offset"};

const char *cal_prompt[] = {					// Indexed by: cal_page
							"Supply:  ",		// 0  - Voltmeter
							"",					// 1  - Ammeter (Place holder)
							"",					// 2  - RF Power (High Power - Place holder)
							"",					// 3  - RF Power (Low Power - Place holder)
							"Beep Len 0 = Off",	// 4  - Beep Length
							"Beep Tone       ",	// 5  - Beep Tone (JUMA/RS-928) - DL4JC
							"Power Averaging ",	// 6  - RF Power Measurement Averaging
							"Overvoltage     ",	// 7  - Over-voltage Trip On/Off
							"Overvoltage Trip",	// 8  - Over-voltage Trip Setting
							"Low Voltage     ",	// 9  - Under-voltage Trip On/Off
							"Low Voltage Trip",	// 10 - Under-voltage Trip Setting
							"Pre-Limit Trip  ",	// 11 - Low Voltage warning
							"Full-Scale Power",	// 12 - Graphic Power Meter Full-Scale Setting
							"",					// 13 - Frequency Meter calibration factor (Place holder)
							"Splash Screen   "	// 14 - Splash Screen  (On/Off)
							};

const char *svc_5_prompt[] = {					// Indexed by: 0/1 logic test in svc_5()
							"",					// 0
							"(Off)"				// 1
							};

// Standard Prompts & Messages
const char svc_4_Msg1[] = {"Beep:%9imS"};
const char svc_4_Msg2[] = {"Beep:%11s"};
const char svc_5_Msg[] = {"Samples:%6s%2i"};
const char beep_tone_msg[] = {"Tone:%11s"};
const char *beep_tone_txt[] = {"JUMA", "RS-928"};	// Indexed by: Beep_Tone
const char svc_11_Msg[] = {"Max Power:%5iW"};
const char svc_12_msg[] = {"Factor:%9ld"};
const char svc_13_msg[] = {"Display:%8s"};
const char Service_Calibration[] = {"\n\rService & Calibration Mode"};
const char Display_Config[] = {"\n\rDISPLAY/CONFIG: Select Page\n\rOPER: Save & Exit\n\r"};
const char Calibration_Saved[] = {"\n\rCalibration settings saved to EEPROM\n\r"};

int cal_page = 0;			// Starting Calibration Page
int old_lower_trip = 0;		// Used to take account of any changes to the Low-Voltage trip point.

// User Configuration Values
extern struct
	{
	struct defval defval;
	} eeprom;

// System Calibration Values
extern struct
	{
	struct calval calval;
	} cal;

// DL4JC Extension Values (Beep Tone)
extern struct
	{
	struct extval extval;
	} ext;

void set_alarm_flag(int on, int off)
	{
	int i = 0;

	if(adjust_flag) i = encoder_get();

	if(i > 0) Enabled_Alarms |= on;

	if(i < 0) Enabled_Alarms &= off;
	}

int set_range(long value, long cal_factor, unsigned long max, unsigned long min)
	{
	value *= cal_factor;

	if(value > max) value = max;
	if(value < min) value = min;

	return (int)(value / cal_factor);
	}

void svc_1_2_3_prompt(void)
	{
	lcd_putst(auto_man[Auto_Manual]);
	lcd_putst(bs_txt[Current_Band]);
	}

void get_fwd_and_rev_pwr(void)
	{
	reverse_pwr = convert_adc12(REV_PWR);
	forward_pwr = convert_adc12(FWD_PWR);
	}

void svc_0(void)			// Voltmeter Calibration
	{
	set_value(1, &Voltmeter_Cal, BATT_MULT_MAX, BATT_MULT_MIN);
	disp_meter(((double)convert_adc12(BATT_CH) * (double)Voltmeter_Cal), VOLTS);
	sprintf(lcdpbuff, cal_msg_fmt, factor, Voltmeter_Cal);		// Show scale multiplier factor
	}

void svc_1(void)			// Ammeter Calibration
	{
	amp_current = convert_adc12(ID_CUR);
	set_value(1, &Ammeter_Cal, ID_MULT_MAX, ID_MULT_MIN);
	svc_1_2_3_prompt();
	disp_id();
	sprintf(lcdpbuff, cal_msg_fmt, factor, Ammeter_Cal);		// Show scale multiplier factor
	}

void svc_2(void)			// RF Power Meter Calibration (High Power)
	{
	get_fwd_and_rev_pwr();
	set_value(1, &cal.calval.fwd_pwr_mult, HI_PWR_MULT_MAX, HI_PWR_MULT_MIN);
	svc_1_2_3_prompt();
	disp_fwd_pwr();
	sprintf(lcdpbuff, cal_msg_fmt, factor, cal.calval.fwd_pwr_mult);// Show scale multiplier factor
	}

void svc_3(void)			// RF Power Meter Calibration (Low Power)
	{
	get_fwd_and_rev_pwr();
	set_value(1, &cal.calval.lo_pwr_offset, LO_PWR_OFFSET_MAX, LO_PWR_OFFSET_MIN);
	svc_1_2_3_prompt();
	disp_fwd_pwr();
	sprintf(lcdpbuff, cal_msg_fmt, offset, cal.calval.lo_pwr_offset);// Show scale offset factor
	}

void svc_4(void)			// Beep Length Setting
	{
	set_value(1, &Beep_Time, 100, 0);

	if(Beep_Time) sprintf(lcdpbuff, svc_4_Msg1, Beep_Time);			// Show current setup
	else sprintf(lcdpbuff, svc_4_Msg2, on_off[OFF]);
	}

/*
 Beep tones for the buzzer of the RS-928 clone, which only sounds clean between about 2300 and 2800Hz, see beep() in
 timers_pwm.c. A changed setting is played at once. Stored in the extension block, which save_calval() also saves. DL4JC
*/
void svc_beep_tone(void)	// Beep Tone (JUMA/RS-928)
	{
	int t = Beep_Tone;

	get_one_zero(&Beep_Tone);

	if(Beep_Tone != t) beep(HZ466_85, 100);	// Sample of the selected tones

	sprintf(lcdpbuff, beep_tone_msg, beep_tone_txt[Beep_Tone]);
	}

void svc_5(void)			// Power Averaging Samples
	{
	int s;

	s = (int)cal.calval.samples;
	set_value(1, &s, SAMPLE_MAX, SAMPLE_MIN);
	cal.calval.samples = (unsigned char)s;
	sprintf(lcdpbuff, svc_5_Msg, svc_5_prompt[(cal.calval.samples == SAMPLE_MIN)], cal.calval.samples);		// Improved display - 5B4AIY
	}

void svc_6(void)			// Over-voltage Trip On/Off
	{
	set_alarm_flag(HI_V_ON, HI_V_OFF);
	sprintf(lcdpbuff, trip_fmt, (Enabled_Alarms & HI_V) ? on_off[ON] : on_off[OFF]);
	}

void svc_7(void)			// Over-voltage Trip Setting
	{
	if(adjust_flag) cal.calval.overvoltage_trip += encoder_get();

	cal.calval.overvoltage_trip = set_range((long)cal.calval.overvoltage_trip, (long)Voltmeter_Cal, (OVERVOLTAGE_UPPER * 10000UL), (OVERVOLTAGE_LOWER * 10000UL)); 
	sprintf((char *)lcdpbuff, trip_voltage, ((double)cal.calval.overvoltage_trip * (double)Voltmeter_Cal) / 1000000.0);
	}

void svc_8(void)			// Under-voltage Trip On/Off
	{
	set_alarm_flag(LO_V_ON, LO_V_OFF);
	sprintf(lcdpbuff, trip_fmt, (Enabled_Alarms & LO_V) ? on_off[ON] : on_off[OFF]);
	}

void svc_9(void)			// Under-voltage Trip Setting
	{
	if(adjust_flag) cal.calval.undervoltage_trip += encoder_get();

	cal.calval.undervoltage_trip = set_range((long)cal.calval.undervoltage_trip, (long)Voltmeter_Cal, (UNDERVOLTAGE_UPPER * 10000UL), (UNDERVOLTAGE_LOWER * 10000UL));
	sprintf((char *)lcdpbuff, trip_voltage, ((double)cal.calval.undervoltage_trip * (double)Voltmeter_Cal) / 1000000.0);
	}

void svc_10(void)			// Under-voltage Trip Pre-Limit
	{
	long upper_limit, lower_limit;

	if(old_lower_trip != cal.calval.undervoltage_trip)		// There has been a change to the Low-Voltage trip point,
		{
		cal.calval.pre_limit_trip += (cal.calval.undervoltage_trip - old_lower_trip);		// so take account of the change,
		old_lower_trip = cal.calval.undervoltage_trip;										// and cancel it.
		}

	upper_limit = ((long)cal.calval.undervoltage_trip * (long)Voltmeter_Cal) + 800000L;		// Upper Pre-Limit Trip at +800,000uV
	lower_limit = upper_limit - 700000L;													// Lower Pre-Limit Trip at +100,000uV

	if(adjust_flag) cal.calval.pre_limit_trip += encoder_get();

	cal.calval.pre_limit_trip = set_range((long)cal.calval.pre_limit_trip, (long)Voltmeter_Cal, upper_limit, lower_limit);
	sprintf((char *)lcdpbuff, trip_voltage, ((double)cal.calval.pre_limit_trip * (double)Voltmeter_Cal) / 1000000.0);
	}

void svc_11(void)			// Graphic Power Meter Maximum Scale Setting
	{
	if(adjust_flag) cal.calval.max_power += encoder_get();

	cal.calval.max_power = set_range((long)cal.calval.max_power, 1L, MAX_SCALE_POWER, MIN_SCALE_POWER);
	sprintf((char *)lcdpbuff, svc_11_Msg, cal.calval.max_power);
	}

void svc_12(void)			// Frequency Counter calibration
	{
	sprintf((char *)lcdpbuff, freq_cal_fmt, get_freq());
	lcd_putst(lcdpbuff);

	if(adjust_flag) cal.calval.freq_cal += (long)encoder_get();

	if(cal.calval.freq_cal > F_CAL_UPPER) cal.calval.freq_cal = F_CAL_UPPER;
	if(cal.calval.freq_cal < F_CAL_LOWER) cal.calval.freq_cal = F_CAL_LOWER;

	sprintf((char *)lcdpbuff, svc_12_msg, cal.calval.freq_cal);
	}

void svc_13(void)
	{
	int s;

	s = cal.calval.splash;
	get_one_zero(&s);
	cal.calval.splash = (unsigned char)s;
	sprintf((char *)lcdpbuff, svc_13_msg, on_off[s]);
	}

void (*set_svc[])(void) = {		// Indexed by: cal_page
						svc_0,	// 0	Voltmeter Calibration
						svc_1,	// 1	Ammeter Calibration 
						svc_2,	// 2	RF Power Meter Calibration (High Power)
						svc_3,	// 3	RF Power Meter Calibration (Low Power)
						svc_4,	// 4	Beep Length Setting
						svc_beep_tone,	// 5	Beep Tone (JUMA/RS-928) - DL4JC
						svc_5,	// 6	Power Averaging Samples
						svc_6,	// 7	Over-voltage Trip (On/Off)
						svc_7,	// 8	Over-voltage Trip Setting
						svc_8,	// 9	Under-voltage Trip (On/Off)
						svc_9,	// 10	Under-voltage Trip Setting
						svc_10,	// 11	Pre-Limit Trip Setting
						svc_11,	// 12	Graphic Power Meter Maximum Scale Setting
						svc_12,	// 13	Frequency Meter calibration factor
						svc_13	// 14	Splash Screen Display (On/Off)
						};

void display_svc_page(void)
	{
	display_line(LINE1, cal_prompt[cal_page]);		// Display current page's prompt on line 1.
	set_svc[cal_page]();							// Execute current calibration page's code and
	display_line(LINE2, lcdpbuff);					// display the returned value on line 2.

	if(cal_page != 10) old_lower_trip = cal.calval.undervoltage_trip;	// Take account of the current Low-Voltage trip point
	}

void change_cal_page(int direction)
	{
	cal_page += direction;

	if(!(Enabled_Alarms & HI_V) && (cal_page == 8)) cal_page += direction;	// If the High-Voltage alarm is disabled, then skip its page.

	if(!(Enabled_Alarms & LO_V) && (cal_page == 10 || cal_page == 11)) cal_page += (2 * direction);	// If Low-Voltage alarm is disabled, skip 2 pages

	if(cal_page > MAX_SERVICE_PAGES) cal_page = 0;

	if(cal_page < 0) cal_page = MAX_SERVICE_PAGES;

	rep_dly = (cal_page == 6) ? _SLOW : _FAST;
	display_beeps(cal_page);
	}

// Service & Calibration Functions
void service(int service_mode)
	{
	int rf_gain[11], i = 0;

	Power_Units = _WATT;				// Ensure power meter is set to Watts for calibration

	do	{								// Save current gain settings,
		rf_gain[i] = RF_Gain[i];
		RF_Gain[i] = 0;					// and set the gain to minimum.
		} while (++i < 11);

	do	{
// TX request
		main_heartbeat = 0;				// Service loop is running, see tx_guard() in timers_pwm.c
		key = KEY;						// Copy I/O bit to status flag

		if(!key) filter_mismatch = FALSE;	// Reset at the end of each transmission
// Do measurements & check alarms
		analog_measurements();
		check_alarms();

		if(alarms) service_mode = svc_flag = FALSE;	// Exit service mode (the loop tests service_mode, not svc_flag)
		else
			{
// Auto band select
			if(Auto_Manual == AUTO_BAND)				// AUTO Band Select,
				eval_band();							// so use the Frequency Sense mode.
			else if(Current_Band != TEN_METRES)
				Current_Band = TEN_METRES;				// MANUAL Band Select, then select 10m.
// Set relays
			set_relays();
// Evaluate TX possibility, needed for RF Power & Drain Current calibration
			if(key && (Current_Band != NOT_KNOWN) && (cal_page == 1 || cal_page == 2) && !relay_settle && !filter_mismatch)	// TX request
				{
				pa_state = OPERATE;
				TX_ON = ON;								// RF on
				}
			else
				{
				TX_ON = OFF;							// RF off
				pa_state = STANDBY;
				}
// Cooling Fan
			fan_control();
// Read Switches
			while(!DISP || PWR_SW)						// Change Calibration Page while either the DISP or PWR button is pressed
				{
				adjust_flag = FALSE;					// Used to prevent spurious parameter changes
				change_cal_page((PWR_SW) ? DECREMENT : INCREMENT);
				display_svc_page();
				ms_delay(PAGE_CHANGE);					// Also serves as button debounce
				enc = 0;								// Reset encoder
				adjust_flag = TRUE;
				}

			display_svc_page();							// Now display the service page and its calibration data

			if(!OPER)									// Test SW2 (OPER)
				{
				beep(HZ587_31, Beep_Time);				// D tone beep for OPER
				save_settings(0, 1);
				cal_page = 0;							// Reset page
				service_mode = FALSE;
				}	// End IF
			}
		} while(service_mode);

	i = 0;

	do	{
		RF_Gain[i] = rf_gain[i];						// Restore original gain settings
		} while (++i < 11);

	pa_state = STANDBY;									// Reset state to STANDBY
	}	// End function


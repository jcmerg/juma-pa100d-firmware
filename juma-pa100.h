// Board specific definitions for
// JUMA-PA100 RF amplifier
// Juha Niinikoski OH2NLT 22.11.2008

// Processor header
#include <p30f6014A.h>

// Version Data
#define VERSION				"v4.02a"
#define BUILD_NUMBER		"2-DL4JC"
#define BUILD_DATE			"04/OCT/2026"

// Macro Definitions
#define lcd_spc(count)		set_ch_bits(' ', count)		// Print spaces
#define set_pwm3_dac(pwm)	OC3RS = pwm					// LCD Backlight
#define set_pwm4_dac(pwm)	OC4RS = pwm					// LCD Contrast
#define clear_lcd()			lcd_cmd(CLEAR)
#define Start_Page			eeprom.defval.pa_state		// Shorthand for Start-Up Page Select
#define Temp_Scale			eeprom.defval.temp_units	// Shorthand for temperature scale display (C/F)
#define Alarm_Temp			eeprom.defval.temp_limit	// Shorthand for temperature alarm
#define Band_Select_Mode	eeprom.defval.bsel_mode		// Shorthand for band select mode
#define Current_Band		eeprom.defval.band			// Shorthand for current selected band
#define Auto_Manual			eeprom.defval.auto_band		// Shorthand for auto/manual band select
#define Poll_Time			eeprom.defval.poll_timer	// Shorthand for polling time
#define	Serial_Test_Mode	eeprom.defval.serial_test	// Shorthand for serial port mode setting (Off/Remote/Test)
#define SWR_Trip			eeprom.defval.swr_limit		// Shorthand for SWR trip limit
#define Fan_Speed			eeprom.defval.fan_control	// Shorthand for fan speed (Normal/Low/Medium/High)
#define Fan_Start			eeprom.defval.fan_start		// Shorthand for fan start temperature
#define Band_Units			eeprom.defval.band_units	// Shorthand for band display units (MHz/m)
#define Scale_Type			eeprom.defval.display_type	// Shorthand for graphic scale markers (Original/Large/Small)
#define Power_Units			eeprom.defval.rf_power		// Shorthand for RF power units (Watts/dBm)
#define	RF_Gain				eeprom.defval.rf_gain		// Shorthand for RF Amplifier gain setting
#define	Beep_Time			cal.calval.beep_len			// Shorthand for Beep length
#define	Voltmeter_Cal		cal.calval.batt_mult		// Shorthand for Voltmeter Calibration
#define	Ammeter_Cal			cal.calval.id_mult			// Shorthand for Ammeter Calibration
#define	Cal_Checksum		cal.calval.c_csum			// Shorthand for Calibration Checksum
#define	Cfg_Checksum		eeprom.defval.d_csum		// Shorthand for Configuration Checksum
#define Enabled_Alarms		cal.calval.alarm_flags		// Shorthand for Enabled Alarms mask

//Standard Constants
#define	MAX_SERVICE_PAGES	13
#define MAX_LCD_MODE		1				// Last display "main" page
#define USER_CONFIG_MODE	MAX_LCD_MODE
#define NORMAL_DISPLAY_MODE	0
#define	MAX_SUB_PAGE0		4				// Sub page 0 normal displays
#define	MAX_SUB_PAGE1		15				// Sub page 1 configuration displays
#define	DEFAULT_BAUD_RATE	3				// Default Baud rate = 9600
#define	MAX_BUFFER			16				// Elecraft Receiver Buffer Size
#define MSG_LEN				14				// Message length for FA data packet
#define INCREMENT			1				// Increment calibration/configuration page
#define DECREMENT			-1				// Decrement calibration/configuration page

// General Logic Status
#define	TRUE				1
#define	FALSE				0
#define ON					TRUE
#define OFF					FALSE
#define	DISABLED			FALSE
#define STANDBY				0
#define OPERATE				1
#define TRANSMIT			TRUE
#define RECEIVE				FALSE
#define MSG_START			'='
#define MSG_END				0x0D

// Start-Up Page Constants
#define DEFAULT_PWR			0
#define VSWR				1
#define VOLTAGE				2
#define CURRENT				3
#define TEMPERATURE			4

// Gain Constants
#define MAX_GAIN			3
#define MIN_GAIN			0

// Temperature Scale
#define FAHRENHEIT			0
#define CELSIUS				1

// UART setup according clock configuration
#define BAUD				9600
#define POLL_TIMER			2			// Yaesu CAT and KX2/KX3 protocol default query time, seconds
#define MAX_POLL_TIMER		10			// MAX = 10 seconds, 0 = Off

// Clock Constants
//#define FCY				7500000UL	// 30MHz ext Osc / 4  = 7,5MHz
#define FCY					7372800UL	// 29,4912MHz ext Osc / 4  = 7,3728MHz
#define CLK_FRQ				FCY / 1000L	// Clock frequency in kHz

#define DLYCONST			((FCY / 1000000UL) + 1)	// Constant for timer delay routines
/*
// PA100 band select limits(kHz) (Original Settings)
#define LM1					1500		// Band Number: 0 Invalid when under this limit
#define LM2					2650		// Band Number: 1 1.8 MHz filter band when under this limit
#define LM4					5250		// Band Number: 2 3.5 MHz filter band when under this limit
#define LM7					8500		// Band Number: 3 7.0 MHz filter band when under this limit
#define LM10				12000		// Band Number: 4 10 MHz filter band when under this limit
#define LM14				16000		// Band Number: 5 14 MHz filter band when under this limit
#define LM18				19500		// Band Number: 6 18 MHz filter band when under this limit
#define LM21				22500		// Band Number: 7 21 MHz filter band when under this limit (15-12-10m)
#define LM24				26000		// Band Number: 8 21 MHz filter band when under this limit
#define LM28				30000		// Band Number: 9 28 MHz filter band when under this limit
*/
// PA100 band select limits to coincide with standard JUMA-TRX2 settings
// Whenever the detected frequency is less than these limit values, then this band will be selected.
// Frequencies are rounded to the nearest kHz.
#define LM1					1500		// Band Number: 0	Invalid when under this limit
#define LM2					2001		// Band Number: 1	1.8 MHz		160m (Avoids a spurious band change at exactly the top end of 160m)
#define LM4					4001		// Band Number: 2	3.5 MHz		80m	(Avoids a spurious band change at exactly the top end of 80m)
#define LM7					8000		// Band Number: 3	7.0 MHz		40m
#define LM10				12000		// Band Number: 4	10.0 MHz	30m	
#define LM14				15000		// Band Number: 5	14.0 MHz	20m - 17m
#define LM18				19000		// Band Number: 6	18.0 MHz	20m - 17m
#define LM21				23000		// Band Number: 7	21.0 MHz	15m - 12m - 10m
#define LM24				26000		// Band Number: 8	24.0 MHz	15m - 12m - 10m
#define LM28				30001		// Band Number: 9	28.0 MHz	15m - 12m - 10m (Allows for the 10m band to be selected for 30MHz rather than UNKNOWN)

// Select algorithm parameters
#define F_SAMPLES			20			// Number of samples, 1ms/sample
#define	V_SAMPLES			5			// Required number of non-zero samples for decision

// Band Selector
#define MAX_FREQ			30000000L
#define MIN_FREQ			1500000L
#define MIN_BAND			1			// Band selector limits
#define MAX_BAND			9
#define TEN_METRES			MAX_BAND
#define NOT_KNOWN			10			// When Auto / Frequency unknown
#define OUT_OF_BAND			0			// Out-Of-Band indicator
#define YAESU				0
#define MAX_YAESU_CMD		5			// Maximum length of Yaesu binary command
#define ELECRAFT_KX3		1
#define JUMA_TRX2			2
#define FREQ_SENSE			3
#define FT_817				4
#define MANUAL				5
#define XIEGU				6			// Xiegu ACC port band voltage. Added after MANUAL so that saved settings remain valid.
#define MAX_BSEL_MODE		XIEGU		// Last band select mode
#define REMOTE				1			// Remote control operation
#define SERIAL_TEST			2			// Serial Test mode
#define MANUAL_BAND			0
#define AUTO_BAND			1
#define MHZ					0
#define METRES				1

// LCD parameters & factory defaults
#define DEFAULT_CONTRAST	2000
#define MAX_CONTRAST 		3000
#define MIN_CONTRAST		0
#define DEFAULT_BL			300
#define MAX_BL				1000
#define MIN_BL				50
#define ORIGINAL			0
#define LARGE				1
#define SMALL				2
#define RELAY_SETTLE		20			// mS, relay release/settling time for a band change, see set_relays()
#define FREQ_CAL			999985L		// See comments in timers_pwm.c
#define F_CAL_UPPER			FREQ_CAL + 360L
#define F_CAL_LOWER			FREQ_CAL - 360L

// 2 * 16 LCD display memory layout
#define LN1					0x00		// Address of Line 1
#define LN2					0x40		// Address of Line 2

// LCD Commands
#define CURSET				0x80		// Set memory cursor
#define	CLEAR				0x01		// Clear display
#define ADRSET				0x40		// Set CGRAM address
#define S_BAR				0x06		// Graphic meter End-Of-Scale symbol
#define LINE1				LN1 | CURSET
#define LINE2				LN2 | CURSET
#define LINE2PLUS1			(LN2 + 1) | CURSET
#define LINE1PLUS4			(LN1 + 4) | CURSET
#define LINE2PLUS4			(LN2 + 4) | CURSET
#define LINE2PLUS10			(LN2 + 10) | CURSET

// Meter Constants
#define DISP_LEN			8			// Display length (char)
#define FONT_W 				6			// Font width
#define MAX_GRAPH			(DISP_LEN * FONT_W)	// Graph length in pixels (6 * 8 = 48)
#define FULL_BLOCK			0x05
#define _WATT				0			// Used to select either Watt or dBm for power display
#define _DBM				1			// Used to select either Watt or dBm for power display
#define WATTS				2
#define SWR					3
#define VOLTS				4
#define AMPS				5
#define DBM					6

// Panel buttons
#define BUTTON_DEBOUNCE 	200			// Debounce delay(ms) "original"
//#define BUTTON_DEBOUNCE		100			// Debounce delay(ms) for fast fingers(oh7sv)
#define MEDIUM_PUSH			500			// Medium button push for emergency off
#define PAGE_CHANGE			650			// Time interval between auto-page increments
#define LONG_PUSH			650			// Long button push
#define VERY_LONG_PUSH		1200		// Very long push
#define POWER_OFF 			1200		// Power-off push
#define DEFAULT_BEEP_LEN	50			// Button beep length 50ms
#define BLINK_RATE 			500			// Alarm text blink rate (ms)
#define ALARM_BEEP	 		30			// Alarm beep length
#define LOOP_COUNT			400			// Alarm blink timer - 400mS
#define SHORT_BEEP			50
#define LONG_BEEP			350

// Encoder simulator with UP and DOWN buttons
#define _FAST				100
#define _SLOW 				650			// UP / DOWN buttons repeat interval
#define REP_DLY 			650			// Delay before repeat starts
/*
 The SPEED_ARRAY is really a sequence of bits whose position relates to the page in the User Configuration
 menu, thus:
	Page	15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
	Speed	 S  S  S  S  S  F  F  S  S  F  F  F  S  S  S  S  S = Slow, 0, F = Fast, 1
 Representing this as a HEX number, 0x0670
 By masking this array with a '1' bit shifted left into the page position by sub_page1 gives a TRUE/FALSE
 result, which is then used to return either the _FAST or the _SLOW timer count. Thus the expression is:
 repeat speed = (SPEED_ARRAY & (1 << sub_page1)) ? _FAST : _SLOW
*/
#define SPEED_ARRAY			0x0670

/*
 ALARM FLAGS & SETTINGS
 The alarm bits are mapped:
 D0	High SWR
 D1	Over-Current
 D2	High Temperature
 D3	Over-Voltage
 D4	Low-Voltage (Pre-Limit)
 D5	Low-Voltage (Final Limit)
 D6 - D15 Not Used
 Example: SWR alarm is asserted then bit 0 is set.
 To set the alarms bit, alarms |= SWR_AL;
 To clear the SWR alarm, alarms &= SWR_AL_OFF;

 For alarms that could be masked, for example, the High Voltage alarm, then the alarms word is compared with the alarms mask, Enabled_Alarms
 To set the alarms bit, alarms |= HI_V;
 To test whether the alarm is enabled,
 if(alarms & Enabled_Alarms) - If the alarm is enabled, and set then this will be TRUE.
 Initially, the alarm mask variable in the EEPROM, cal.calval.alarm_flags, otherwise known as Enabled_Alarms, sets all the valid bits
 on except for the battery pre-limit, as this is only a warning. This mask is then modified by setting or clearing the bits associated
 with the maskable alarms. For example, suppose the High Voltage alarm is disabled, then, Enabled_Alarms is AND'ed with HI_V_OFF,
 which is 0b00110111. Here, bit 3, the High Voltage alarm bit, is zero, and thus, in check_alarms() the final alarms setting is
 determined by alarms &= Enabled_Alarms. The alarm mask must be such that the non-maskable alarms are always enabled.
*/
#define SWR_AL				0b00000001
#define SWR_AL_OFF			0b00111110
#define CURR_AL				0b00000010
#define CURR_AL_OFF			0b00111101
#define TEMP_AL				0b00000100
#define TEMP_AL_OFF			0b00111011
#define HI_V				0b00001000
#define HI_V_AL_OFF			0b00110111
#define HI_V_ON				0b00001111	// To turn the alarm mask on, you OR it. This setting ensures that SWR, OC, TEMP, and HI-V are all on.
#define HI_V_OFF			0b00110111	// To disable, you AND it with the existing mask
#define LO_BATT				0b00010000	// Pre-limit alarm
#define LO_BATT_AL_OFF		0b00101111
#define LO_V				0b00100000	// Final limit alarm
#define LO_V_AL_OFF			0b00011111
#define LO_V_ON				0b00110111	// Make sure we do not disturb the HI-V alarm flag
#define LO_V_OFF			0b00001111	// We turn off both the pre-limit and the final limit alarm bits
#define ALARM_MASK			0b00101111	// Mask the Low Battery pre-limit alarm, it is only a warning.
#define ALL_ALARMS_ON		(SWR_AL | CURR_AL | TEMP_AL | HI_V | LO_BATT | LO_V)

// Alarm trip limits
#define TEMP_LIMITC			70			// Degrees C / Display units
#define TEMP_LIMITF			158			// Degrees F / Display units
#define MIN_TEMPC			50			// Adjustment limits, C
#define MIN_TEMPF			120			// Adjustment limits, F
#define MAX_TEMPC			100
#define MAX_TEMPF			212
#define OVERVOLTAGE_UPPER	1500		// Maximum over-voltage trip = 15.01V
#define OVERVOLTAGE_LOWER	1400		// Minimum over-voltage trip = 14.01V
#define OVERVOLTAGE_TRIP	2762		// Nominal over-voltage trip = 14.5V
#define UNDERVOLTAGE_UPPER	1150		// Maximum under-voltage trip = 11.5V
#define UNDERVOLTAGE_LOWER	1050		// Minimum under-voltage trip = 10.5V
#define UNDERVOLTAGE_TRIP	2096		// Nominal under-voltage trip = 11.0V
#define PRELIMIT_TRIP		2134		// Nominal pre-limit trip = 11.2V
#define SWR_LIMIT			300			// Default value 3.00:1, 500 = 5.0:1
#define MAX_RO				0.816514	// See comments in juma-pa100.c calc_swr() function
#define MAX_SWR				900			// Max setup value - Changed to 9:1
#define MIN_SWR				100			// Min setup value
#define MAX_SCALE_POWER		160			// Maximum graphical power meter full-scale setting
#define MIN_SCALE_POWER		40			// Minimum graphical power meter full-scale setting

// Power meter
// Fast attack, slow release parameters
#define PEAK_SHOW_TIME		1000		// Freeze after new peak value(ms)
#define PWR_MTR_DEAD_BAND	300			// Approximately 1W

// ADC values
// Channel definitions
#define ID_CUR				9			// PA Drain current channel
#define BATT_CH				10			// Battery channel
#define B_TYPE				11			// Board type
#define Y_817				11			// Yaesu 817 band data
#define REV_PWR				12			// Reverse power channel
#define FWD_PWR				13			// Forward power
#define TEMP				14			// PA Temperature
#define amp_current			AD_Values[0]
#define batt_raw			AD_Values[1]
#define YAESU_FT_817		AD_Values[2]
#define reverse_pwr			AD_Values[3]
#define forward_pwr			AD_Values[4]
#define hs_temp				AD_Values[5]

// Board type limits
#define B1					819			// 1V
#define B2					1638		// 2V, F-sense board type = 1...2V
#define B3					2457		// 3V
#define B4					3276		// 4V
#define B5					4095		// 5V

// Tone generator constants, FCY / tone * 2 Both frequencies in Hz
// Some harmony sounds FCY = 7,3728MHz
#define HZ392_01			4702		// G, off
#define HZ466_85			3948		// B-flat, push button standard tone
#define HZ587_31			6138		// D, fast tune tone
#define HZ698_45			2639		// F, CW side tone
#define	HZ4000				921			// 4kHz

// Meter scaling factors, factory defaults
/*
  The battery voltage is sensed across R28 (10K) in series with R27 (33K). For a nominal 13.80V input this gives
  a voltage into the A-D converter of: 13.80 * 10 / (10 + 33) = 3.2093V
  This gives a raw converter value of: 3.2093 * 4096 / 5.0 = 2629
  The necessary calibration multiplier is therefore: 13800000 / 2629 = 5249.144
  This is rounded up to 5250 just to give a 'nice' presentation.
*/
#define BATT_MULT 			5250		// Battery voltage (uV) = ADC * BATT_MULT / 1000000
#define BATT_MULT_MAX		5750
#define BATT_MULT_MIN		4750
/*
  Nominal sense voltage for the current readings is 100mV/A. Thus, for a 25A current, the sense voltage is 2.5V
  and using the +5V logic supply as the reference gives an A-D output of 2048. The default scaling factor is
  therefore 2442. Measurements made on a number of amplifiers suggests that the actual sense voltage is somewhat
  lower than this, hence the current default value of 2520.
*/
#define ID_MULT 			2520		// Default value for Drain current scaling, ID = ADC * ID_MULT / 200000
#define ID_MULT_MIN			1500
#define ID_MULT_MAX			4000 
/*
  The nominal forward output voltage of the SWR bridge is 3.6V @ 100W. This is digitised by the A-D converter as: 4096 * 3.6 / 5.0 = 2950
  Consequently, the default value for the forward power multiplier = 10,000,000,000 / ((2950 + 120) * (2950 + 120)) = 1061.02 i.e, 1062,
  taking into account the default low power compensation offset.

  The SCALE_CONST is the constant factor to scale the maximum value of the graphical bar graph when displaying power.
  The maximum value for the graphical scale for the bar graph is derived from the following:
  Since the bar graph is intended to display power, and power is proportional to the voltage squared, the A-D value must be squared to
  correctly represent power. The 'pseudo resistance' across which the voltage of 3.6V @ 100W is developed is
  R = V * V / P, 3.6 * 3.6 / 100 = 0.1296 Ohms.
  In order to determine the voltage for other maximum power levels, V = sqrt( 0.1296 * Power )
  To determine the A-D output this represents: Count = 4096 * V / 5
  The reference voltage for the A-D converter is the +5V logic supply.
  The measured power is: (A-D + Offset) * (A-D + Offset) = Mp
  To find the fraction of the bar graph used: Mp / Count
  To determine the number of bars, the maximum number is 48, Bars = 48 * Mp / Count
  Putting this altogether, first notice that the square root is not really required, since the count needs to be squared anyway, thus
  Count = 4096 * 4096 * 0.1296 * Power / ( 5 * 5)
  This reduces to 86973.08774 * Power

  Notice also that what we really want is the quantised number of blocks, so this further reduces to:

  86973.08774 * Power / 48 = 1811.9393 * Power, or using integers, 1812 * Power

  But we also have to take into account the forward power calibration value.
  The nominal value for the multiplier as derived above is 1061. If the actual SWR bridge is somewhat more sensitive, then its output
  voltage will be a little higher than 3.6V @ 100W, and thus the calibration factor will be lower than 1061. Conversely, if it is
  slightly less sensitive, then it will need to be higher. To take this into account so that the maximum scale factor is corrected
  for the different calibration settings, we need to correct the expression by the ratio of the normal calibration factor to the
  actual factor in the inverse sense, since we are going to divide the forward power which is an A-D value squared by the scale factor
  to give us the number of block to display. Thus the scale factor in the main program is calculated as:

  pwr_scale_factor = (SCALE_CONST * cal.calval.max_power / cal.calval.fwd_pwr_mult);

  and SCALE_CONST = 1812 * FWD_PWR_MULT, where FWD_PWR_MULT is the nominal calibration multiplier factor.
  */
// Default value for Forward & Reverse Power scaling, P = ((ADC + LO_PWR_OFFSET) * (ADC + LO_PWR_OFFSET)) * FWD_PWR_MULT / 100000000
#define	FWD_PWR_MULT 		1062UL
#define SCALE_CONST			1812 * FWD_PWR_MULT
#define LO_PWR_OFFSET		120
#define HI_PWR_MULT_MAX		1500
#define HI_PWR_MULT_MIN		750
#define LO_PWR_OFFSET_MAX	150
#define LO_PWR_OFFSET_MIN	0
#define LOW_PWR_LIMIT		39810710	// Lower limit for power meter. 400mW
/*
 The temperature sensor is a transistor, and its base-emitter voltage will vary by -2mV/K. The output voltage is fed to the inverting
 input of an op-amp with a gain of -10. The non-inverting input is fed with an adjustable DC voltage to offset the standing DC
 voltage from the base-emitter junction, and thus the output voltage of the op-amp will represent temperature in the range 0C - 100C
 with a voltage of 0V - 2.0V. The A-D converter is a 12-bit device, with a reference voltage of +5.0V and having a maximum count of 4095,
 thus with a +2.0V input representing 100C the count will be 2 / 5 * 4096 = 1,638. Thus, for this to represent 100C, the displayed value
 has to be ADC / 16.38. For the Fahrenheit scale, the range is from 32F to 212F, or a range of 180 degrees. For the same temperature to
 show as 212 in this scale, the scale factor is 1,638 / (212 - 32) = 9.1. In this case the displayed value is (1,638 / 9.1) + 32. 
*/
#define T_MULTC				16.38		// Degrees C temperature scaling. Tdisplay = ADC / T_MULTC
#define T_MULTF				9.1			// Degrees F temperature scaling. Tdisplay = (ADC / T_MULTF) + 32

// Fan Operating Mode
#define NORMAL_SPEED		0
#define LOW_SPEED			1
#define MEDIUM_SPEED		2
#define HIGH_SPEED			3

// Fan speed table, deg C/deg F
#define FAN_STARTC			40			// Fan Speed 1, Start temp, Low, C (Low)
#define FAN_STARTF			104			// Fan Speed 1, Start temp, Low, F (Low)
#define PLUS_5C				5			// Fan Speed 2, Low + 5 = 45C (Medium)
#define PLUS_10F			10			// Fan Speed 2, Low + 10 = 114F (Medium)
#define PLUS_10C			10			// Fan Speed 3, Low + 10 = 50C (High)
#define PLUS_20F			20			// Fan Speed 3, Low + 20 = 124F (High)
#define MAX_FAN_STARTC		100			// Adjustment limit, C
#define MAX_FAN_STARTF		212			// Adjustment limit, F

// Sample average
#define SAMPLE_MIN			1
#define SAMPLE_MAX			16			// Maximum number of power samples

// Power-on & calibration value addresses in the EEPROM
#define EEPAGE 				0x7F		// EEPROM address high part
#define EEDEF 				0xF000		// EEPROM default values storage area
#define EECAL 				0xF040		// EEPROM calibration values storage area
#define EE_FD_LOC 			0xF0F0		// Factory Default Reset Counter address in EEPROM

// PA100 board I/O definitions
// Port A Switches
#define INIT_TRISA 			0xFF3F		// PortA switch inputs
#define INIT_PORTA 			0x0000

#define AUTO				_RA12		// SW2, AUTO
#define DISP				_RA13		// SW1, FUNC / DISP
#define DN					_RA14		// SW8, DOWN button
#define UP 					_RA15		// SW5, UP button

// Port B ADC inputs
//#define INIT_TRISB 		0x7E0B		// PortB ADC and IDC pins = inputs
#define INIT_TRISB 			0x7E08
#define INIT_PORTB 			0x0000

#define IRQ_TEST			_LATB0		// Timing test, J19-4
#define MAIN_TEST			_LATB1		// Timing test, J19-5

#define TX_ON				_LATB4		// 1 = PA active, RF power on
#define KEY					_RB3		// 1 = TX Request

#define OC_CLR				_LATB5		// Over-current clear, 0 = clear

#define DB_4				_LATB6		// RF attenuators
#define DB_2				_LATB7

#define PTT_IN_IO			_RB3		// PTT input / TX on, 0 = active, Test

// Port C Switches & power control
#define INIT_TRISC			0x401E
#define INIT_PORTC			0x0000

#define OPER				_RC4		// SW3, OPER
#define BAND_DN				_RC3		// SW4, Band- button
#define BAND_UP				_RC1		// SW6, Band+ button

#define F_SENSE				_RC14		// F-Sense counter input
#define PWR_ON				_RC13		// Power-on

// Port D LCD & PWM out
#define INIT_TRISD			0x0001		// LCD & LED I/O, all outputs
#define INIT_PORTD			0x0000

// Power switch
#define PWR_SW				_RD0		// Power switch input, 1 = switch pushed

// Tone output
#define TONE_OUT			_LATD1		// Tone output
#define TONE_TRIS			_TRISD1		// Tone output tri-state control

// LCD signals
#define LCD_E				_LATD5		// LCD E signal

#define LCD_RW				_LATD6		// LCD R/W signal
#define LCD_RS				_LATD7		// LCD RS signal

#define LCD_DATA			LATD		// LCD 8-bit data bus out register
#define LCD_BUS				PORTD		// LCD 8-bit data bus state
#define LCD_TRIS			TRISD		// LCD data direction 

#define LCD_SHIFT			8			// LCD bits shifted from D0
#define LCD_MASK			0xFF00		// Used to extract LCD data at upper byte of 16-bit I/O port
#define LCD_BUSY_FLAG		0x8000		// Used to extract LCD Busy Flag from upper byte of 16-bit I/O port data

// PORT F RS-232
#define INIT_TRISF			0xFC7C
#define INIT_PORTF			0x0000

// PORT G FAN control
#define INIT_TRISG 			0x8000		// RG15 input, others outputs
#define INIT_PORTG 			0x0000

#define OC					_RG15		// Over-Current trip indicator from PA board

#define FAN1				_LATG2		// FAN High speed
#define FAN2				_LATG3		// FAN Low speed

#define TUNER				_LATG1		// Not used.

// Filter select I/O
#define M1_8				_LATG13		// 1.8 MHz
#define M3_5				_LATG12		// 3.5 MHz
#define M7					_LATG14		// 7 MHz
#define M10					_LATA7		// 10 MHz
#define M14_18				_LATA6		// 14-18 MHz
#define M21_28				_LATG0		// 21-28 MHz filter select


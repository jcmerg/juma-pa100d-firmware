//
// Board specifc definitions for
// JUMA-TRX2 transceiver
// Juha Niinikoski OH2NLT 22.11.2006
//
// MPLAB C30 version
// 
//

// processor header
#include <p30f6014A.h>

// uart setup according clock config
#define BAUD 9600
#define SERIAL_MODE 3			// test interface

// clock constants

#define FCY  7500000UL			// 30MHz ext Osc / 4  = 7,5MHz

#define DLYCONST ((FCY/1000000UL) + 1)	// constant for timer delay routines


// TRX-2 common definitions, used by various modules

// DDS constants
#define REF_OSC 180000000		// 180,000 MHz ref osc nominal frequency
#define MAX_DDS 1908874354		// upper limit 80MHz, Nyquist frequency = 90MHz
#define DEFAULT_FREQ_A 176523156	// "factory default" VFO A 3.699 MHz
#define DEFAULT_FREQ_B 336916371	// "factory default" VFO B 7.060 MHz
#define MIN_DDS 0				// low limit
#define M7_FILT 190887435		// 3,5 / 7MHz filter switch at 4MHz
#define CW_SHIFT -33405			// 700 Hz down when CW TX

// RIT addjustment
#define RIT_SCALE 24L			// rit pot scaling factor
#define RIT_DEAD_ZONE 50		// rit pot center dead zone, A/D steps

// CW keyer
#define KEYER 1					// factory default = Iambic

// LCD parameters & factory defaults
#define DEFAULT_CONTRAST 1200
#define MAX_CONTRAST 3500
#define DEFAULT_BL 300
#define MAX_BL 1400

// panel buttons
#define BUTTON_DEBOUNCE 200		// debounce delay(ms)
#define LONG_PUSH 1000			// long button push
#define VERY_LONG_PUSH	3000	// very long push
#define POWER_OFF 1500			// power off push
#define DEFAULT_BEEP_LEN 50;	// button beep length 50ms, long = * 10

// ADC values

// channell definitions
#define ID_CUR 9				// PA Drain current chanell
#define BATT_CH 10				// battery chanell
#define S_METER	11				// S-meter
#define REV_PWR 12				// reverse power chanell
#define FWD_PWR 13				// forward power
#define CW_POT 14				// CW speed pot, R24
#define RIT_POT 15				// RIT adjust pot, R9


// Tone generator constants, 7,5MHz / tone(Hz) * 2
// some fixed tones
#define HZ350	10714
#define HZ400	9375
#define HZ500	7500
#define HZ700	5357			// 700 Hz
#define HZ1000	3750
#define HZ1500	2500
#define HZ2000	1875

// some harmony sounds
#define HZ392_01	9566		// G, off
#define HZ466_85	8044		// Bp, push button standard tone
#define HZ587_31	6385		// D, fast tune tone
#define HZ698_45	5369		// F, CW side tone

// meter scaling factors, factory defaults
#define BATT_MULT 132UL			// 100 x battery voltage ADC * BATT_MULT / 256 
#define ID_MULT 6613UL			// default value for Drain current scaling, ID*100 = ADC * ID_MULT / 65536 
#define	S_MTR_MULT 2134UL		// default value for S-meter scaling, S-meter = ADC * S_MTR_MULT / 65536, 48 = max meter 
#define	REV_PWR_MULT 23UL		// default value for Reverse Power scaling
#define	FWD_PWR_MULT 23UL		// default value for Forward Power scaling,P*100 =  ADC^2 * FWD_PWR_MULT / 65536, 10W = 1000
#define PWR_GRAPH_MULT 18UL		// power bar graph,48=max = 10W, ADC^2 * PWR_GRAPH_MULT / 1048576
#define F_VOLT_GRAPH_MULT 1874UL // power bar graph,48=max = 10W, ADC * F_VOLT_GRAPH_MULT / 65536

// alarm limits
#define SWR_ALARM 300			// SWR alarm 3.00
#define SWR_AL_DLY 200			// PTT to alarm active delay about 400ms @ 2ms main loop

// SCAF filter clock
// divider = 7500000 / Filt(Hz)*100
#define DEFAULT_NAR	75			// 1kHz
//#define DEFAULT_MID	37			// 2,0kHz
#define DEFAULT_MID	34			// 2,2kHz
//#define DEFAULT_WID	28			// 2,7kHz
#define DEFAULT_WID	29			// 2,6kHz
#define DEFAULT_TX	28			// 2,7kHz

#define MIN_SCAF	18			// set value low limit, 4000Hz
#define MID_SCAF	33			// set value wide-mid limit, 2300Hz
#define NAR_SCAF	58			// set value mid-nar limit, 1300Hz
#define MAX_SCAF	111			// set value high limit, 675Hz

// Power on & calibration value addresses in the EEPROM
#define EEPAGE 0x7F				// EEPROM address high part
#define EEDEF 0xF000			// EEPROM default values storage area
#define EECAL 0xF040			// EEPROM calibration values storage area
#define EE_FD_LOC 0xF0F0		// counter address in EEPROM


// TRX2 board I/O definitions

// Port A Switches

#define INIT_TRISA 0xFF3F				// PortA switch inputs
#define INIT_PORTA 0x0000

#define SW2	_RA12						// SET
#define SW1	_RA13						// FUNC
#define ENQ_B 	_RA14					// rotary encoder, phase B / INT 3
#define ENQ_A 	_RA15					// rotary encoder, phase A / INT 4


// Port B ADC inputs

#define INIT_TRISB 0xFE73				// PortB ADC and IDC pins = inputs
#define INIT_PORTB 0x0000

#define PTT_IN	_RB6					// PTT input / TX on, 0 = active
#define PTT_OUT	_LATB7					// PTT output, 1 = active

#define DOT		_RB5					// paddle DOT input, 0 = active
#define DASH	_RB4					// paddle DASH input, 0 = active
#define KEY		_LATB3					// key output, 1 = active

// Port C Switches & power control

#define INIT_TRISC 0x401E				//
#define INIT_PORTC 0x0000				//

#define SW3		_RC4					// RIT
#define SW4		_RC3					// VFO
#define SW5		_RC2					// MODE
#define SW6		_RC1					// FILTER

#define PWR_SW	_RC14					// Power switch input
#define PWR_ON	_RC13					// Power on


// Port D LCD & PWM out

#define INIT_TRISD 0x0000			// LCD & LED I/O, all outputs
#define INIT_PORTD 0x0000			//

// tone output
#define TONE_OUT	_LATD1			// tone output
#define TONE_TRIS	_TRISD1			// tone output tri state control

// LCD signals
#define LCD_E		_LATD5			// LCD E signal

#define LCD_RW		_LATD6			// LCD R/W signal
#define LCD_RS		_LATD7			// LCD RS signal

#define LCD_DATA	LATD			// LCD 8-bit data bus out register
#define LCD_BUS		PORTD			// LCD 8-bit data bus state
#define LCD_TRIS	TRISD			// LCD data direction 

#define LCD_SHIFT 8					// LCD bits shifted from D0
#define LCD_MASK	(0x00FF<<LCD_SHIFT)			// LCD data lines at port


// PORT F & G 
// RS232, DDS, SPI and Codec/AUX signals

#define INIT_TRISF 0xFC7C			//
#define INIT_PORTF 0x0000

//#define INIT_TRISG 0xD080
#define INIT_TRISG 0x0080
#define INIT_PORTG 0x0000

#define DDS_SER		_LATF0			// AD9850 DDS D7/SER
#define DDS_RESET	_LATG0			// AD9850 DDS Reset
#define DDS_FQ_UD	_LATG1			// AD9850 DDS FQ_UD
#define DDS_W_CLK	_LATF1			// AD9850 DDS W_CLK	

#define TEST		_LATG15			// test signal	
#define IRQ_TEST	_LATG14			// test signal


// 74HC595 Main board shift register bits
// shift register is loaded with SPI1 functions, MSB goes first

#define SSB_CW		(1 << 0)		// SSB / !CW select
#define SB_SELECT	(1 << 1)		// sideband select, 0 = ??, 1 = ??
#define NAR			(1 << 2)		// Narrow BPF filter, 1 = Narrow
#define FAST_AGC	(1 << 3)		// Fast AGC select, 1 = Fast
#define PROC_ON		(1 << 4)		// TX speech processor on, 1 = on
#define NB			(1 << 5)		// Noice blacer on, tbd
#define	SPARE		(1 << 6)
#define MHZ7		(1 << 7)		// 3,5 / 7 MHz filter change

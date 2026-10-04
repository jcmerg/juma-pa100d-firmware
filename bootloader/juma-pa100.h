//
// Board specifc definitions for
// JUMA-TRX2 transceiver
// Juha Niinikoski OH2NLT 22.11.2006
//
// MPLAB C30 version
// 
// S-meter default calibration changed for Rev.B board. 2.0V = full scale. 17.03.2007
// Voice memory Rev.B board def changes 21.03.2007
// Button switch debounce time shortened 03.04.2007
// All Band PA filter select 15.04.2007
// PTT_IN changed to PTT_IN_IO for main logic change 02.06.2007

// processor header
#include <p30f6014A.h>

// uart setup according clock config
#define BAUD 9600
#define SERIAL_MODE 3			// test interface
//#define SERIAL_MODE 1			// Yaesu CAT emulation


// clock constants

//#define FCY  7500000UL			// 30MHz ext Osc / 4  = 7,5MHz
#define FCY    7372800UL			// 29,4912MHz ext Osc / 4  = 7,3728MHz

#define DLYCONST ((FCY/1000000UL) + 1)	// constant for timer delay routines


// PA100 filter select limits(kHz)

#define LM1		1000			// Not valid when  under this limit

#define LM2		3000			// 1.8 MHz filter band when under this limit
#define LM4		6000			// 3.5 MHz filter band when under this limit
#define LM7		9000			// 7 MHz filter band when under this limit
#define LM10		13000			// 10 MHz filter band when under this limit
#define LM14		16000			// 14 MHz filter band when under this limit
#define LM18		20000			// 18 MHz filter band when under this limit
#define LM21		23000			// 21 MHz filter band when under this limit
#define LM24		26000			// 21 MHz filter band when under this limit
#define LM28		30000			// 28 MHz filter band when under this limit


// Band selector
#define MIN_BAND 1					// band selector limits
#define MAX_BAND 9					//
#define NOT_KNOWN 10					// When Auto / frequency not known 

// LCD parameters & factory defaults
#define DEFAULT_CONTRAST 2200
#define MAX_CONTRAST 3500
#define DEFAULT_BL 100
#define MAX_BL 1100

// panel buttons
//#define BUTTON_DEBOUNCE 200		// debounce delay(ms) "original"
#define BUTTON_DEBOUNCE 100			// debounce delay(ms) for fast fingers(oh7sv)
#define LONG_PUSH 1000				// long button push
#define VERY_LONG_PUSH	3000		// very long push
#define POWER_OFF 1200				// power off push
#define DEFAULT_BEEP_LEN 50			// button beep length 50ms, long = * 10
#define BLINK_RATE 500				// alarm text blink rate (ms)

// alarm flags
// Alarm bits
#define SWR_AL 	(1<<1)
#define CURR_AL	(1<<2)
#define TEMP_AL	(1<<3)

// ADC values
// channell definitions
#define ID_CUR 9				// PA Drain current chanell
#define BATT_CH 10				// battery chanell
#define B_TYPE	11				// Board type
#define REV_PWR 12				// reverse power chanell
#define FWD_PWR 13				// forward power
#define TEMP 14					// PA Temperature

// board type limits
#define B1	819					// 1V
#define B2	1638				// 2V, F-sense board type = 1...2V
#define B3	2457				// 3V
#define B4	3276				// 4V
#define B5	4095				// 5V	

// Tone generator constants, FCYMHz / tone(Hz) * 2

// some harmony sounds, FCY = 7,5MHz
/*
#define HZ392_01	9566		// G, off
#define HZ466_85	8044		// Bp, push button standard tone
#define HZ587_31	6385		// D, fast tune tone
#define HZ698_45	5369		// F, CW side tone
*/

// some harmony sounds, FCY = 7,3728MHz
/*
#define HZ392_01	9404		// G, off
#define HZ466_85	7896		// Bp, push button standard tone
#define HZ587_31	6277		// D, fast tune tone
#define HZ698_45	5278		// F, CW side tone
*/

// some harmony sounds * 2, FCY = 7,3728MHz

#define HZ392_01	4702		// G, off
#define HZ466_85	3948		// Bp, push button standard tone
#define HZ587_31	6138		// D, fast tune tone
#define HZ698_45	2639		// F, CW side tone

/*
// some harmony sounds * 4, FCY = 7,3728MHz
#define HZ392_01	2351		// G, off
#define HZ466_85	1974		// Bp, push button standard tone
#define HZ587_31	1569		// D, fast tune tone
#define HZ698_45	1319		// F, CW side tone
*/


// meter scaling factors, factory defaults
#define BATT_MULT 132UL			// 100 x battery voltage ADC * BATT_MULT / 256 

//#define ID_MULT 6613UL			// default value for Drain current scaling, ID*100 = ADC * ID_MULT / 65536
#define ID_MULT 8266UL			// default value for Drain current scaling, ID*10 = ADC * ID_MULT / 65536
 
#define	S_MTR_MULT 1920UL		// default value for S-meter scaling, S-meter = ADC * S_MTR_MULT / 65536, 48 = max meter = 2.0V FS

//#define	REV_PWR_MULT 23UL		// default value for Reverse Power scaling
//#define	FWD_PWR_MULT 23UL		// default value for Forward Power scaling,P*100 =  ADC^2 * FWD_PWR_MULT / 65536, 10W = 1000

#define	REV_PWR_MULT 9UL		// default value for Reverse Power scaling
#define	FWD_PWR_MULT 9UL		// default value for Forward Power scaling,P*100 =  ADC^2 * FWD_PWR_MULT / 65536, 100W = 1000

#define PWR_GRAPH_MULT 18UL		// power bar graph,48=max = 10W, ADC^2 * PWR_GRAPH_MULT / 1048576
#define F_VOLT_GRAPH_MULT 1874UL // power bar graph,48=max = 10W, ADC * F_VOLT_GRAPH_MULT / 65536

// alarm limits
#define SWR_ALARM 300			// SWR alarm 3.00
#define SWR_AL_DLY 200			// PTT to alarm active delay about 400ms @ 2ms main loop


// Power on & calibration value addresses in the EEPROM
#define EEPAGE 0x7F				// EEPROM address high part
#define EEDEF 0xF000			// EEPROM default values storage area
#define EECAL 0xF040			// EEPROM calibration values storage area
#define EE_FD_LOC 0xF0F0		// counter address in EEPROM

// MIC / Line input
#define MIC 1					// Microphone input level / bias current on
#define LINE 0					// Line input level / bias off 



// PA100 board I/O definitions

// Port A Switches

#define INIT_TRISA 0xFF3F				// PortA switch inputs
#define INIT_PORTA 0x0000

#define AUTO	_RA12					// SW2, AUTO
#define DISP	_RA13					// SW1, FUNC / DISP
#define DN		_RA14					// SW8, DOWN button
#define UP 		_RA15					// SW5, UP button


// Port B ADC inputs

#define INIT_TRISB 0x7E0B				// PortB ADC and IDC pins = inputs
#define INIT_PORTB 0x0000

#define TX_ON		_LATB4				// 1 = PA active, RF power on
#define KEY			_RB3				// 1 = TX Request

#define OC_CLR		_LATB5				// Over current clear, 0 = clear

#define DB_4		_LATB6				// RF attenuators
#define DB_2		_LATB7


#define PTT_IN_IO	_RB3		// PTT input / TX on, 0 = active, Test


// Port C Switches & power control

#define INIT_TRISC 0x401E				//
#define INIT_PORTC 0x0000				//

#define OPER	_RC4					// SW3, OPER
#define BAND_DN	_RC3					// SW4, Band - button
#define BAND_UP	_RC1					// SW6, Band + button

#define F_SENSE	_RC14					// F-Sense counter input
#define PWR_ON	_RC13					// Power on


// Port D LCD & PWM out

#define INIT_TRISD 0x0001			// LCD & LED I/O, all outputs
#define INIT_PORTD 0x0000			//

// power switch
#define PWR_SW		_RD0			// Power switch input, 1 = switch pushed

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


// PORT F RS232

#define INIT_TRISF 0xFC7C			//
#define INIT_PORTF 0x0000


// PORT G FAN control

#define INIT_TRISG 0x8000			// RG15 input, others outputs
#define INIT_PORTG 0x0000

#define OC			_RG15			// Over Cueernt trip indicator from PA board

#define FAN1		_LATG2			// FAN High speed
#define FAN2		_LATG3			// FAN Low speed

#define TUNER		_LATG1			// Tuner relay


// Filter select I/O
#define M1_8		_LATG13			// 1.8 MHz
#define M3_5		_LATG12			// 3.5 MHz
#define M7			_LATG14			// 7 MHz
#define M10			_LATA7			// 10 MHz
#define M14_18		_LATA6			// 18-18 MHz
#define M21_28		_LATG0			// 21-28 MHz filter select


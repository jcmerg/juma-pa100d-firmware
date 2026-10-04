// JUMA-PA100 serial I/O test command set
// Juha Niinikoski, OH2NLT 06.07.2006

#include <stdio.h>					// Added - 5B4AIY
#include <ctype.h>					// Added - 5B4AIY
#include "juma-pa100.h"				// Board definitions
#include "DataEEPROM.h"				// EEPROM I/O definitions, interface to assembler module
#include "pa100_eeprom.h"			// Get EEPROM structure definitions

#define	DEBUG		FALSE			// Used for Test & Debug
#define	CRC_CHK		FALSE			// Used to verify CRC calculation

// External references
// External functions
extern unsigned char kbhit(void);
extern unsigned char getch(void);
extern unsigned long getlong(void);				// see uart.c
extern unsigned char get2hex(void);
extern void ClearUSART1queue();
extern void draw_s_meter(int);					// LCD functions
extern void set_chgen(int);
extern void lcd_cmd(unsigned char);
extern void lcdoutch(unsigned char);			// Added - 5B4AIY
extern void lcd_putst(register const char *);
extern void display_screen(const char *, const char *);
extern int convert_adc12(unsigned int);			// ADC
//extern void exchg_data_spi1(unsigned int);	// Not Used
extern void ms_delay(unsigned int);				// General delay
extern void beep(int, int);						// Added - 5B4AIY
extern int get_817_band(int);					// Get Yaesu 817 band data
extern int get_xiegu_band(int);					// Get Xiegu band data
extern void display_hdr(void);
extern void save_defaults();
extern void save_calval();
extern unsigned int crc_16(unsigned int, unsigned int);
extern unsigned int crc_8(unsigned char, unsigned int);

// External Data
extern int fsense_tst;				// Flag for F-sense test printouts
extern int br_txt[];
extern int alarms;
extern int last_man_band;
extern int pa_state;

extern char lcdpbuff[];	// LCD print buffer

extern const char firmware[];
extern const char copyright[];
extern const char additional_features[];

extern const char *c_or_f[];
extern const char *f_sense[];
extern const char *band_units[];
extern const char *on_off[];
extern const char *amp_state[];
extern const char *band_select[];
extern const char *bs_txt[];
extern const char *mb_txt[];
extern const char *graph_type[];
extern const char *pwr_mtr[];
extern const char *start_page_select[];
extern const char T_Char[];

// EEPROM Structures
// User Configuration Data
extern struct
	{
	struct defval defval;
	} eeprom;

// System Calibration Data
extern struct
	{
	struct calval calval;
	} cal;

// Local Data
const char temp_format[] = {"Sensor:%5.1f%c Displayed:%4.0f%c\n\r"};
const char LCD_Test[] = {"LCD Bar Graph & Character Test"};
const char Write_ASCII[] = {"Write ASCII to LCD, ESC to exit.\n\r"};
const char Write_HEX[] = {"Write HEX to LCD, 1B to exit.\n\r"};
const char ov_msg[] = {"High-Voltage"};
const char uv_msg[] = {"Low-Voltage"};
const char pl_msg[] = {" (Pre-Limit)"};
const char alarm_is_disabled[] = {" alarm is disabled.\n\r"};
const char amps_volts[] = {"%7.3f %s"};
const char pwr_fmt[] = {"%5.1f Watts (%s)"};
const char temp_fmt[] = {"%5.0f "};
const char fan_temp_fmt[] = {"%s%-d%c\n\r"};
const char batt_voltage_dsp[] = {"%4i = %5.2fV\n\r"};
const char band_select_msg[] = {"Last Valid Band Select :%s\n\r"};
const char dividing_line[] = {"----------------------------------------\n\r"};
const char any_key_exits[] = {", any key exits...\n\r"};
const char sensor_cal_msg[] = {"Temperature Sensor Calibration\n\r"};
const char Input_Atten_Status[] = {"Input Attenuator Status\n\r"};
const char Divide_By_Zero[] = {"Divide-By-Zero Trap\n\r"};
const char Test_Terminated[] = {"Test Terminated\n\r"};
const char Alarm_Test[] = {"Alarm System Test\n\r"};
const char fsense_tst_on_off[] = {"\n\rF-Sense Test: %s\n\r"};
const char fsense_hdr[] = {"   160m 80m 40m 30m 20m 17m 15m 12m 10m\n\r"};
const char bad_value[] = {"\n\rIllegal Value!\n\r"};
const char npc[] = {"Non-Printable Character"};

const char *fan_ctrl[] = {		// Indexed by: Fan_Speed
						"Normal",
						"Low Speed ON",
						"Medium Speed ON",
						"High Speed ON"
						};

const char *y817[] = {			// Indexed by: j in A-D Converter test
						"Out-Of-Band!",	// 0
						"1.8MHz/160m",	// 1
						"3.5MHz/80m",	// 2
						"7.0MHz/40m",	// 3
						"10MHz/30m",	// 4
						"14MHz/20m",	// 5
						"18MHz/17m",	// 6
						"21MHz/15m",	// 7
						"24MHz/12m",	// 8
						"28MHz/10m",	// 9
						"Unknown Band!"	// 10
					};

double get_factor(double adc, int cal_factor, double divisor)
	{
	return adc * (double)cal_factor / divisor;
	}

void dump_eeprom(void)
	{
	int i;
	unsigned int w;

	printf("\n\rDump EEPROM contents\n\r%4s%23s\n\r%.23s%.22s", "ADDR", "DATA", dividing_line, dividing_line);

	for(i = 0; i < 256; i += 2)					// Byte address for EEPROM, but data is stored in words
		{
		ReadEE(EEPAGE, (i + EEDEF), (int *)&w, WORD);	// EEPROM Address 8 high bits, address + physical EEPROM start 16 low bits

		if((i % 16) == 0) printf("\n\r%4.4X:", i);

		printf("%5.4X", w);
		}

	printf("\n\r%.23s%.22s\n\r", dividing_line, dividing_line);
	}

// Only Used For Test & Debug
#if DEBUG
void erase_EEPROM(void)
	{
	int i;

	dump_eeprom();

	for(i = 0; i < 256; i += 2) EraseEE(EEPAGE, (i + EEDEF), WORD);

	dump_eeprom();
	save_defaults();
	save_calval();
	dump_eeprom();
	}
#endif

void print_pwr(double factor, char *c)
	{
	factor *= factor;
	printf(pwr_fmt, get_factor(factor, cal.calval.fwd_pwr_mult, 100000000.0), c);
	}

void ad_msg0(double factor)
	{
	printf(amps_volts, get_factor(factor, cal.calval.id_mult, 200000.0), "Amps");
	}

void ad_msg1(double factor)
	{
	printf(amps_volts, get_factor(factor, cal.calval.batt_mult, 1000000.0), "Volts");
	}

void ad_msg2(double factor)
	{
	int i;

	i = (Band_Select_Mode == XIEGU)				// Show the band for the selected voltage table.
		? get_xiegu_band((int)factor)
		: get_817_band((int)factor);
	printf("%3i - %s", i, y817[i]);
	}

void ad_msg3(double factor)
	{
	print_pwr(factor, "Reverse");
	}

void ad_msg4(double factor)
	{
	factor += (double)cal.calval.lo_pwr_offset;
	print_pwr(factor, "Forward");
	}

void ad_msg5(double factor)
	{
	if(Temp_Scale) printf(temp_fmt, factor / T_MULTC);
	else printf(temp_fmt, (factor / T_MULTF) + 32.0);

	printf("%s", c_or_f[Temp_Scale]);
	}

void (*ad_msg[])(double) = {
							ad_msg0,
							ad_msg1,
							ad_msg2,
							ad_msg3,
							ad_msg4,
							ad_msg5
							};

void alarm_msg0(void)
	{
	printf("0 - All Alarms Off\n\r");
	alarms = 0;
	}

void alarm_msg1(void)
	{
	printf("1 - Over-Current\n\r");
	alarms |= CURR_AL;
	}

void alarm_msg2(void)
	{
	printf("2 - High SWR\n\r");
	alarms |= SWR_AL;
	}

void alarm_msg3(void)
	{
	printf("3 - Over-Temperature\n\r");
	alarms |= TEMP_AL;
	}

void alarm_msg4(void)
	{
	if(cal.calval.alarm_flags & HI_V)
		{
		printf("4 - %s\n\r", ov_msg);
		alarms |= HI_V;
		}
	else printf("4 - %s%s", ov_msg, alarm_is_disabled);
	}

void alarm_msg5(void)
	{
	if(cal.calval.alarm_flags & LO_V)
		{
		printf("5 - %s%s\n\r", uv_msg, pl_msg);
		alarms |= LO_BATT;
		}
	else printf("5 - %s%s%s", uv_msg, pl_msg, alarm_is_disabled);
	}

void alarm_msg6(void)
	{
	if(cal.calval.alarm_flags & LO_V)
		{
		printf("6 - %s Limit\n\r", uv_msg);
		alarms |= LO_V;
		}
	else printf("6 - %s%s", uv_msg, alarm_is_disabled);
	}

void alarm_msg7(void)
	{
	printf("7 - All Alarms On\n\r");
	alarms = ALL_ALARMS_ON;
	}

void (*alarm_msg[])(void) = {
							alarm_msg0,	// 0 - All Alarms OFF
							alarm_msg1,	// 1 - High Current Alarm
							alarm_msg2,	// 2 - High SWR Alarm
							alarm_msg3,	// 3 - High Temperature Alarm
							alarm_msg4,	// 4 - High Voltage Alarm
							alarm_msg5,	// 5 - Low Voltage Pre-Limit Alarm
							alarm_msg6,	// 6 - Low Voltage Alarm
							alarm_msg7	// 7 - All Alarms ON
							};

void serial_test(void)
	{
	double a;
	long e, f;
	int i, j;
	unsigned char c;

	if(kbhit())
		{
		c = toupper(getch());

		switch(c)
			{
// Info commands
			case 'H':	// System info
			case '?':
				display_hdr();
				printf("\n\r%.20s[COMMAND TABLE]%.20s\n\r", dividing_line, dividing_line);
				printf("H/? Help - (This Screen)\n\r");
				printf("A   ADC Channel Dump\n\r");
				printf("B   %s", Alarm_Test);
				printf("C   %s\n\r", LCD_Test);
				printf("D   Clear Factory Default Reset Counter\n\r");
				printf("E   Dump System & User Settings\n\r");
				printf("F   Dump EEPROM contents\n\r");
				printf("G   Toggle Frequency Sense Test On/Off\n\r");
				printf("I   %s", Input_Atten_Status);
				printf("J   %s", sensor_cal_msg);
				printf("K   Buzzer Sound Test\n\r");
				printf("L   %s", Write_ASCII);
				printf("M   %s", Write_HEX);
				printf("Z   %s%.15s%s", Divide_By_Zero, dividing_line, dividing_line);
			break;

// Test Commands
			case 'A':	// Measure and display all ADC channels 9 - 14
				printf("\n\r%7s%6s%8s%15s\n\r%.22s%.22s", "CHANNEL", "VALUE", "INPUT", "DISPLAY", dividing_line, dividing_line);
				i = 9;

				do	{
					j = convert_adc12(i);
					printf("\n\rADC%3i:%6i%7.3fV ", i, j, get_factor((double)j, 5, 4096.0));
					ad_msg[i - 9]((double)j);
					} while (++i < 15);

				printf("\n\r%.4s%s", dividing_line, dividing_line);
			break;

			case'B':	// Alarm System Test
				printf("\n\r%sEnter 0,1,2,3,4,5,6 or 7 ...\n\r", Alarm_Test);
				c = getch() & 0x07;				// Ensure value is in range.
				alarm_msg[(int)c]();
			break;

#if DEBUG
			case '*':							// Test & Debug Facility
				erase_EEPROM();
			break;
#endif

			case 'C':	// Bar Graph Test
				printf("\n\r");
				printf("%s%sSelect Scale: 0,1,2: ", LCD_Test, any_key_exits);
				c = getch() % 3;
				printf("%d - %s\n\r", c, graph_type[c]);
				set_chgen(c);					// Select the desired character generator map, and clear the display.
				c = 0;							// Initialise the character

				do	{
					i = 0;
					lcd_cmd(LINE2PLUS4);
					lcdoutch('[');
					lcdoutch(c);
					sprintf((char *)lcdpbuff, " =%3.2X]", c);	// Display LCD characters - Added cast - 5B4AIY
					lcd_putst((char *)lcdpbuff);

					do	{
						lcd_cmd(LINE1PLUS4);
						draw_s_meter(i);
						ms_delay(10);
						} while (++i < 49);

					c++;
					ms_delay(500);
					} while (!kbhit());

				getch();						// Consume break character
				set_chgen(Scale_Type);			// Load the original bar graph fonts
				printf(Test_Terminated);
			break;

			case 'D':	// Clear factory default reset counter
				printf("\n\rEEPROM Reset Counter cleared.");
				j = 0;
				EraseEE(EEPAGE, EE_FD_LOC, WORD);
				WriteEE(&j, EEPAGE, EE_FD_LOC, WORD);
				dump_eeprom();
			break;

#if CRC_CHK
			case '#':	// CRC Check (Debug)
				i = crc_8(0xC3, 0x00);
				printf("\n\rData: %.2X  CHECKSUM: %.4X", 0xC3, i);
				i = crc_8(0x55, i);
				printf("\n\rData: %.2X  CHECKSUM: %.4X", 0x55, i);
				i = crc_8(0xAA, i);
				printf("\n\rData: %.2X  CHECKSUM: %.4X", 0xAA, i);
				printf("\n\rFinal Checksum should be: 1806\n\r");
				i = crc_16(0x55C3, 0x00);
				printf("\n\rData: %.4X  CHECKSUM: %.4X", 0x55C3, i);
				i = crc_16(0x00AA, i);
				printf("\n\rData: %.4X  CHECKSUM: %.4X", 0x00AA, i);
				printf("\n\rFinal Checksum should be: 06FA\n\r");
			break;
#endif

			case 'E':	// Dump System & User Settings
// System Calibration Settings
				printf("\n\r%33s\n\r%7sFirmware: %s Build: %s\n\r%33s", "SYSTEM CALIBRATION SETTINGS", " ", VERSION, BUILD_NUMBER, dividing_line);
				printf("Battery Voltage Factor : %-4d\n\r", cal.calval.batt_mult);
				printf("PA Current Factor      : %-4d\n\r", cal.calval.id_mult);
				printf("RF Power Meter Factor  : %-4d\n\r", cal.calval.fwd_pwr_mult);
				printf("RF Power Meter Offset  : %-4d\n\r", cal.calval.lo_pwr_offset);
				printf("Beep Length Time       : ");
				(cal.calval.beep_len)
					? printf("%-d mS\n\r", cal.calval.beep_len)
					: printf("OFF\n\r");

				printf("PWR Measurement Samples: ");
				(cal.calval.samples == SAMPLE_MIN) ? printf("1 (Off)\n\r") : printf("%d\n\r", cal.calval.samples);

				printf("Over-Voltage Trip      : ");
				(Enabled_Alarms & HI_V)
					? printf(batt_voltage_dsp, cal.calval.overvoltage_trip, get_factor((double)cal.calval.overvoltage_trip, cal.calval.batt_mult, 1000000.0))
					: printf("%s\n\r", on_off[OFF]);

				printf("Under-Voltage Trip     : ");
				(Enabled_Alarms & LO_V)
					? printf(batt_voltage_dsp, cal.calval.undervoltage_trip, get_factor((double)cal.calval.undervoltage_trip, cal.calval.batt_mult, 1000000.0))
					: printf("%s\n\r", on_off[OFF]);

				printf("Pre-Limit Trip         : ");
				(Enabled_Alarms & LO_V)
					? printf(batt_voltage_dsp, cal.calval.pre_limit_trip, get_factor((double)cal.calval.pre_limit_trip, cal.calval.batt_mult, 1000000.0))
					: printf("%s\n\r", on_off[OFF]);
				
				printf("Power Meter Full-Scale : %-dW\n\r", cal.calval.max_power);
				printf("Freq Meter Calibration : %-7ld\n\r", cal.calval.freq_cal);
				printf("Splash Screen Display  : %s\n\r", on_off[(int)cal.calval.splash]);
// User Configuration Settings
				printf("\n\r%33s\n\r%s", "USER CONFIGURATION SETTINGS", dividing_line);
				printf("Frequency Sense Mode   : %s\n\r", f_sense[Band_Select_Mode]);
				printf("Serial Link Polling    : ");
				(Poll_Time)
					? printf("%-d Seconds", Poll_Time)
					: printf("Off");
				printf("\n\rRS-232 Port Mode       : Test Mode\n\r");
				printf("RS-232 Port Speed      : %-d00 Baud\n\r", br_txt[eeprom.defval.br]);
				printf("LCD Backlighting       : %-4d\n\r", eeprom.defval.back_light);
				printf("LCD Contrast           : %-4d\n\r", eeprom.defval.contrast);
				printf("SWR Trip Limit         : %.2f\n\r", (double)(SWR_Trip) / 100);
				printf("Fan Speed Control      : %s\n\r", fan_ctrl[Fan_Speed]);
				printf("Temperature Units      : %s\n\r", c_or_f[Temp_Scale]);
				printf(fan_temp_fmt, "Temperature Alarm Limit: ", Alarm_Temp, T_Char[Temp_Scale]);
				printf(fan_temp_fmt, "Fan Start Temperature  : ", Fan_Start, T_Char[Temp_Scale]);
				printf("Band Display Units     : %s\n\r", band_units[Band_Units]);
				printf("Graphic Limits Display : %s\n\r", on_off[Graph_Limits]);
				printf("F-Sense QSK            : %s\n\r", on_off[FSense_QSK]);
				printf("Graphic Display Type   : %s\n\r", graph_type[Scale_Type]);
				printf("Power Meter Type       : %s\n\r", pwr_mtr[Power_Units]);
				printf("Start-Up Page          : %s\n\r", start_page_select[Start_Page]);
				printf("Current Amplifier State: %s\n\r", amp_state[pa_state]);
				printf("Band Select Mode       : %s\n\r", band_select[Auto_Manual]);
				printf(band_select_msg, (Band_Units) ? mb_txt[last_man_band] : bs_txt[last_man_band]);
				printf("%s", dividing_line);
			break;

			case 'F':	// Dump EEPROM
				dump_eeprom();
			break;

			case 'G':	// Frequency-Sense Test On / Off
				fsense_tst ^= 1;

				if(fsense_tst)
					{
					printf(fsense_tst_on_off, on_off[fsense_tst]);
					printf(fsense_hdr);
					printf("%.39s\n\r", dividing_line);
					}
				else
					{
					printf("%.39s", dividing_line);
					printf(fsense_tst_on_off, on_off[fsense_tst]);
					}
			break;

			case 'I':	// Display Input Attenuator Settings
				printf("\n\r%s", Input_Atten_Status);
				i = 1;

				do	{
					printf("Band: %s G%1i: = %2idB\n\r", bs_txt[i], 1 + eeprom.defval.rf_gain[i], -2 * (3 - eeprom.defval.rf_gain[i]));
					} while (++i < 10);
			break;

			case 'J':	// Calibrate Temperature Sensor
				printf("\n\r%sPress any key to start%s\n\r", sensor_cal_msg, any_key_exits);
				getch();
				c = T_Char[Temp_Scale];

				do	{
					a = 0.0;
					i = 0;

					do	{						// Average reading over 10 samples.
						a += (double)convert_adc12(TEMP);
						} while (++i < 10);

					if(Temp_Scale)
						{
						a /= (T_MULTC * 10.0);
						}
					else
						{
						a /= (T_MULTF * 10.0);
						a += 32.0;
						}

					printf(temp_format, a, c, a, c);
					ms_delay(50);
					} while (!kbhit());

				getch();						// Flush buffer
				printf(Test_Terminated);
			break;

			case 'K':	// Buzzer Sound Test, Pitch = FCY MHz / tone(Hz) * 2
				printf("\n\rSound Test\n\rEnter Frequency (100 - 4000Hz): ");
				e = getlong();

				if(e < 100L || e > 4000L)
					{
					printf(bad_value);
					break;
					}

				f = FCY / (e << 1);
				printf("\n\rData Value: %li\n\r", e);
				printf("Enter duration (10 - 32,000mS): ");
				e = getlong();

				if(e < 10L || e > 32000L)
					{
					printf(bad_value);
					break;
					}

				beep((int)f, (int)e);
				printf("\n\r");
			break;

			case 'L':	// Test LCD Character Generator, ASCII
				printf("\n\r%s", Write_ASCII);
				clear_lcd();
				i = 0;

				do	{
					c = getch();

					if(i == 16) lcd_cmd(LINE2);
					if(c == 0x0D || i == 32)
						{
						clear_lcd();
						i = 0;
						}

					if(isprint(c))
						{
						printf("Char = 0x%.2X\n\r", c);
						lcdoutch(c);
						i++;
						}
					else printf("Char = 0x%.2X - %s\n\r", c, npc);
					} while (c != 0x1B);

				printf(Test_Terminated);
			break;

			case 'M':	// Test LCD Character Generator, HEX
				printf("\n\r%s", Write_HEX);
				clear_lcd();
				i = 0;

				do	{
					c = get2hex();

					if(i == 16) lcd_cmd(LINE2);
					if(i == 32)
						{
						clear_lcd();
						i = 0;
						}

					lcdoutch(c);

					if(isprint(c))
						{
						i++;
						printf(" = %c\n\r", c);
						}
					else printf(" = %s\n\r", npc);
					} while (c != 0x1B);

				printf(Test_Terminated);
			break;

			case 'Z':	// Trap Divide By Zero Error Test
				i = 1;
				j = 0;
				printf("\n\r%s", Divide_By_Zero);
				i /= j;							// Generate error ==> trap
			break;

			default:	// Invalid Command
				printf("\n\rInvalid Command: %c Value: %d\n\r", c, c);
				ClearUSART1queue();				// Flush the receive buffer
			}	// End SWITCH
		}	// End IF
	}	// End FUNCTION


// JUMA-PA100 EEPROM save structures
// Juha Niinikoski, OH2NLT 14.07.2008
// Changed id_mult, batt_mult, and fwd_pwr_mult to integers.
// Removed rev_pwr_mult, it is always the same as fwd_pwr_mult. A.Ryan 5B4AIY 22/OCT/2012

// User Configuration Values
struct defval
	{
	int rf_gain[11];				// RF Gain setting for each band. Bands 0 and 10 are UNKNOWN, and set to 0, Valid bands are 1 to 9.
	int band;						// Selected band
	int bsel_mode;					// Auto band select mode, 0 = Yaesu CAT, 1 = KX2/KX3, 2 = Juma TRX-2, 3 = F-Sense, 4 = FT817 Analog Voltage, 5 = Manual, 6 = Xiegu Analog Voltage
	int poll_timer;					// Juma TRX-2/Elecraft KX3 Polling Interval Timer
	int contrast;					// LCD display contrast, pwm4
	int back_light;					// LCD back light, pwm3 
	int serial_test;				// Serial interface test mode 0 = Off, 1 = Remote, 2 = Test
	int br;							// Baud rate index
	int swr_limit;					// SWR alarm limit
	int temp_units;					// Temperature Units, 1 = C, 0 = F
	int fan_control;				// Fan Speed Control, 0 = Normal, 1 = Low, 2 = Medium, 3 = High
	int temp_limit;					// Temperature alarm limit
	int fan_start;					// Cooling fan start temp
	int pa_state;					// Amplifier state, 0 = Standby, 1 = Operate
	int auto_band;					// Band select type, 0 = Manual, 1 = Auto
	int band_units;					// 0 = MHz, 1 = Metres
	int graph_limits;				// Graphical Limits Display
	int display_type;				// Graphic Display Type
	int rf_power;					// RF Power Display, 0 = Watts, 1 = dBm
	unsigned int d_csum;			// Checksum
	};

// System Calibration Values
struct calval
	{
	int id_mult;					// Drain current meter calibration
	int batt_mult;					// Battery voltage meter calibration
	int fwd_pwr_mult;				// Forward power meter calibration
	int	beep_len;					// Tone marker length(ms)
	unsigned char samples;			// Power measurement averaging samples
	unsigned char splash;			// Splash screen flag
	int alarm_flags;				// Used to enable/disable alarms
	int overvoltage_trip;			// Over-voltage trip (Nominally 2762 = 14.5V)
	int undervoltage_trip;			// Under-voltage trip (Nominally 2096 = 11.0V)
	int pre_limit_trip;				// Pre-Limit trip voltage (Nominally 200mV, but adjustable between 100mV and 800mV)
	int max_power;					// Graphical power meter setting
	long freq_cal;					// Frequency Meter calibration factor
	int lo_pwr_offset;				// Offset for the low power range of the RF power meter
	unsigned int c_csum;			// Checksum
	};
/*
 Extension block for settings added by DL4JC. It is stored separately at EEEXT, so that the original configuration and
 calibration blocks keep their layout: no checksum error when loading this firmware, and the original firmware can
 still be loaded without losing the calibration (it ignores this block). If the block is missing or invalid, only
 these settings are set to their defaults. New settings use a spare word and keep their default value 0, or increase
 EXT_VERSION.
*/
struct extval
	{
	unsigned int magic;				// EXT_MAGIC
	unsigned int version;			// EXT_VERSION
	int fsense_qsk;					// F-Sense QSK, 0 = Off, 1 = On
	int spare[4];					// Reserved, 0
	unsigned int e_csum;			// Checksum
	};


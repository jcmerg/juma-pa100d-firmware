<div class="titlepage">

<p class="title">JUMA PA-100D<br>Operating Manual</p>

<p class="sub">Firmware v5.01 (DL4JC) · October 2026</p>

<p class="based">Based on the operating manual for firmware v4.00a by Adrian Ryan, 5B4AIY
(revision 15-DEC-2021). Original firmware by Juha Niinikoski, OH2NLT, and Matti Hohtola, OH7SV.</p>

<p class="note"><strong>Not an official JUMA release.</strong> You use this firmware at your own
risk. Test every new firmware with a dummy load and low power first. You can return to the
original v4.01a at any time without losing the calibration (see section 10).</p>

</div>

<div class="toc-page"><p class="toc-title">Contents</p>

[[TOC]]

</div>

# 1 Introduction

The PA-100D is a 100 W all-band (160–10 m) linear amplifier, designed as a companion to the JUMA
TRX-2 but usable with almost any low-power transceiver (Elecraft KX2/KX3, Yaesu FT-817/818, Xiegu,
Icom IC-705 and others). This manual describes operation and setup with **firmware v5.01**. It runs
on the JUMA PA-100D and on the RS-928 clone.

## 1.1 Version numbers

Some dealers list the original firmware **v4.01a** as **"4.1a"**. That is the original firmware,
not a newer version. This firmware continues from v4.01a Build 3 (5B4AIY) and starts at **v5.00**
so that the two cannot be confused. The version is shown on the start-up screen and on the
*About* page of the user configuration.

## 1.2 What is new compared with v4.01a

**Protection**

- All TX protection runs in the 1 ms interrupt: RF off 2 ms after KEY is released, SWR trip,
  over-current, filter mismatch, active alarms, and a main-loop watchdog (2 s). In v4.01a the
  protection stopped whenever the main loop waited (button held, save prompt, serial test).
- The filter relays are never switched under power. On a band change RF is turned off first, the
  relays switch 20 ms after RF off, and TX is held off for another 20 ms while they settle.
- The input frequency is compared with the selected low-pass filter every millisecond. Above the
  filter for 2 ms → RF off until the band is correct. This cures the **O/C alarm** of v4.01a on the
  first transmission after a band change in F-Sense mode.
- The gain (attenuator) relays no longer switch while transmitting.
- RF is turned off in the user configuration, on a processor fault and before power-off.

**Band selection**

- F-Sense works reliably with two-tone, noise and SSB, and never selects a filter below the
  transmitted frequency (section 6.2).
- New switch **F-Sense QSK** (section 4.17).
- New band select modes **Xiegu** (band voltages, section 8.4) and **HR50** (Hardrock-50 serial
  protocol, section 8.5).
- KX2/KX3: frequencies above 30 MHz no longer wrap to a low band; incomplete or corrupted `FA`
  messages are ignored.

**Operation**

- **OPER held** saves the settings without switching off (section 5).
- New user configuration page **About** with version and credits; the start-up screen shows only
  the model and firmware version.
- Service menu **Beep Tone** for the RS-928 buzzer (section 7.6).
- Serial receive overrun at high baud rates fixed; status reply sent in one piece.

The calibration and configuration in the EEPROM are kept when you load this firmware.

# 2 Front panel controls

The PA-100D has eight buttons: **PWR**, **UP**, **DOWN**, **DISPLAY/CONFIG**, **OPER**, **AUTO**,
**BAND+** and **BAND−**. "Brief press" means less than approx. 0.5 s, "hold" means approx. 0.7 s or
longer. The quick reference in section 11 summarises all functions.

## 2.1 PWR

- **Power on:** briefly press PWR. The start-up screen appears (if *Splash Screen* is on):

  <pre class="lcd">  JUMA PA-100D
Firmware   v5.01</pre>

  The amplifier always starts in **Standby**.
- **Power off:** hold PWR. If you have changed a gain, the band (manual mode) or Auto/Manual since
  the last save, the save prompt appears first (section 5). RF is turned off before the fans.
- **Cancel alarms:** a brief press clears the most urgent active alarm. If an alarm cannot be
  cleared, hold PWR for an emergency shutdown; nothing is saved.
- **Previous page:** in the normal display a brief press steps back to the previous measurement
  page (e.g. quickly between power and SWR). In the user configuration and the service menu it
  steps back one page; held, the pages step back automatically.
- **No** in every save prompt.

## 2.2 UP / DOWN

In the normal display UP/DOWN change the **gain** of the current band (G1–G4). G1 is the lowest
gain; each step is approx. 2 dB. The "gain" is in fact an input attenuator (G1 = −6 dB … G4 = 0 dB).
Each of the nine bands has its own gain setting.

A gain change while transmitting is applied **after** the transmission, so that the attenuator
relays never switch under drive power.

In the menus UP/DOWN change the value of the current page. Held, they repeat; the repeat speed
depends on the parameter.

## 2.3 DISPLAY/CONFIG

- **Brief press** in the normal display: next measurement page – output power, SWR, supply voltage,
  current, heat-sink temperature, and in F-Sense mode an additional page with the input frequency
  (1 kHz resolution) and output power.
- **Hold:** enters the **user configuration** (section 4). In the user configuration a brief press
  goes to the next page; held, the pages advance automatically.
- **Held while switching on:** RS-232 loop-back test. Characters received from a terminal are
  echoed and shown on the display. Brief PWR ends the test.

## 2.4 OPER

- **Brief press** in the normal display: toggles **Operate / Standby**. The change happens when the
  button is **released**.
- **Hold** (approx. 0.7 s, not while transmitting): **save prompt** for the current settings,
  without switching off (section 5).
- In the user configuration and the service menu: **save & exit** prompt.
- **Held while switching on:** starts the boot loader (flash writer) for a firmware update
  (section 10).

In **Standby** the signal path goes straight through, without filters or attenuator; the
transceiver works directly into the antenna. The meters still show power and SWR. In **Operate**
the amplifier is active.

## 2.5 AUTO

Toggles between **automatic** (`A` on the lower display line) and **manual** (`M`) band selection.
In manual mode BAND+/BAND− select the band (160 m … 10 m). Pressing BAND+ or BAND− while in AUTO
also switches to manual. Not available in the band select mode *Manual*.

With a serial band select mode (Yaesu CAT, KX2/KX3, Juma-TRX2), every manual band change and every
change between A and M sends a frequency query to the transceiver, and a valid answer overrides the
manual band. This prevents an inadvertently wrong filter.

## 2.6 BAND+ / BAND−

- Select the next higher / lower band in manual mode; held, the band repeats. Not possible while
  transmitting.
- **BAND+** is **Yes** in every save prompt.
- **BAND+ held while switching on:** reload the factory defaults (prompt: BAND+ = Yes, PWR = No).
- **BAND− held while switching on:** the serial message time-out of the HR50 mode becomes 5 s
  instead of 200 ms, for tests with a terminal program (display: *Serial Time-Out / Set to: 5
  Secs*). Until the next power-on. The KX2/KX3 mode no longer needs this: it works with a terminal
  at any typing speed.

## 2.7 Normal display

<pre class="lcd">▮▮▮▮▮▯▯  52.7W
G3 OPER A 14MHz</pre>

Upper line: bar graph and the selected measurement. Lower line: gain (G1–G4), state, A/M and band.

| State | Meaning |
|---|---|
| `STBY` | Standby |
| `OPER` | Operate, receiving |
| `TX` | Operate, transmitting |
| `Err` | KEY active, but the band is unknown or out of band – TX inhibited |
| `SWR`, `O/C`, `TEMP`, `Hi-V`, `Lo-V`, `Batt` | Alarm (flashing, section 9) |

Band `?` = band unknown (e.g. no answer to a poll), `?!` = out of band. Both inhibit TX.

# 3 Getting started

1. Connect the transceiver to the RF input, the antenna (or dummy load) to the output, and the PTT
   (KEY) line to the T/R socket. Use a fused power lead.
2. Switch on, enter the user configuration (DISPLAY held) and choose the **Auto Band Detect**
   mode that suits your transceiver (section 4.1). Without a CAT/band data connection use
   **F-Sense**.
3. Leave with OPER and save (BAND+).
4. Start with gain G1 and low drive (1–2 W), check power and SWR, then increase.

# 4 User configuration

Hold **DISPLAY/CONFIG** until a long beep sounds and *User Configuration* appears. Release the
button; the last used page is shown. Next page: DISPLAY brief (or held to scroll). Previous page:
PWR brief (or held). Change the value with **UP/DOWN**. A long beep and a longer delay mark page 0.
TX is off as long as the menu is open.

To leave, briefly press **OPER** and answer the save prompt: **BAND+** saves, **PWR** discards
the changes and restores the previous settings.

Pages that do not apply to the current mode are skipped (see notes on each page).

## 4.1 Auto Band Detect

Default: **F-Sense**

| Mode | Band source | Notes |
|---|---|---|
| **Yaesu CAT** | Yaesu 5-byte binary CAT protocol (e.g. FT-817/818 with CT-62 level converter) | Polling is set to 2 s when selected |
| **KX2/KX3** | ASCII `FA` frequency (Elecraft, Kenwood, newer Yaesu, any radio that answers `FA;`) | Polling set to 2 s when selected; section 8.3 |
| **HR50** | The PA answers the Hardrock-50 serial commands; band from the host | Section 8.5 |
| **Juma-TRX2** | Band messages of the JUMA TRX-2 | Polling not needed, disabled when selected |
| **F-Sense** | Frequency of the RF drive signal | Only RF and PTT needed; section 6.2 |
| **FT817/818** | Band voltage on the BAND socket (FT-817/818) | |
| **Xiegu** | Band voltage of the Xiegu ACC port (230 mV steps), also Brick2/3, SquareSDR | Section 8.4 |
| **Manual** | Only BAND+/BAND− | **No protection at all** – a wrong band can destroy the filters or transistors. Use only as a last resort. |

The menu order is: Yaesu CAT → KX2/KX3 → HR50 → Juma-TRX2 → F-Sense → FT817/818 → Xiegu → Manual.

## 4.2 Serial Speed

Default: **9600 Baud** · Range: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 Baud ·
8 data bits, 1 stop bit, no parity.

## 4.3 Serial Port

Default: **Off** · Off / Remote / Test

- **Remote:** remote control and status (section 8.6).
- **Test:** serial test suite (section 8.1).

Only shown in the modes F-Sense, FT817/818, Xiegu and Manual – in the other modes the serial port
carries the band data.

## 4.4 Polling Interval

Default: **2 s** · Range: 0 (off) – 10 s

- Yaesu CAT, KX2/KX3, Juma-TRX2: the transceiver is asked for its frequency at this interval. If no
  answer arrives within one interval, the band becomes unknown (`?`) and TX is inhibited until the
  next valid answer.
- Remote mode: the amplifier sends a status message at this interval (section 8.6).

Only shown in Yaesu CAT, KX2/KX3, Juma-TRX2, or with *Serial Port = Remote*. Not shown in HR50 mode
(the host sends the band).

## 4.5 LCD Display – Backlight

Default: **300** · Range: 50 – 1000

## 4.6 LCD Display – Contrast

Default: **2000** · Range: 0 – 3000

## 4.7 SWR Trip

Default: **3.0** · Range: 1.0 – 9.0

The SWR alarm only trips in **Operate**. 1.0 is useful for testing the alarm with a dummy load.

## 4.8 Fan Control

Default: **Normal**

| Setting | Fan |
|---|---|
| Normal | Controlled only by the heat-sink temperature |
| Low | Always at least low speed; higher speeds by temperature |
| Medium | Always at least medium speed; high speed by temperature |
| High | Always high speed |

## 4.9 Temperature

Default: **Celsius** · Celsius / Fahrenheit. Changing the scale resets the over-temperature limit
and fan cut-in temperature to their defaults in the new scale.

## 4.10 Overtemp

Default: **70 °C / 158 °F** · Range: 50 – 100 °C / 120 – 212 °F

## 4.11 Fan Cut-In Temp

Default: **40 °C / 104 °F** · Range: 0 °C (32 °F) up to the over-temperature limit − 20 °C (− 40 °F)

At the cut-in temperature the fan starts at low speed, 5 °C (10 °F) higher at medium, 10 °C
(20 °F) higher at high speed. It stops 2 °C (4 °F) below the cut-in temperature. A low setting
(e.g. 0 °C) is a quick test of the fan.

## 4.12 Band Display

Default: **MHz** · MHz / Metres

## 4.13 Graphical Limits

Default: **Off**

- **Off:** the bar graph shows the output power; full scale is set in the service menu (40–160 W).
- **On:** the bar graph shows the selected measurement relative to its limit: current to the 24 A
  hardware trip, temperature to the over-temperature limit, SWR from 1:1 to the trip limit,
  voltage from the under-voltage to the over-voltage trip. Recommended with the dBm power meter.

## 4.14 Graphic Display

Default: **Original** · Original / Large / Small scale markers.

## 4.15 RF Power Meter

Default: **Watts** · Watts / dBm (1 W = +30 dBm, 10 W = +40 dBm, 100 W = +50 dBm).

## 4.16 Start-Up Display

Default: **O/P Power** · O/P Power / SWR / Voltage / Current / Temperature – the measurement page
shown after power-on.

## 4.17 F-Sense QSK

Default: **Off** · Off / On · Only shown in F-Sense mode.

| | **Off** (default) | **On** |
|---|---|---|
| TX enabled | only once F-Sense has measured the frequency **in this transmission** – until then the signal passes at transceiver power through the bypass | at once, with the last measured band |
| First transmission after a change to a higher band (e.g. 40 → 20 m) | PA never amplifies through a wrong filter | RF off after 2 ms until the band is measured |
| First transmission after a change to a lower band (e.g. 20 → 80 m) | PA never amplifies through a wrong filter | RF off after 3 ms until the band is measured; **a few ms** of poorly suppressed harmonics |
| Delay at the start of each transmission | approx. 20–40 ms without PA (longer with SSB if the speech starts quietly) | none |
| Suitable for | SSB, digital modes, CW without full QSK | CW with full break-in |

Rule of thumb: leave it **Off** unless you use CW with full break-in. With **On**, after a change
to a lower band, key briefly at low power first.

## 4.18 About

The last page. Firmware version and credits; UP/DOWN scrolls:

| Line | |
|---|---|
| `Firmware   v5.01` | Installed firmware version |
| `OH2NLT  Original` | Juha Niinikoski – original firmware |
| `OH7SV       JUMA` | Matti Hohtola – JUMA |
| `5B4AIY  to 4.01a` | Adrian Ryan – extensions and maintenance up to v4.01a |
| `DL4JC  from 4.03` | Modifications from v4.03 on |

Nothing on this page is stored.

# 5 Saving settings

The gains of all bands, the band, Auto/Manual and all user configuration pages are stored together
in the EEPROM. There are three ways to save them; all show the same prompt:

<pre class="lcd"> Save Settings?
PWR:No BAND+:Yes</pre>

| Where | Opened by | BAND+ (Yes) | PWR (No) |
|---|---|---|---|
| Normal display | **OPER held** (approx. 0.7 s, not while transmitting) | saves | settings stay active, unsaved |
| Leaving the user configuration | OPER brief | saves | restores the previous settings |
| Power off | PWR held, only if gain, band or Auto/Manual were changed | saves, then off | off without saving |

`Saved!` or `Cancelled!` confirms the choice.

# 6 Band selection and filter protection

## 6.1 Protection on a band change

- The filter relays are never switched under power. If the band changes while transmitting, RF is
  turned off first; the relays switch 20 ms after RF off, and TX stays off for 20 ms more.
- Every millisecond the input frequency is compared with the selected low-pass filter, in every
  band select mode. If it is above the filter for 2 ms, RF goes off until the band is correct.
- F-Sense mode, first 200 ms of a transmission: if the frequency is below the selected filter for
  3 ms (poor harmonic suppression, e.g. 80 m through the 20 m filter), RF goes off until the band
  has been measured.
- An invalid or unknown band (`?`, `?!`) always inhibits TX.

## 6.2 F-Sense

The RF drive signal is counted (1 ms gate) and the band is selected from 20 samples. Drive with
**100 mW – 10 W**. For a reliable first measurement on a new band, key **TUNE** or CW briefly.

- A **higher** band is selected from any signal at once – the safe direction for the amplifier.
- A **lower** band is selected from a **clean carrier** (TUNE, CW: all samples within approx. 3 %).
- Modulated signals (two-tone, noise, SSB) are counted too low by the input shaper (two-tone approx.
  85 % of the real frequency). From such a signal a lower band is only selected at the start of a
  transmission and only if even the highest sample + 20 % is below the current filter. The band is
  then the **highest amateur band** between the highest sample and + 20 %: e.g. 14 MHz two-tone
  (counted as 11.9 MHz) selects 20 m, not 30 m, and 3.7 MHz SSB selects 80 m, not 40 m. If the real
  frequency is still higher, the filter test of 6.1 turns RF off at once and the next measurement
  corrects the band.
- Once the band has been measured in a transmission, it is only increased, never lowered – SSB
  speech cannot switch the filter down in mid-transmission.
- If the new frequency is only just below the current filter (e.g. 10.1 MHz noise from the 20 m
  filter), the amplifier stays on the higher filter; set the band once with TUNE.

The frequency page (DISPLAY, F-Sense only) shows the counted frequency; with modulated signals it
reads too low.

# 7 Service and calibration mode

From the off state, hold **PWR** until a beep sounds and

<pre class="lcd">  Calibration
  Mode  v5.01</pre>

appears. Release PWR. Pages: DISPLAY brief/held forward, PWR brief/held back. Values: UP/DOWN.
**OPER** opens the save prompt (BAND+ saves, PWR restores the previous calibration).

Connect a dummy load. In this mode the amplifier starts in Operate with the minimum gain (G1, 5 W
drive gives approx. 40–50 W) – the band gains are restored on exit. It can
only be keyed on the ammeter and RF power pages, which also show A/M and the selected filter. A 20 m
signal is recommended. Before entering, select AUTO (filter by F-Sense) or MANUAL (10 m filter) in
the normal mode and save. Any alarm ends the service mode.

| Page | Setting | Default | Range |
|---|---|---|---|
| 0 | Supply (voltmeter factor) | 5250 | 4750 – 5750 |
| 1 | Ammeter factor | 2520 | 1500 – 4000 |
| 2 | RF power factor (high power) | 1062 | 750 – 1500 |
| 3 | RF power offset (low power) | 120 | 0 – 150 |
| 4 | Beep Len | 50 ms | 0 (off) – 100 ms |
| 5 | **Beep Tone** | JUMA | JUMA / RS-928 |
| 6 | Power Averaging | 1 (off) | 1 – 16 |
| 7 | Overvoltage alarm | On | On / Off |
| 8 | Overvoltage Trip | 14.50 V | 14.0 – 15.0 V |
| 9 | Low Voltage alarm | On | On / Off |
| 10 | Low Voltage Trip | 11.00 V | 10.5 – 11.5 V |
| 11 | Pre-Limit Trip | 11.20 V | Low Voltage Trip + 0.1 … + 0.8 V |
| 12 | Full-Scale Power (bar graph) | 100 W | 40 – 160 W |
| 13 | Frequency meter factor | 999985 | 999625 – 1000345 |
| 14 | Splash Screen | On | On / Off |

Pages 8, 10 and 11 are skipped when the corresponding alarm is off.

## 7.1 Voltmeter

Measure the supply at the amplifier's power connector with an accurate 4-digit multimeter and adjust
the factor until the display agrees (±1 digit). The display here is not averaged (in normal
operation it is a 50-sample average), which helps to find the most stable setting.

## 7.2 Ammeter

Drive to approx. 30–80 W CW, wait for the reading to settle, and adjust to a reference (clip-on DC
ammeter, shunt, or an accurate power supply meter). Expect approx. ±0.2 A at full power (shunt
contact resistance and heating). In normal operation the ammeter is a peak-hold meter (approx. 1 s).

## 7.3 RF power (high power)

Drive to 50–100 W CW into a dummy load and adjust to an accurate power meter (or a directional
coupler and oscilloscope). SWR meters with a power scale are usually not accurate enough. Without
reference equipment keep the default.

## 7.4 RF power (low power)

At approx. 4–5 W output on 20 m adjust the offset. Switch between pages 2 and 3 (PWR brief) with the
corresponding drive levels until both agree. If you end up near a limit, reset to approx. 1060 /
120 and start again. Expect approx. ±10 %; the lowest displayed power is approx. 0.4 W.

## 7.5 Beep Len

0 switches the beeper off; important acknowledgements (e.g. entering the menus) still beep.

## 7.6 Beep Tone

| Setting | Tones |
|---|---|
| **JUMA** (default) | unchanged: 601, 784, 934, 1397 Hz, alarm 2000 Hz |
| **RS-928** | 2300, 2450, 2600, 2750 Hz, alarm 2700 Hz |

The RS-928 buzzer only sounds clean between approx. 2300 and 2800 Hz. A changed setting is played at
once.

## 7.7 Power Averaging

Averages the power and SWR measurement over 1–16 samples (approx. 4 ms each). The SWR protection
in the interrupt averages over at least 8 ms and starts 20 ms after the beginning of a transmission,
so normal relay switching does not cause false trips. Leave at **1** unless an external linear or
separate RX/TX antennas cause transient SWR alarms; more than 6–10 samples point to a problem
elsewhere.

## 7.8 Voltage alarms

- **Overvoltage:** for mobile use; alternator spikes or poor connections can trigger it.
- **Low Voltage:** final limit, e.g. for lead-acid batteries (do not discharge below 10.5 V).
- **Pre-Limit:** an early warning above the Low Voltage Trip. If the Low Voltage Trip is changed,
  the pre-limit follows by the same amount.

## 7.9 Frequency meter

Feed 100 mW – 5 W of a CW signal on an exact kHz frequency in the 10 m band (e.g. 28.850 MHz). Step
the factor up until the display just reads one kHz less without jitter, note it; step down until it
just reads one kHz more, note it. The mean of both (integer) is the factor.

## 7.10 Splash Screen

Off: the start-up screen is skipped and the amplifier starts almost at once.

# 8 Serial port

## 8.1 Serial test suite

*Serial Port = Test* (modes F-Sense, FT817/818, Xiegu, Manual), terminal 8N1 at the set speed.

| Key | Function |
|---|---|
| `H`, `?` | Help |
| `A` | A-D channel dump (current, voltage, band voltage, reverse/forward power, temperature) |
| `B` | Alarm test (0 = all off, 1 O/C, 2 SWR, 3 temperature, 4 over-voltage, 5 pre-limit, 6 low voltage, 7 all on) |
| `C` | LCD bar graph and character test |
| `D` | Clear the factory-default reset counter |
| `E` | Dump calibration and user settings – keep a copy before a firmware update |
| `F` | EEPROM dump |
| `G` | F-Sense test on/off: each sample set with bins, lowest/highest sample, `clean`/`mod`/`far` |
| `I` | Input attenuator (gain) of each band |
| `J` | Temperature sensor calibration (fast display, any key ends) |
| `K` | Buzzer sound test (frequency and duration) |
| `L` / `M` | Write ASCII / hex to the LCD |
| `Z` | Divide-by-zero trap test (RF off, fan on) |

**Temperature sensor:** the BD139 (Q3) on the heat sink is both the sensor and the bias reference.
Let the amplifier reach room temperature, measure the heat sink with an accurate thermometer, and
adjust the potentiometer on the control board using `J`.

## 8.2 Cables and levels

| Signal (PC, DB-9) | Pin | 3.5 mm plug on the PA-100D |
|---|---|---|
| RX data (from the PA) | 2 | Ring |
| TX data (to the PA) | 3 | Tip |
| Ground | 5 | Sleeve |

On the **RS-928 tip and ring are swapped** on the serial and the PTT socket. The Elecraft KX3 ACC1
uses 0 V / +12 V instead of true RS-232 levels; the PA accepts them, some USB adapters do not.

## 8.3 KX2/KX3 and other ASCII transceivers

The PA uses the `FA` frequency answer, e.g. `FA00014175000;` (11 digits, Hz – Elecraft/Kenwood) or
`FA014175000;` (9 digits – Yaesu FT-991, FTDX10) or `FA14175000;` (8 digits – Yaesu FT-450/950/2000, FTDX1200/3000/5000). Other lengths and corrupted messages are ignored; a new
message always starts with `F`.

KX3 setup:

1. KX3 RF output → PA RF input.
2. KX3 **ACC2** → PA **T/R**: the keying transistor is on the ring of the 2.5 mm plug and goes to the
   tip of the 3.5 mm plug; sleeve to sleeve. **Do not connect the tip of the 2.5 mm plug.**
3. KX3 **ACC1** → PA **RS-232** with a tip/ring crossed cable (or set the PA's internal jumpers to the
   update position and use a straight cable).
4. PA: *Auto Band Detect = KX2/KX3*, *Serial Speed* as on the KX3 (KX3 default 4800).
5. Either KX3 menu **AUTOINF = ANT CTRL** (the KX3 sends `FA` on every change; polling may be off),
   or leave polling on (2 s; 1 s responds faster).

With polling off, switch on the KX3 first and then the PA: the PA sends one `FA;` query at power-on
(the KX3 sends nothing on its own when switched on). Any manual band change or A/M change also sends
a query.

KX2: same, through the TRRS ACC socket (keyline and serial data; e.g. Elecraft KX2ACBL breakout).

Test without a radio: connect a terminal and type e.g. `FA007000000;` (9 digits) – the 40 m filter
is selected.

## 8.4 Xiegu band voltages

Input as for the FT-817 band voltage. Thresholds midway between the levels, ±115 mV tolerance:

| Band | 160 m | 80 m | 60 m* | 40 m | 30 m | 20 m | 17 m | 15 m | 12 m | 10 m |
|---|---|---|---|---|---|---|---|---|---|---|
| Voltage | 0.23 V | 0.46 V | 0.69 V | 0.92 V | 1.15 V | 1.38 V | 1.61 V | 1.84 V | 2.07 V | 2.30 V |

\* 60 m uses the 40 m filter. Below 115 mV = out of band, above 2.415 V = unknown; both inhibit TX.
Brick2/3 and SquareSDR deliver the same XPA125 levels.

FT-817/818 band voltages (mode FT817/818): 0.33 V 160 m, 0.67 V 80 m, 1.00 V 40 m, 1.33 V 30 m,
1.67 V 20 m, 2.00 V 17 m, 2.33 V 15 m, 2.67 V 12 m, 3.00 V 10 m.

## 8.5 HR50 mode

*Auto Band Detect = HR50*: the PA behaves like a HobbyPCB Hardrock-50 on the serial port. Programs
and transceivers with HR50 support can select the band, switch Operate/Standby and read the status.
The port then speaks only HR50: remote control, serial test and polling are off. Set *Serial Speed*
to the host (the HR50 USB port defaults to 19200).

| Command | Function |
|---|---|
| `FAxxxxxxxxxxx;` | Frequency in Hz, selects the band. Kenwood `IF…;` data is handled the same way. |
| `HRBNn;` / `HRBN;` | Set / read band: 0 = 6 m, 1 = 10 m, 2 = 12 m, 3 = 15 m, 4 = 17 m, 5 = 20 m, 6 = 30 m, 7 = 40 m, 8 = 60 m, 9 = 80 m, 10 = 160 m, 99 = unknown |
| `HRMDn;` / `HRMD;` | 1 = PTT (Operate), 0 = OFF (Standby). 2 (COR) and 3 (QRP) select Standby. |
| `HRRX;` | Status, e.g. `RX,PTT,20M,27C,13.8V;` |
| `HRTP;` / `HRVT;` | Temperature `HRTP27C;` / supply voltage `HRVT13.8V;` |
| `HRAT;` / `HRKX;` / `HRBR;` | `HRAT0;` (no ATU) / `HRKX0;` / serial speed 0–3 (4800–38400); below 4800 the answer is 0, above 38400 (57600, 115200) it is 3 – the HR50 protocol knows no higher speeds |
| `HRTM…;` | ATU pass-through, answer `HRTM;` as an HR50 without ATU |

Commands end with `;`, upper or lower case; answers end with `;\r\n`. SET commands are not answered.
The band is kept until the host sends a new one. 6 m and "unknown" inhibit TX. `HRBR`, `HRTP` and
`HRKX` are read-only; set speed and temperature scale in the menu.

## 8.6 Remote control

*Serial Port = Remote* (modes F-Sense, FT817/818, Xiegu, Manual). Command format:
`=<letter>[digit]` followed by CR.

| Command | Function |
|---|---|
| `=A` | Automatic band selection |
| `=Bn` | Manual band, n = 1 (160 m) … 9 (10 m) |
| `=C` | Clear alarm |
| `=Gn` | Gain n = 1 … 4 (applied after the transmission if sent during TX) |
| `=O` | Operate |
| `=Pn` | Power off; n = 1 saves the current settings, n = 0 or no digit does not save. RF is turned off first. |
| `=R` | Request status |
| `=S` | Standby |

Status reply, e.g. `O:A:T:C: 5:1:1.0:14.09: 8.1: 27.2: 26:0: 0` followed by LF CR. Fields:

| # | Value | Meaning |
|---|---|---|
| 1 | O / S | Operate / Standby |
| 2 | A / M | Automatic / manual band selection |
| 3 | T / R | Transmit / receive |
| 4 | C / F | Temperature scale |
| 5 | nn | Band 1 (160 m) … 9 (10 m), 10 = unknown |
| 6 | n | Gain 1 – 4 (G1 = −6 dB … G4 = 0 dB) |
| 7 | n.n | SWR |
| 8 | nn.nn | Supply voltage, V |
| 9 | nn.n | Current, A |
| 10 | nnn.n | Output power, W |
| 11 | nnn | Temperature |
| 12 | n | Fan 0 = off, 1 = low, 2 = medium, 3 = high |
| 13 | HH | Alarms, hexadecimal: bit 0 SWR, 1 over-current, 2 temperature, 3 over-voltage, 4 pre-limit, 5 low voltage |

The reply is formatted completely and sent in one piece.

**Remote time-out:** with polling off, a remotely commanded Operate returns to Standby if no command
arrives for 5 s – poll at least every few seconds (1 s recommended). With polling on, the amplifier
sends a status message at every interval and never times out; a failed remote host is then not
detected (as in v4.01a). Operate selected at the front panel does not time out.

**USB-serial adapters** split lines according to their latency timer (FTDI: 16 ms). Programs should
assemble lines up to LF CR. Under Windows the latency timer can be set to 1 ms (Device Manager →
COM port → Advanced).

**Cautions:** always **fuse the power leads**. Remote power-off does not disconnect the PA module from
the supply; only the fuse protects against a shorted transistor. Jumper **J4** on the control board
makes the PA switch on as soon as power is applied (for fully remote operation with a remotely
switched power supply – use with great care). Request the status periodically.

# 9 Alarms

| Alarm | Display | Cause | Effect |
|---|---|---|---|
| High SWR | `SWR` | SWR above the trip limit, Operate only | RF off, Standby |
| Over-current | `O/C` | Hardware trip of the MAX4373 at 24 A (latched) | RF off at once (also read directly in the 1 ms interrupt), Standby |
| Over-temperature | `TEMP` | Heat sink above the limit | RF off, Standby; fan at full speed |
| Over-voltage | `Hi-V` | Supply above the trip (if enabled) | RF off, Standby |
| Low voltage | `Lo-V` | Supply below the final limit (if enabled) | RF off, Standby |
| Pre-limit | `Batt` | Supply below the pre-limit (if Low Voltage is enabled) | Warning: no Standby, but TX stays off until acknowledged with PWR. Occurs once per power-on. |

The alarm flashes and a loud beep sounds. A brief **PWR** clears the most urgent alarm first, then
the next. DISPLAY, OPER, AUTO and BAND± are inactive while an alarm is shown, and TX stays off. If an over-current alarm
cannot be cleared, switch off at once and look for the fault. The low voltage alarm can only be
cleared once the voltage has risen again. Recharge batteries as soon as possible – a discharged
lead-acid battery sulphates.

Test the alarms with the serial test (`B`) or, for SWR, with *SWR Trip = 1.0* into a dummy load.

# 10 Firmware update and going back

The PA-100D has the **Ingenia boot loader**. A firmware file is loaded over the serial port with
the Ingenia loader (Windows) or with `tools/juma-flash.py` (Windows, macOS, Linux). Full
instructions: README in the repository (github.com/jcmerg/juma-pa100d-firmware).

1. Check the serial connection first: *Serial Speed* 115200, *Serial Port* Test, type `H` and `F`
   several times – the output must be error-free. Save the `E` dump.
2. Switch off. **Hold OPER and press PWR**: the boot loader starts.
3. Load `firmware/Juma PA-100D v5.01.hex` – program flash only, **never** the data EEPROM or the
   configuration registers. With `juma-flash.py`:
   `python3 tools/juma-flash.py --port COM3 "firmware/Juma PA-100D v5.01.hex"`
4. Disconnect the power supply (PWR does not work in the boot loader), then switch on normally.

**EEPROM:** the original configuration and calibration blocks are unchanged – no checksum error, the
calibration is kept. New settings (F-Sense QSK, Beep Tone, Xiegu/HR50) are in a separate extension
block.

**Going back:** load `firmware/Juma PA-100D v4.01a Build 3 (original).hex` the same way. The
calibration and all original settings are kept; Xiegu becomes F-Sense, HR50 becomes KX2/KX3, F-Sense
QSK is ignored.

**RS-928:** delivered without the boot loader. It must be programmed once with a PICkit through
header J19 (keep PWR pressed during programming), see the README.

# 11 Quick reference

| Button | Brief press | Hold |
|---|---|---|
| **PWR** | Power on · cancel alarm · previous page · **No** in prompts | Power off (with save prompt) · from off: **service mode** · in menus: pages back |
| **UP / DOWN** | Gain (normal) · value (menus) | Repeat |
| **DISPLAY/CONFIG** | Next measurement page · next menu page | **User configuration** · in menus: pages forward · from off (with PWR): RS-232 loop-back test |
| **OPER** | Operate/Standby (on release) · save & exit in menus | **Save prompt** (normal display, not in TX) · from off (with PWR): **boot loader** |
| **AUTO** | Auto / Manual band selection | – |
| **BAND+** | Higher band · **Yes** in prompts | Repeat · from off (with PWR): factory defaults |
| **BAND−** | Lower band | Repeat · from off (with PWR): HR50 message time-out 5 s |

# 12 Settings record

**User configuration**

| Setting | Default | Your setting |
|---|---|---|
| Auto Band Detect | F-Sense | |
| Serial Speed | 9600 Baud | |
| Serial Port | Off | |
| Polling Interval | 2 s | |
| Backlight | 300 | |
| Contrast | 2000 | |
| SWR Trip | 3.0 | |
| Fan Control | Normal | |
| Temperature | Celsius | |
| Overtemp | 70 °C | |
| Fan Cut-In Temp | 40 °C | |
| Band Display | MHz | |
| Graphical Limits | Off | |
| Graphic Display | Original | |
| RF Power Meter | Watts | |
| Start-Up Display | O/P Power | |
| F-Sense QSK | Off | |
| Band select / band / gain | Manual, 10 m, G1 on all bands | |

**Calibration**

| Setting | Default | Your setting |
|---|---|---|
| Voltmeter | 5250 | |
| Ammeter | 2520 | |
| RF power factor | 1062 | |
| RF power offset | 120 | |
| Beep Len | 50 ms | |
| Beep Tone | JUMA | |
| Power Averaging | 1 | |
| Overvoltage / Trip | On / 14.50 V | |
| Low Voltage / Trip | On / 11.00 V | |
| Pre-Limit | 11.20 V | |
| Full-Scale Power | 100 W | |
| Frequency meter | 999985 | |
| Splash Screen | On | |

# 13 Credits

- Original firmware: **Juha Niinikoski, OH2NLT**, and **Matti Hohtola, OH7SV** (JUMA)
- Extensions, maintenance and the v4.00a manual: **Adrian Ryan, 5B4AIY**
- Modifications from v4.03 and this manual: **DL4JC**
- Ingenia dsPIC boot loader: Ingenia-CAT S.L., adapted by OH2NLT

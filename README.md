# JUMA PA-100D Firmware – v4.02a (DL4JC)

[Deutsch](README.de.md) | English

Modified firmware for the **JUMA PA-100D** HF linear amplifier (dsPIC30F6014A, also runs on the
[RS-928 clone](#rs-928-clone)), based on
**v4.01a Build 3** by Adrian Ryan, 5B4AIY. The original firmware was written by Juha Niinikoski,
OH2NLT, and Matti Hohtola, OH7SV.

This version focuses on **TX protection** (amplifier and low-pass filters), fixes several bugs that
were also present in v4.01a, adds a **Xiegu** band voltage mode and a **Hardrock-50** compatible
serial mode, and can be built with the current Microchip **XC16** compiler.

> **Not an official JUMA release.** You use this firmware at your own risk. Loading firmware that
> is not from JUMA may void the manufacturer's warranty and support. Test every new build with a
> dummy load and low power first. You can return to the original v4.01a at any time, see
> [Going back to the original firmware](#going-back-to-the-original-firmware).

---

## Contents

- [What's new](#whats-new)
- [Operation – new settings](#operation--new-settings)
- [EEPROM and compatibility](#eeprom-and-compatibility)
- [Flashing with the Ingenia boot loader](#flashing-with-the-ingenia-boot-loader)
- [RS-928 clone](#rs-928-clone)
- [Building the firmware](#building-the-firmware)
- [Repository layout](#repository-layout)
- [Known limitations](#known-limitations)
- [Copyright and credits](#copyright-and-credits)

---

## What's new

### TX protection

| Change | Effect |
|---|---|
| **Protection runs in the 1 ms interrupt** (`tx_guard()` in `timers_pwm.c`) | In v4.01a all protection was in the main loop and stopped whenever the main loop waited, e.g. while a button was held, at the save prompt or during serial test commands. TX stayed on in the meantime. |
| **KEY release** | RF off 2 ms after KEY becomes inactive, wherever the main loop is. |
| **SWR in the interrupt** | The A-D conversions now run in the 1 ms interrupt. The SWR is averaged over *Power Averaging* × 4 ms (at least 8 ms) and tested 20 ms after TX starts, which gives fast and reliable shutdown without nuisance trips. |
| **Main loop watchdog** | RF off if the main loop has not run for 2 s. |
| **User configuration mode** | TX_ON is forced off. In v4.01a it stayed in the state it had when the menu was entered. |
| **Trap handlers** | On a processor fault (address, stack or math trap) the RF is turned off and the fan is switched on before anything else. |
| **Service mode** | An alarm now really exits the service mode. In v4.01a the device hung in the loop. |

### Low-pass filter protection (band change)

| Change | Effect |
|---|---|
| **No relay switching under power** | On a band change RF is turned off first. The filter relays switch once the PA relays have released, and TX is held off for 20 ms until the relays have settled. |
| **Frequency above the filter** | If the input frequency measured by F-Sense is above the selected filter for 2 ms, RF goes off until the band has been corrected. This fixes the **O/C alarm** of v4.01a on the first transmission after a band change in F-Sense mode, e.g. 40 m → 20 m, which happened because 20 m was amplified through the 40 m filter. Works in every band select mode. |
| **Frequency below the filter** (F-Sense) | In the first 200 ms of a transmission: 3 ms below the selected filter means RF off until the band has been measured (e.g. 80 m through the 20 m filter, poor harmonic suppression). |
| **F-Sense QSK switch** | New menu page, see [below](#f-sense-qsk). |

### Band select

| Change | Effect |
|---|---|
| **Xiegu mode** (new) | Band voltages from the Xiegu ACC port or other XPA125 band level sources such as Brick2/3 and SquareSDR (230 mV steps, including 60 m). See [table](#xiegu-band-voltages). |
| **HR50 mode** (new) | The PA answers the serial commands of the HobbyPCB Hardrock-50: band, Operate/Standby and status from programs and transceivers with HR50 support. See [Hardrock-50 mode](#hardrock-50-mode). |
| **KX2/KX3 (ASCII)** | Frequencies above 30 MHz are limited before the 16-bit conversion. In v4.01a, for example, 144 MHz wrapped to 12.9 MHz and selected the **20 m filter with TX enabled**. |
| **Juma TRX-2** | An invalid band from the TRX-2 now means "unknown" (TX inhibited) instead of 10 m. |
| **Menu order** | Yaesu CAT → KX2/KX3 → **HR50** → Juma-TRX2 → F-Sense → FT817/818 → **Xiegu** → Manual |

### Serial interface

| Change | Effect |
|---|---|
| **Receive overrun** | The receive interrupt now reads the whole UART FIFO and an overrun is cleared automatically. In v4.01a reception could stop for good at higher baud rates (e.g. 115200) after a short burst, while transmission carried on. |
| **Status line in one piece** | The status reply (`=R` / polling) is formatted completely and then sent in one go. Previously gaps between the fields could split the line, and JUMA_CTRL then showed `???`. The format is unchanged. |

### Other

- Builds with **MPLAB XC16 v2.10**, no compiler warnings. The configuration bits are identical to the original HEX file. The C30 project files are still included.
- Linker script: program memory ends **below the boot loader** (0x17D00). The build script refuses to produce a HEX file containing data in the boot loader area.
- **EEPROM extension block** for the new settings, see [EEPROM](#eeprom-and-compatibility).
- Service menu **Beep Tone**: tones in the clean range of the RS-928 buzzer, see [Beep Tone](#beep-tone-service-menu).
- Start-up screen: `JUMA PA100v4.02a` / `OH2NLT/7SV DL4JC`.

The complete technical change history is in the comment header of `juma-pa100.c` (section *DL4JC Modifications*).

---

## Operation – new settings

### F-Sense QSK

User configuration, last page **"F-Sense QSK"**. The page only appears when *Auto Band Detect = F-Sense*.

Compared with the F-Sense mode of the original v4.01a:

| | v4.01a (original) | F-Sense QSK **Off** (default) | F-Sense QSK **On** |
|---|---|---|---|
| **TX enable** | immediately, with the last measured band | only once F-Sense has measured the frequency **in this transmission**; until then the signal passes through the bypass at the transceiver's power | immediately, with the last measured band |
| **First transmission after a change to a higher band** (e.g. 40 → 20 m) | PA amplifies through the 40 m filter until measured → **O/C alarm** | PA never amplifies through the wrong filter | RF off after 2 ms until the band has been measured |
| **First transmission after a change to a lower band** (e.g. 20 → 80 m) | PA amplifies through the 20 m filter until measured → poorly suppressed harmonics | PA never amplifies through the wrong filter | RF off after 3 ms until the band has been measured; **a few ms** of poorly suppressed harmonics |
| **Filter relays on a band change** | switched under full power | switched without RF, then TX held off for 20 ms | switched without RF, then TX held off for 20 ms |
| **Delay at the start of each transmission** | none | approx. 20–40 ms without the PA (longer with SSB if the speech starts quietly) | none |
| **Suitable for** | – | SSB, digital modes, CW without full QSK | CW with full QSK |

Rule of thumb: leave it **Off** unless you use CW with full break-in. With **On**, after a change to a lower band, key briefly at low power first.

### Xiegu band voltages

Input as for the FT-817 band voltage. Thresholds midway between the levels, tolerance ±115 mV:

| Band | 160 m | 80 m | 60 m* | 40 m | 30 m | 20 m | 17 m | 15 m | 12 m | 10 m |
|---|---|---|---|---|---|---|---|---|---|---|
| Voltage | 0.23 V | 0.46 V | 0.69 V | 0.92 V | 1.15 V | 1.38 V | 1.61 V | 1.84 V | 2.07 V | 2.30 V |

\* 60 m uses the 40 m filter. Below 115 mV the band is "out of band", above 2.415 V it is "unknown"; both inhibit TX.

Brick2/3 and SquareSDR output the same XPA125 band levels and also use the **Xiegu** mode.

### Hardrock-50 mode

*Auto Band Detect = HR50*: the PA behaves like a HobbyPCB Hardrock-50 on the serial port. Programs
and transceivers that support the HR50 can select the band, switch between Operate and Standby, and
read the status. The serial port then only speaks HR50: the JUMA remote protocol, the serial test
and polling are off, and the *Serial Port* and *Polling Interval* pages are hidden. Set *Serial
Speed* to the speed of the host (the HR50 USB port defaults to 19200).

| Command | Function |
|---|---|
| `FAxxxxxxxxxxx;` | Frequency in Hz, selects the band. Kenwood `IF…;` data is used the same way. |
| `HRBNn;` / `HRBN;` | Set / read the band: 0 = 6 m, 1 = 10 m, 2 = 12 m, 3 = 15 m, 4 = 17 m, 5 = 20 m, 6 = 30 m, 7 = 40 m, 8 = 60 m, 9 = 80 m, 10 = 160 m, 99 = unknown |
| `HRMDn;` / `HRMD;` | 1 = PTT (Operate), 0 = OFF (Standby). 2 (COR) and 3 (QRP) select Standby. |
| `HRRX;` | Status, e.g. `RX,PTT,20M,27C,13.8V;` |
| `HRTP;` / `HRVT;` | Temperature `HRTP27C;` / supply voltage `HRVT13.8V;` |
| `HRAT;` / `HRKX;` / `HRBR;` | `HRAT0;` (no ATU) / `HRKX0;` / serial speed 0–3 (4800–38400) |
| `HRTM…;` | ATU pass-through, answered with `HRTM;` as by an HR50 without ATU |

- Commands end with `;`, upper or lower case. Replies end with `;\r\n`. As with the HR50, SET
  commands are not answered.
- The band is held until the host sends a new one; the PA never polls the host. 6 m and "unknown"
  inhibit TX, 60 m uses the 40 m filter. The filter protection works as in the other modes.
- `HRBR`, `HRTP` and `HRKX` can only be read. Serial speed and temperature scale are set in the menu
  (the temperature scale also sets the alarm and fan limits).

### Beep Tone (service menu)

Service menu (from the off state hold **PWR** until *Calibration Mode* appears), page **"Beep Tone"**
after *Beep Len*, set with UP/DOWN, save with OPER. A changed setting is played at once.

| Setting | Tones |
|---|---|
| **JUMA** (default) | unchanged: 601, 784, 934, 1397 Hz, alarm 2000 Hz |
| **RS-928** | 2300, 2450, 2600, 2750 Hz, alarm 2700 Hz – for the buzzer of the [RS-928](#buzzer) |

The order of the tones is kept. (The tone names in the source code, e.g. `HZ466_85`, are one octave
lower than the tones actually played.)

### Remote control

Unchanged (`=A`, `=Bn`, `=C`, `=Gn`, `=O`, `=Pn`, `=R`, `=S`; status `O:M:R:C: 5:4:0.0:13.81: 0.0:  0.0: 24:0: 0`).
For the known limitation with polling enabled see [Known limitations](#known-limitations).

---

## EEPROM and compatibility

- The original **configuration and calibration blocks are unchanged**. There is **no checksum
  error** when loading this firmware, and calibration and settings are kept.
- The new settings live in an **extension block at EEPROM address 0xF100** with its own identifier,
  version and CRC. If it is missing (first start) or invalid, only the new settings get their
  default values.
- **Xiegu mode** is stored in the original block as *F-Sense*, and only in the extension block as
  *Xiegu*. **HR50 mode** is stored in the original block as *KX2/KX3*.
- **Beep Tone** is only stored in the extension block. It is also saved with the service settings.

### Going back to the original firmware

Possible at any time, without preparation and without losing the calibration:

| Setting in this firmware | What the original v4.01a does with it |
|---|---|
| Calibration, all original settings | taken over unchanged |
| Xiegu | F-Sense (band selected by frequency measurement, works with any transceiver) |
| HR50 | KX2/KX3 (band from the `FA` frequency data, polling off); the `HR…` commands are ignored |
| F-Sense QSK | ignored |

The original HEX file is included in the repository: `firmware/Juma PA-100D v4.01a Build 3 (original).hex`.
Note: the original F-Sense mode still has the O/C problem described above.

---

## Flashing with the Ingenia boot loader

The PA-100D has the **Ingenia dsPIC boot loader** in the upper flash memory (0x17D00–0x17FFE).
The firmware is loaded over the serial port from a Windows PC. Based on the JUMA document
*"Firmware Updating for the JUMA TRX2 & PA100D"* (5B4AIY).

### 1. Serial cable

| Signal (PC) | DB-9 pin | 3.5 mm stereo plug (PA-100D) |
|---|---|---|
| RX data | 2 | Ring (TX data of the PA) |
| TX data | 3 | Tip (RX data of the PA) |
| Ground | 5 | Sleeve |

The TRX-2 cable (Tip ↔ pin 2, Ring ↔ pin 3) can be used if the amplifier's internal jumpers are set
to *PROGRAM*, or with a null-modem adapter. USB-serial adapters with an **FTDI** chipset are reliable;
some other adapters do not work at 115200 baud.

### 2. Check the serial port first – do not skip this

If the connection is not reliable, the boot loader can be corrupted, and recovery then needs a
programmer (see [Recovery](#recovery-with-a-programmer)).

1. On the PA: User configuration → *Serial Speed* **115200**, *Serial Port* **Test**, save.
   (The Serial Port page only appears for F-Sense/FT817/Xiegu/Manual.)
2. Terminal program on the PC: 115200 baud, 8N1.
3. Type `H` → help text, type `F` → EEPROM dump. Repeat several times; the output must be identical
   and error-free.
4. **Save the settings:** type `E` and keep the output (calibration and configuration).

### 3. Install the Ingenia loader

1. Download [`ingeniadsPICbootloader1.1.zip`](https://www.jumaradio.com/juma-trx2/bootloader/ingeniadsPICbootloader1.1.zip) from JUMA and install the
   `ingeniadsPICbootloader.exe` it contains (Ingenia dsPIC bootloader 1.1). The loader is not included in this
   repository, as its license does not allow redistribution.
   SHA-256 of the ZIP: `c5664e750d726a0b6049d24004fbc1d6d43b58fd5b68fd37137e2d8486ca9449`
2. **Replace the device file:** copy [`tools/ingenia/ibl_dspiclist.xml`](tools/ingenia/ibl_dspiclist.xml)
   into the installation folder and overwrite the existing file, typically
   `C:\Program Files\Ingenia\ingeniadsPICbootloader\` (64-bit Windows: `C:\Program Files (x86)\...`).
   It contains the dsPIC30F6014A with the boot loader area 0x17D00–0x17FFE. Without it the device is
   not recognised, or the tool does not know the protected area.
3. From Windows Vista on: icon → Properties → Compatibility → *Windows XP (Service Pack 3)* and
   *Run this program as an administrator*.

### 4. Flash the firmware

1. Connect the PA, **switch it off**.
2. Start Ingenia → *OK, my platform is shut down*.
3. Choose the COM port and baud rate (115200, or lower if there are problems) → *configuration done*.
4. **Press and hold OPER, then press PWR.** The boot loader checks OPER right at power-on, switches
   the power latch on itself and shows that the flash writer has started. Then release both buttons.
   (The note "keep PWR pressed" in the old TRX-2 instructions predates the power latch, which the
   boot loader has had since 23.01.2007, see `iBL.s` / `mini_lcd-trx2.c`.)
5. Wait until *dsPIC6014A detected, firmware version 1.1* appears → OK.
6. *open HEX file* → select `firmware/Juma PA-100D v4.02a Build 5-DL4JC.hex` (also attached to the
   [latest release](https://github.com/jcmerg/juma-pa100d-firmware/releases/latest)).
7. Only **"program flash"** may be ticked. **"write data EEPROM" and "configure registers" must not
   be ticked.** There must be no error message (see below).
8. *start write* → takes approx. 10–15 s at 115200 baud → *write completed*.
9. Close Ingenia, **disconnect the power supply** (the PWR button does not work in the boot loader),
   then switch on normally.

**"Your hex file contains data in bootloader addresses"**: do not flash this file. Release builds
of this repository are checked against it by the build script.

**Disturbances:** close all programs that are not needed during flashing (virus scanner, network
tools). Errors while writing almost always come from the serial connection: check the cable,
try a lower baud rate.

### Alternative: juma-flash.py (Windows, macOS, Linux)

[`tools/juma-flash.py`](tools/juma-flash.py) is a command-line loader written from the boot loader
source `bootloader/iBL.s`. It should also work with other JUMA devices that use the Ingenia boot loader (e.g. TRX-2),
but has only been tested with the PA-100D. It replaces steps 3 and 4 and does not need Ingenia, the device file or
administrator rights. Requirements: Python 3 and pyserial (`pip install pyserial`). Cable and the
serial port check (steps 1 and 2) are the same.

```
python3 tools/juma-flash.py --port COM3 "firmware/Juma PA-100D v4.02a Build 5-DL4JC.hex"
```

(macOS/Linux: e.g. `--port /dev/cu.usbserial-XXXX` or `/dev/ttyUSB0`; without `--port` the available
ports are listed.) Start the command with the PA switched off, then **press and hold OPER and press
PWR**. The tool detects the boot loader and the dsPIC30F6014A, writes the firmware, and reads it back
for verification. Then disconnect the power supply and switch on normally.

- Only the program memory is written, never the configuration registers or the data EEPROM, so the
  calibration is kept. HEX files with data in the boot loader area are refused.
- The boot loader address is read from the device's reset vector (PA-100D: 0x17D00) and this area is
  never written. The reset vector always keeps pointing to the boot loader, so it stays reachable even
  if flashing is interrupted. In that case simply flash again.
- `--dry-run` only checks the HEX file, `--verify-only` compares the flash with the HEX file,
  `--baud` sets a lower speed (default 115200).

### Recovery with a programmer

If the boot loader is damaged, `bootloader/PA100_boot_loader.hex` must be loaded with a programmer
(ICD/PICkit) through the ICD header. After that the firmware can be loaded with Ingenia or `juma-flash.py` again.
The boot loader source code (`iBL.s`, modified for JUMA by OH2NLT) is in [`bootloader/`](bootloader/).

---

## RS-928 clone

This firmware also runs on the **RS-928**, a PA-100D clone whose hardware is practically the same as
the original. Three things differ:

### First installation: PICkit through header J19

The RS-928 ships with its own firmware (v1.05q), **without the Ingenia boot loader** and without a
usable remote control. The serial update therefore only works after a one-off programming step:

1. With a **PICkit** (or another dsPIC programmer) on header **J19**, write
   [`bootloader/Bootldr_Juma-PA100_v104.hex`](bootloader/Bootldr_Juma-PA100_v104.hex) to the
   dsPIC30F6014A. This file is a complete chip image: the JUMA firmware v1.04, the Ingenia boot
   loader and the configuration bits. J19 is the 6-pin header next to the dsPIC (IC9) on the back
   of the front-panel board, see the photo *rs928ampfrontpanel.jpg* in the
   [OARC wiki](https://wiki.oarc.uk/rs928ampreview). The PICkit plugs directly onto J19: **the arrow on
   the PICkit (pin 1) goes to the framed pin of J19**. Instructions and files:
   [hermes-lite thread](https://groups.google.com/g/hermes-lite/c/breb9kSmeYc/m/xKfDIW6zEQAJ) by
   KD2NFC.

   **Power during programming – the tricky part:**
   - Supply the PA from its **own power supply**, not from the PICkit. In the PICkit software set the
     target to **self-powered** (do not let the PICkit supply VDD).
   - **Keep PWR pressed for the whole programming process.** The programmer holds the processor in
     reset, so nothing can switch the power latch on; releasing PWR switches the PA off and aborts
     the programming. (This differs from the later Ingenia update, see below.)
2. Check that the PA starts with the JUMA firmware.
3. Then load this firmware over the serial port with Ingenia, exactly as described in
   [Flashing with the Ingenia boot loader](#flashing-with-the-ingenia-boot-loader) (OPER + PWR).

The clone's original firmware is overwritten in step 1. If you might want it back, read the chip
out with the programmer first and keep the file.

### Jacks wired the other way round

On the RS-928 **tip and ring are swapped** compared with the PA-100D – on the **serial jack and on
the PTT jack**. The sleeve is ground on both.

| Sub-D pin (PC) | Signal | PA-100D | RS-928 |
|---|---|---|---|
| 3 | TxD, into the PA | Tip | **Ring** |
| 2 | RxD, out of the PA | Ring | **Tip** |
| 5 | GND | Sleeve | Sleeve |

On the **PTT jack** the RS-928 likewise has on the ring what the PA-100D has on the tip, and the
other way round. Check this before the first transmission, otherwise the amplifier does not key or
keys permanently. A swapped serial cable only results in silence (no data, no detection in Ingenia).

### Buzzer

The RS-928 buzzer has its resonance at about **2.7 kHz** and only sounds clean between about
**2300 and 2800 Hz** (measured with the sound test `K` of the serial test mode). The JUMA tones of
600–2000 Hz therefore sound harsh on the RS-928, with the original firmware as well, while they are
clean on a JUMA PA-100D. Remedy without soldering: service menu → **Beep Tone = RS-928**, see
[Beep Tone](#beep-tone-service-menu). Alternatively the buzzer can be replaced with the type used
in the JUMA.

Background: [RS-928 review in the OARC wiki](https://wiki.oarc.uk/rs928ampreview).

---

## Building the firmware

### With MPLAB XC16 (macOS, Linux, Windows)

Requirement: [MPLAB XC16](https://www.microchip.com/xc16) v2.10 (the free version is enough; the
project is built without optimisation, `-O0`, as before).

```sh
./build-xc16.sh
```

Output: `firmware/Juma PA-100D <VERSION> Build <BUILD>.hex`. Version and build number
come from `juma-pa100.h` (`VERSION`, `BUILD_NUMBER`). The script

- compiles all modules with `-mcpu=30F6014A -Wall`,
- links with `juma-trx2.gld` (boot-loader-specific: code from 0x100, program memory below 0x17D00),
- generates the HEX file and **aborts if anything ends up in the boot loader area**.

Other XC16 location: `XC16=/path/to/xc16/v2.10 ./build-xc16.sh`.

**macOS on Apple Silicon:** XC16 is an Intel program and runs under Rosetta (the script calls it with
`arch -x86_64`). The installer stops silently on Apple Silicon; start the inner installer directly:
`arch -x86_64 ".../xc16-v2.10-osx-installer.app/Contents/MacOS/osx-x86_64"` (it needs administrator
rights).

### With MPLAB C30 (original toolchain)

`Juma PA-100D.mcp` is the original MPLAB 8 / C30 project. The sources contain `#ifdef __XC16__` only
for the configuration bits; everything else is common code. C30 builds have not been tested for
this version.

---

## Repository layout

| Path | Contents |
|---|---|
| `juma-pa100.c` | Main program, menus, band select, remote control, change history |
| `timers_pwm.c` | 1 ms interrupt: frequency counter, buttons, `tx_guard()` (TX/SWR/filter protection) |
| `adc12.c` | A-D conversions in the interrupt |
| `uart.c`, `serial_pa100.c`, `serial_test.c` | Serial port, TRX-2 protocol, test suite |
| `service.c` | Calibration and service mode |
| `lcd-trx2.c`, `traps.c`, `tmr5delay.c`, `spi1.c`, `DataEEPROM.s` | LCD, trap handlers, delays, SPI, EEPROM access |
| `juma-pa100.h`, `pa100_eeprom.h` | Hardware definitions, EEPROM structures |
| `juma-trx2.gld` | Linker script for the Ingenia boot loader |
| `build-xc16.sh` | Build script for XC16 |
| `firmware/Juma PA-100D v4.02a Build *-DL4JC.hex` | DL4JC builds (the highest build number is the current one) |
| `firmware/Juma PA-100D v4.01a Build 3 (original).hex` | Original v4.01a Build 3 (to go back) |
| `tools/ingenia/ibl_dspiclist.xml` | Device file for the Ingenia loader |
| `tools/juma-flash.py` | Serial firmware loader (alternative to Ingenia) |
| `bootloader/` | Boot loader source and HEX; `Bootldr_Juma-PA100_v104.hex` = complete image for a first installation with a programmer (RS-928) |
| `Juma PA-100D.mcp/.mcw/.mcs` | Original MPLAB 8 project |

---

## Known limitations

- **F-Sense QSK = On:** after a change to a lower band, a few milliseconds of poorly suppressed
  harmonics remain, because the frequency can only be measured once RF is present. See
  [F-Sense QSK](#f-sense-qsk).
- **Remote with polling:** as in v4.01a, the remote time-out never expires while polling is
  enabled, because every automatic status message restarts it. A failed remote host is therefore
  not detected. This is kept deliberately for compatibility with existing remote programs.
- **USB-serial adapters** split lines according to their latency timer (FTDI: 16 ms). Remote
  programs should join lines up to `\n\r`. On Windows the *latency timer* can be set to 1 ms
  (Device Manager → COM port → Advanced).

---

## Copyright and credits

- Original firmware: **Juha Niinikoski, OH2NLT**, and **Matti Hohtola, OH7SV** (JUMA)
- Extensions and maintenance up to v4.01a: **Adrian Ryan, 5B4AIY**
- Modifications v4.02a: **DL4JC**
- `DataEEPROM.s`, `DataEEPROM.h`: Microchip Technology Inc. (Microchip licence, see file header)
- Ingenia dsPIC boot loader: Ingenia-CAT S.L., adapted by OH2NLT

The original sources are distributed freely by JUMA (jumaradio.com) without an explicit licence.
The copyright of the original authors remains in force; this repository does not add a licence of
its own. The modifications by DL4JC may be used under the same conditions as the original firmware.

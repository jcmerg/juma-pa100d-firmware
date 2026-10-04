# JUMA PA-100D Firmware – v4.02a (DL4JC)

[Deutsch](README.de.md) | English

Modified firmware for the **JUMA PA-100D** HF linear amplifier (dsPIC30F6014A, also runs on the
[RS-928 clone](#rs-928-clone)), based on
**v4.01a Build 3** by Adrian Ryan, 5B4AIY. The original firmware was written by Juha Niinikoski,
OH2NLT, and Matti Hohtola, OH7SV.

This version focuses on **TX protection** (amplifier and low-pass filters), fixes several bugs that
were also present in v4.01a, adds a **Xiegu** band voltage mode, and can be built with the current
Microchip **XC16** compiler.

> **Not an official JUMA release.** You use this firmware at your own risk. Test every new build
> with a dummy load and low power first. You can return to the original v4.01a at any time, see
> [Going back to the original firmware](#going-back-to-the-original-firmware).

---

## Contents

- [What's new](#whats-new)
- [Operation – new settings](#operation--new-settings)
- [EEPROM and compatibility](#eeprom-and-compatibility)
- [Flashing with the Ingenia boot loader](#flashing-with-the-ingenia-boot-loader)
- [RS-928 clone](#rs-928-clone)
- [Testing after an update](#testing-after-an-update)
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
| **Xiegu mode** (new) | Band voltages from the Xiegu ACC port (230 mV steps, including 60 m). See [table](#xiegu-band-voltages). |
| **KX2/KX3 (ASCII)** | Frequencies above 30 MHz are limited before the 16-bit conversion. In v4.01a, for example, 144 MHz wrapped to 12.9 MHz and selected the **20 m filter with TX enabled**. |
| **Juma TRX-2** | An invalid band from the TRX-2 now means "unknown" (TX inhibited) instead of 10 m. |
| **Menu order** | Yaesu CAT → KX2/KX3 → Juma-TRX2 → F-Sense → FT817/818 → **Xiegu** → Manual |

### Serial interface

| Change | Effect |
|---|---|
| **Receive overrun** | The receive interrupt now reads the whole UART FIFO and an overrun is cleared automatically. In v4.01a reception could stop for good at higher baud rates (e.g. 115200) after a short burst, while transmission carried on. |
| **Status line in one piece** | The status reply (`=R` / polling) is formatted completely and then sent in one go. Previously gaps between the fields could split the line, and JUMA_CTRL then showed `???`. The format is unchanged. |

### Other

- Builds with **MPLAB XC16 v2.10**, no compiler warnings. The configuration bits are identical to the original HEX file. The C30 project files are still included.
- Linker script: program memory ends **below the boot loader** (0x17D00). The build script refuses to produce a HEX file containing data in the boot loader area.
- **EEPROM extension block** for the new settings, see [EEPROM](#eeprom-and-compatibility).
- Start-up screen: `JUMA PA100v4.02a` / `OH2NLT/7SV DL4JC`.

The complete technical change history is in the comment header of `juma-pa100.c` (section *DL4JC Modifications*).

---

## Operation – new settings

### F-Sense QSK

User configuration, last page **"F-Sense QSK"**. The page only appears when *Auto Band Detect = F-Sense*.

| Setting | Behaviour |
|---|---|
| **Off** (default) | On **every** transmission, TX is only enabled once F-Sense has measured the frequency. Until then the signal passes through the bypass at the transceiver's power. The PA never amplifies through the wrong filter. Cost: approx. 20–40 ms without the PA at the start of each transmission (longer with SSB if the speech starts quietly). |
| **On** | TX immediately, suitable for full QSK. The filter protection above still applies. After a change from a higher to a lower band, a window of a few milliseconds remains in which harmonics are poorly suppressed. |

### Xiegu band voltages

Input as for the FT-817 band voltage. Thresholds midway between the levels, tolerance ±115 mV:

| Band | 160 m | 80 m | 60 m* | 40 m | 30 m | 20 m | 17 m | 15 m | 12 m | 10 m |
|---|---|---|---|---|---|---|---|---|---|---|
| Voltage | 0.23 V | 0.46 V | 0.69 V | 0.92 V | 1.15 V | 1.38 V | 1.61 V | 1.84 V | 2.07 V | 2.30 V |

\* 60 m uses the 40 m filter. Below 115 mV the band is "out of band", above 2.415 V it is "unknown"; both inhibit TX.

Devices using the Yaesu band voltage scheme (e.g. Brick2/3) use the **FT817/818** mode as before.

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
  *Xiegu*.

### Going back to the original firmware

Possible at any time, without preparation and without losing the calibration:

| Setting in this firmware | What the original v4.01a does with it |
|---|---|
| Calibration, all original settings | taken over unchanged |
| Xiegu | F-Sense (band selected by frequency measurement, works with any transceiver) |
| F-Sense QSK | ignored |

The original HEX file is included in the repository: `Juma PA-100D.hex` (v4.01a Build 3).
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

1. Install `ingeniadsPICbootloader.exe` (Ingenia dsPIC bootloader 1.1, included in the JUMA firmware
   packages).
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
4. **Press and hold OPER, then press and hold PWR.** The PA shows that the flash writer has started.
   Wait until *dsPIC6014A detected, firmware version 1.1* appears.
5. **Release OPER, keep PWR pressed** until the end. The PA has no power latch while in the boot
   loader; even a short release aborts the transfer.
6. *open HEX file* → select `Juma PA-100D v4.02a Build 3-DL4JC.hex`.
7. Only **"program flash"** may be ticked. **"write data EEPROM" and "configure registers" must not
   be ticked.** There must be no error message (see below).
8. *start write* → takes approx. 10–15 s at 115200 baud → *write completed*.
9. Release PWR, close Ingenia, **disconnect the power supply** (the PWR button does not work in the
   boot loader), then switch on normally.

**"Your hex file contains data in bootloader addresses"**: do not flash this file. Release builds
of this repository are checked against it by the build script.

**Disturbances:** close all programs that are not needed during flashing (virus scanner, network
tools). Errors while writing almost always come from the serial connection: check the cable,
try a lower baud rate.

### Recovery with a programmer

If the boot loader is damaged, `bootloader/PA100_boot_loader.hex` must be loaded with a programmer
(ICD/PICkit) through the ICD header. After that the firmware can be loaded with Ingenia again.
The boot loader source code (`iBL.s`, modified for JUMA by OH2NLT) is in [`bootloader/`](bootloader/).

---

## RS-928 clone

This firmware also runs on the **RS-928**, a PA-100D clone whose hardware is practically the same as
the original. Tested by DL4JC. Two things differ:

### First installation: PICkit through header J19

The RS-928 ships with its own firmware (v1.05q), **without the Ingenia boot loader** and without a
usable remote control. The serial update therefore only works after a one-off programming step:

1. With a **PICkit** (or another dsPIC programmer) on header **J19** of the controller board, write
   [`bootloader/Bootldr_Juma-PA100_v104.hex`](bootloader/Bootldr_Juma-PA100_v104.hex) to the
   dsPIC30F6014A. This file is a complete chip image: the JUMA firmware v1.04, the Ingenia boot
   loader and the configuration bits.
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

Background: [RS-928 review in the OARC wiki](https://wiki.oarc.uk/rs928ampreview).

---

## Testing after an update

Use a dummy load and low power first.

1. Start-up screen `v4.02a`, no checksum error, calibration values as before.
2. Power, current, voltage and temperature as before.
3. **SWR protection:** with a mismatched load, set the trip limit below the actual SWR → alarm and
   STBY immediately; trip limit above it → no alarm, also not with frequent keying or SSB.
4. **Alarm:** clear it with PWR and remotely with `=C`; `=R` before and after (last field = alarm bits).
5. **Band change in F-Sense mode:** 40 m → 20 m, 80 m → 20 m, 30 m → 20 m and back, several times
   each: no O/C alarm.
6. **F-Sense QSK** Off/On, SSB and CW on one band: no dropouts.
7. **Xiegu:** check each band (serial test `A` shows voltage and detected band).
8. **Watchdog:** hold OPER for more than 2 s while transmitting → TX drops.
9. **Settings:** change *F-Sense QSK* / Xiegu, save, switch off and on → the setting is kept.

---

## Building the firmware

### With MPLAB XC16 (macOS, Linux, Windows)

Requirement: [MPLAB XC16](https://www.microchip.com/xc16) v2.10 (the free version is enough; the
project is built without optimisation, `-O0`, as before).

```sh
./build-xc16.sh
```

Output: `Juma PA-100D <VERSION> Build <BUILD>.hex` in the project folder. Version and build number
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
| `Juma PA-100D v4.02a Build *-DL4JC.hex` | Current build |
| `Juma PA-100D.hex` | Original v4.01a Build 3 (to go back) |
| `tools/ingenia/ibl_dspiclist.xml` | Device file for the Ingenia loader |
| `bootloader/` | Boot loader source and HEX; `Bootldr_Juma-PA100_v104.hex` = complete image for a first installation with a programmer (RS-928) |
| `Juma PA-100D.mcp/.mcw/.mcs` | Original MPLAB 8 project |

---

## Known limitations

- **F-Sense QSK = On:** after a change to a lower band there is a window of a few milliseconds
  with poorly suppressed harmonics (the frequency can only be measured once RF is present). If
  that matters to you, leave the setting **Off** or send a short low-power carrier after a band
  change.
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

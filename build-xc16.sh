#!/bin/sh
# Build the PA-100D firmware with MPLAB XC16 on macOS or Linux.
# Output: "firmware/Juma PA-100D <VERSION>.hex". The original C30 HEX is not touched.
# XC16 is searched in its default location (macOS: /Applications/microchip/xc16, Linux: /opt/microchip/xc16), the
# newest installed version is used. Another location: XC16=/path/to/xc16/v2.10 ./build-xc16.sh
set -e
cd "$(dirname "$0")"
if [ -z "$XC16" ]; then
	case "$(uname -s)" in
		Darwin)	BASE=/Applications/microchip/xc16 ;;
		*)		BASE=/opt/microchip/xc16 ;;
	esac
	XC16=$(ls -d "$BASE"/v* 2>/dev/null | sort -V | tail -n 1)
fi
X=$XC16
if [ ! -x "$X/bin/xc16-gcc" ]; then
	echo "ERROR: XC16 not found (${X:-$BASE}). Set XC16=/path/to/xc16/<version>" >&2
	exit 1
fi
# XC16 for macOS is an Intel program: on Apple Silicon run it through Rosetta.
RUN=
if [ "$(uname -s)" = Darwin ] && [ "$(uname -m)" = arm64 ]; then RUN="arch -x86_64"; fi
echo "XC16: $X"
CC="$RUN $X/bin/xc16-gcc"
OUT=build
VERSION=$(sed -n 's/^#define VERSION[[:space:]]*"\(.*\)".*/\1/p' juma-pa100.h | tr -d '\r')
NAME="firmware/Juma PA-100D $VERSION.hex"
mkdir -p $OUT firmware
for f in adc12 juma-pa100 lcd-trx2 serial_pa100 serial_test service timers_pwm tmr5delay traps uart; do
	$CC -mcpu=30F6014A -x c -c "$f.c" -o "$OUT/$f.o" -g -Wall
done
$CC -mcpu=30F6014A -c DataEEPROM.s -o "$OUT/DataEEPROM.o" -Wa,-I"$X/support/dsPIC30F/inc"
$CC -mcpu=30F6014A "$OUT"/*.o -o "$OUT/pa100.elf" -Wl,--script=juma-trx2.gld,--heap=500,-Map="$OUT/pa100.map",--report-mem | grep -E 'Total'
$RUN "$X/bin/xc16-bin2hex" "$OUT/pa100.elf"
# Safety check: nothing may be placed in the boot loader area (PC 0x17D00 - 0x17FFF)
python3 - "$OUT/pa100.hex" <<'PY'
import sys
base = 0; bad = []
for l in open(sys.argv[1]):
    l = l.strip(); n = int(l[1:3], 16); a = int(l[3:7], 16); t = int(l[7:9], 16)
    if t == 4: base = int(l[9:13], 16) << 16
    elif t == 0:
        pc = (base + a) // 2
        if pc + n // 2 > 0x17D00 and pc <= 0x17FFF: bad.append(max(pc, 0x17D00))   # record covers PC pc .. pc + n/2 - 1
if bad:
    print("ERROR: HEX contains data in the boot loader area at PC 0x%05X" % min(bad)); sys.exit(1)
print("Boot loader area free (PC 0x17D00 - 0x17FFF)")
PY
rm -f "$NAME"
cp "$OUT/pa100.hex" "$NAME"
echo "OK: $NAME"

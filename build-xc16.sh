#!/bin/sh
# Build the PA-100D firmware with MPLAB XC16 (macOS, Apple Silicon via Rosetta).
# Output: "Juma PA-100D <BUILD>.hex" in this directory. The original C30 HEX is not touched.
set -e
cd "$(dirname "$0")"
X=${XC16:-/Applications/microchip/xc16/v2.10}
CC="arch -x86_64 $X/bin/xc16-gcc"
OUT=build
BUILD=$(sed -n 's/^#define BUILD_NUMBER[[:space:]]*"\(.*\)".*/\1/p' juma-pa100.h | tr -d '\r')
mkdir -p $OUT
for f in adc12 juma-pa100 lcd-trx2 serial_pa100 serial_test service timers_pwm tmr5delay traps uart; do
	$CC -mcpu=30F6014A -x c -c "$f.c" -o "$OUT/$f.o" -g -Wall
done
$CC -mcpu=30F6014A -c DataEEPROM.s -o "$OUT/DataEEPROM.o" -Wa,-I"$X/support/dsPIC30F/inc"
$CC -mcpu=30F6014A "$OUT"/*.o -o "$OUT/pa100.elf" -Wl,--script=juma-trx2.gld,--heap=500,-Map="$OUT/pa100.map",--report-mem | grep -E 'Total'
arch -x86_64 "$X/bin/xc16-bin2hex" "$OUT/pa100.elf"
cp "$OUT/pa100.hex" "Juma PA-100D $BUILD.hex"
echo "OK: Juma PA-100D $BUILD.hex"

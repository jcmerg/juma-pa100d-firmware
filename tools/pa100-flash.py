#!/usr/bin/env python3
"""
pa100-flash.py - Serial firmware loader for the JUMA PA-100D (and TRX-2) Ingenia boot loader.

A replacement for the Windows Ingenia dsPIC bootloader GUI, written from the boot loader firmware
source bootloader/iBL.s (ingenia-cat S.L., modified for JUMA by OH2NLT). Runs on Windows, macOS and
Linux. Requires Python 3 and pyserial (pip install pyserial).

Protocol (see iBL.s):
  Autobaud   PC sends 0x55, the boot loader measures 8 bit times      -> 0x55 (ACK)
  Version    0x03                                                     -> major, minor, 0x55
  Read       0x01 a2 a1 a0  (program address, 24 bit)                 -> b23..16, b15..8, b7..0, 0x55
  Write      0x02 a2 a1 a0 N data[N-1] chk                            -> 0x55 (ACK) or 0xFF (NACK)
             N includes the check byte; a2+a1+a0+N+data+chk = 0 mod 256. A program row is 32
             instructions = 64 addresses = 128 bytes, 4 bytes per instruction as in the HEX file.
  Run        0x0F                                                     -> jumps to the firmware

Only program memory is written. The configuration registers and the data EEPROM (calibration) are
never touched. The boot loader area 0x17D00 - 0x17FFE is never written, and the reset vector at
address 0 is always replaced by GOTO 0x17D00, so that the boot loader stays reachable. The firmware
is started by the boot loader through the GOTO __reset at 0x100, see juma-trx2.gld. DL4JC
"""

import argparse
import sys
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    serial = None

ACK = 0x55
NACK = 0xFF
C_READ = 0x01
C_WRITE = 0x02
C_VERSION = 0x03
C_USER = 0x0F

ROW_ADDR = 64                       # Program addresses per row (32 instructions)
ROW_BYTES = 128                     # HEX file bytes per row
BOOT_START = 0x17D00                # Boot loader area, see iBL.s and tools/ingenia/ibl_dspiclist.xml
BOOT_END = 0x17FFE
DEVID_ADDR = 0xFF0000
DEVICES = {0x02C3: "dsPIC30F6014A", 0x0198: "dsPIC30F6014"}
RESET_GOTO = (0x047D00, 0x000001)   # GOTO 0x17D00, as in bootloader/PA100_boot_loader.hex
BLANK = bytes((0xFF, 0xFF, 0xFF, 0x00))


class FlashError(Exception):
    pass


def load_hex(path):
    """Read an Intel HEX file into {program row address: 128 bytes}. Data outside program memory
    (configuration registers, EEPROM) is reported and ignored."""
    mem = {}
    base = 0
    with open(path) as f:
        for num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            if not line.startswith(":"):
                raise FlashError(f"{path}:{num}: not an Intel HEX record")
            rec = bytes.fromhex(line[1:])
            if len(rec) < 5 or len(rec) != rec[0] + 5 or sum(rec) & 0xFF:
                raise FlashError(f"{path}:{num}: bad record or checksum")
            count, addr, rtype, data = rec[0], (rec[1] << 8) | rec[2], rec[3], rec[4:-1]
            if rtype == 0x00:
                for i, b in enumerate(data):
                    mem[base + addr + i] = b
            elif rtype == 0x01:
                break
            elif rtype == 0x04:
                base = ((data[0] << 8) | data[1]) << 16
            elif rtype == 0x02:
                base = ((data[0] << 8) | data[1]) << 4
            else:
                raise FlashError(f"{path}:{num}: unsupported record type {rtype:02X}")

    rows = {}
    skipped = set()
    for byte_addr, b in mem.items():
        pc = byte_addr // 2
        if pc >= 0x800000:          # EEPROM 0x7FF000 is below, configuration 0xF80000 above
            skipped.add("configuration")
            continue
        if pc >= 0x7FF000:
            skipped.add("data EEPROM")
            continue
        if BOOT_START <= pc <= BOOT_END + 1:
            raise FlashError(f"HEX file contains data in the boot loader area (0x{pc:06X}), not flashing it")
        if pc > BOOT_END + 1:
            raise FlashError(f"HEX file contains data outside the program memory (0x{pc:06X})")
        row = pc - pc % ROW_ADDR
        if row not in rows:
            rows[row] = bytearray(BLANK * (ROW_BYTES // 4))
        rows[row][byte_addr - row * 2] = b

    if 0 not in rows or 0x100 not in rows:
        raise FlashError("HEX file has no reset vector or no start address 0x100, not a PA-100D firmware")
    reset, start = row_words(rows[0])[0:2], row_words(rows[0x100])[0:2]
    if (reset[0] >> 16) != 0x04 or start != reset:
        raise FlashError("HEX file has no GOTO __reset at 0x100 (linked without juma-trx2.gld?). "
                         "The boot loader could not start this firmware.")
    for i, word in enumerate(RESET_GOTO):  # Keep the boot loader reachable
        rows[0][i * 4:i * 4 + 4] = bytes((word & 0xFF, (word >> 8) & 0xFF, word >> 16, 0))

    return dict(sorted(rows.items())), sorted(skipped)


def row_words(data):
    return [data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) for i in range(0, ROW_BYTES, 4)]


def addr_bytes(pc):
    return bytes(((pc >> 16) & 0xFF, (pc >> 8) & 0xFF, pc & 0xFF))


class Loader:
    def __init__(self, port, baud):
        self.ser = serial.Serial(port, baud, bytesize=8, parity="N", stopbits=1, timeout=0.5)

    def close(self):
        self.ser.close()

    def read_exact(self, n, timeout=1.0):
        data = bytearray()
        end = time.monotonic() + timeout
        while len(data) < n and time.monotonic() < end:
            data += self.ser.read(n - len(data))
        return bytes(data)

    def connect(self, wait):
        """Send 0x55 until the boot loader answers. A continuous 0x55 stream is a square wave, so the
        autobaud measurement is correct wherever the boot loader starts. The bytes that follow the
        detection are answered with NACK and are discarded."""
        print("Switch the PA off. Then press and hold OPER and press PWR. Waiting ...", flush=True)
        self.ser.reset_input_buffer()
        end = time.monotonic() + wait
        while time.monotonic() < end:
            self.ser.write(bytes([ACK]) * 32)
            self.ser.flush()
            time.sleep(0.05)
            if self.ser.in_waiting:
                break
        else:
            raise FlashError("no answer from the boot loader")
        time.sleep(0.2)
        self.ser.reset_input_buffer()

        for _ in range(3):
            self.ser.write(bytes([C_VERSION]))
            ver = self.read_exact(3)
            if len(ver) == 3 and ver[2] == ACK:
                return ver[0], ver[1]
            time.sleep(0.1)
            self.ser.reset_input_buffer()
        raise FlashError("boot loader found, but no valid version answer. "
                         "Disconnect the PA from the power supply and try again, or use a lower baud rate.")

    def read_words(self, addrs, window=16):
        """Read program words. The read commands are sent in windows, so that the boot loader's
        transmit FIFO cannot overflow."""
        words = []
        for i in range(0, len(addrs), window):
            chunk = addrs[i:i + window]
            self.ser.write(b"".join(bytes([C_READ]) + addr_bytes(a) for a in chunk))
            data = self.read_exact(4 * len(chunk))
            if len(data) != 4 * len(chunk):
                raise FlashError(f"read timeout at 0x{chunk[0]:06X}")
            for j in range(len(chunk)):
                b = data[4 * j:4 * j + 4]
                if b[3] != ACK:
                    raise FlashError(f"read error at 0x{chunk[j]:06X}")
                words.append((b[0] << 16) | (b[1] << 8) | b[2])
        return words

    def write_row(self, pc, data):
        frame = bytearray(addr_bytes(pc))
        frame.append(len(data) + 1)
        frame += data
        frame.append(-sum(frame) & 0xFF)
        for attempt in range(3):
            self.ser.write(bytes([C_WRITE]) + frame)
            ans = self.read_exact(1)
            if ans == bytes([ACK]):
                return
            time.sleep(0.1)
            self.ser.reset_input_buffer()
        raise FlashError(f"write error at 0x{pc:06X} (answer {ans.hex() or 'none'})")

    def run(self):
        self.ser.write(bytes([C_USER]))
        self.ser.flush()


def progress(text, i, n):
    print(f"\r{text} {i}/{n} rows", end="", flush=True)


def verify(ldr, rows):
    bad = []
    for i, (pc, data) in enumerate(rows.items(), 1):
        got = ldr.read_words([pc + 2 * k for k in range(ROW_ADDR // 2)])
        if got != row_words(data):
            bad.append(pc)
        progress("Verifying", i, len(rows))
    print()
    return bad


def main():
    ap = argparse.ArgumentParser(description="Flash JUMA PA-100D firmware through the Ingenia boot loader.")
    ap.add_argument("hexfile", help="firmware HEX file, e.g. 'Juma PA-100D v4.02a Build 4-DL4JC.hex'")
    ap.add_argument("-p", "--port", help="serial port, e.g. COM3 or /dev/ttyUSB0 (omit to list ports)")
    ap.add_argument("-b", "--baud", type=int, default=115200, help="baud rate (default 115200)")
    ap.add_argument("--wait", type=float, default=60, help="seconds to wait for the boot loader (default 60)")
    ap.add_argument("--dry-run", action="store_true", help="only check the HEX file and show what would be written")
    ap.add_argument("--verify-only", action="store_true", help="compare the flash with the HEX file, do not write")
    ap.add_argument("--run", action="store_true", help="start the firmware afterwards instead of a power cycle")
    args = ap.parse_args()

    try:
        rows, skipped = load_hex(args.hexfile)
    except (OSError, ValueError, FlashError) as e:
        sys.exit(f"Error: {e}")

    last = max(rows) + ROW_ADDR - 2
    print(f"{args.hexfile}: {len(rows)} rows, 0x000000 - 0x{last:06X}, reset vector -> GOTO 0x{BOOT_START:06X}")
    if skipped:
        print(f"Ignored (never written): {', '.join(skipped)}")
    if args.dry_run:
        for pc in rows:
            print(f"  row 0x{pc:06X}")
        return

    if serial is None:
        sys.exit("Error: pyserial is required: pip install pyserial")
    if not args.port:
        print("Serial ports:")
        for p in serial.tools.list_ports.comports():
            print(f"  {p.device}  {p.description}")
        sys.exit("Choose one with --port")

    try:
        ldr = Loader(args.port, args.baud)
    except serial.SerialException as e:
        sys.exit(f"Error: {e}")

    try:
        major, minor = ldr.connect(args.wait)
        devid = ldr.read_words([DEVID_ADDR])[0] & 0xFFFF
        name = DEVICES.get(devid)
        print(f"Boot loader v{major}.{minor}, device ID 0x{devid:04X} {name or '(unknown)'}")
        if devid != 0x02C3:
            raise FlashError("not a dsPIC30F6014A, stopping")

        # Row 0 holds the reset vector (GOTO boot loader) and the interrupt vectors. Check that the PA has the
        # expected boot loader reset vector before anything is written, as otherwise its boot loader is at a
        # different address. Row 0 is only rewritten if it differs: between its erase and write (a few mS)
        # a power failure would leave no reset vector, and the boot loader could then only be restored with
        # a programmer.
        row0 = ldr.read_words([2 * k for k in range(ROW_ADDR // 2)])
        if tuple(row0[0:2]) != RESET_GOTO:
            raise FlashError(f"unexpected reset vector in the PA ({row0[0]:06X} {row0[1]:06X}), expected GOTO "
                             f"0x{BOOT_START:06X}. Different boot loader? Nothing has been written.")

        if not args.verify_only:
            todo = rows if row0 != row_words(rows[0]) else {pc: d for pc, d in rows.items() if pc}
            if 0 not in todo:
                print("Row 0 (reset and interrupt vectors) unchanged, not rewritten")
            t = time.monotonic()
            for i, (pc, data) in enumerate(todo.items(), 1):
                ldr.write_row(pc, data)
                progress("Writing", i, len(todo))
            print(f" ({time.monotonic() - t:.0f} s)")

        bad = verify(ldr, rows)
        if bad:
            raise FlashError(f"verify failed in {len(bad)} rows, first at 0x{bad[0]:06X}. "
                             "Flash again (the boot loader is not affected).")
        print("Verify OK")

        if args.run:
            ldr.run()
            print("Done, firmware started.")
        else:
            print("Done. Disconnect the power supply (PWR does not work in the boot loader), then switch on normally.")
    except FlashError as e:
        sys.exit(f"\nError: {e}")
    except KeyboardInterrupt:
        sys.exit("\nAborted. Flash again (the boot loader is not affected).")
    finally:
        ldr.close()


if __name__ == "__main__":
    main()

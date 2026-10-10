#!/usr/bin/env python3
"""
juma-flash.py - Serial firmware loader for JUMA devices with the Ingenia dsPIC boot loader (PA-100D, TRX-2, ...).

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
never touched. The boot loader address is taken from the reset vector of the device: the boot loader
only runs because the reset vector points to it. Everything from this address up is never written,
and the reset vector in the firmware is replaced by the one of the device, so that the boot loader
stays reachable. The firmware is started by the boot loader through the GOTO __reset at 0x100, see
juma-trx2.gld. Only tested with the PA-100D (boot loader at 0x17D00). DL4JC
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
DEVID_ADDR = 0xFF0000
DEVICES = {                         # Device ID: name, end of program memory, see tools/ingenia/ibl_dspiclist.xml
    0x01C1: ("dsPIC30F3011", 0x004000),
    0x0101: ("dsPIC30F4011", 0x008000),
    0x0080: ("dsPIC30F5011", 0x00B000),
    0x0198: ("dsPIC30F6014", 0x018000),
    0x02C3: ("dsPIC30F6014A", 0x018000),
}
FLASH_MAX = 0x018000
FILL = bytes(4)                     # Gaps within a row: NOP (0x000000), as written by the Ingenia GUI


class FlashError(Exception):
    pass


def load_hex(path):
    """Read an Intel HEX file into {program row address: 128 bytes} and {program row address: set of the
    instruction indices that are in the file}. Data outside program memory (configuration registers,
    EEPROM) is reported and ignored."""
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
    present = {}
    skipped = set()
    for byte_addr, b in mem.items():
        pc = byte_addr // 2
        if pc >= 0x800000:          # EEPROM 0x7FF000 is below, configuration 0xF80000 above
            skipped.add("configuration")
            continue
        if pc >= 0x7FF000:
            skipped.add("data EEPROM")
            continue
        if pc >= FLASH_MAX:
            raise FlashError(f"HEX file contains data outside the program memory (0x{pc:06X})")
        row = pc - pc % ROW_ADDR
        if row not in rows:
            rows[row] = bytearray(FILL * (ROW_BYTES // 4))
            present[row] = set()
        rows[row][byte_addr - row * 2] = b
        present[row].add((pc - row) // 2)

    if 0 not in rows or 0x100 not in rows:
        raise FlashError("HEX file has no reset vector or no start address 0x100, not a JUMA firmware")
    reset, start = row_words(rows[0])[0:2], row_words(rows[0x100])[0:2]
    if (reset[0] >> 16) != 0x04 or start != reset:
        raise FlashError("HEX file has no GOTO __reset at 0x100 (linked without juma-trx2.gld?). "
                         "The boot loader could not start this firmware.")

    return dict(sorted(rows.items())), present, sorted(skipped)


def row_words(data):
    return [data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) for i in range(0, ROW_BYTES, 4)]


def goto_target(w0, w1):
    """Target of a GOTO instruction (two words), or None."""
    if (w0 >> 16) != 0x04 or (w1 & 0xFFFF80):
        return None
    return (w0 & 0xFFFE) | ((w1 & 0x7F) << 16)


def set_reset(rows, present, w0, w1):
    for i, word in enumerate((w0, w1)):
        rows[0][i * 4:i * 4 + 4] = bytes((word & 0xFF, (word >> 8) & 0xFF, word >> 16, 0))
        present[0].add(i)


def same(got, data, mask):
    """Compare the words read from the device with a row, only where the HEX file has data. The gaps may hold
    0x000000 (Ingenia GUI, this tool) or 0xFFFFFF (erased), they are never executed."""
    exp = row_words(data)
    return all(got[k] == exp[k] for k in mask)


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
        print("Switch the device off and start its boot loader (PA-100D: press and hold OPER, then press PWR).\n"
              "Waiting ...", flush=True)
        self.ser.reset_input_buffer()
        answered = False
        end = time.monotonic() + wait
        while time.monotonic() < end:
            # Without pauses: the boot loader only waits a short time for the first edges (approx. 1 s in iBL.s, possibly
            # less in other builds) and then starts the firmware. A pause between the bursts could fall into this time.
            self.ser.write(bytes([ACK]) * 16)
            self.ser.flush()
            if not self.ser.in_waiting:
                continue
            # Something answered. It may also be the running firmware, e.g. in the serial test mode, so only
            # a valid version answer counts. Otherwise keep waiting until the device starts its boot loader.
            answered = True
            time.sleep(0.2)
            self.ser.reset_input_buffer()
            for _ in range(2):
                self.ser.write(bytes([C_VERSION]))
                ver = self.read_exact(3)
                if len(ver) == 3 and ver[2] == ACK:
                    return ver[0], ver[1]
                time.sleep(0.1)
                self.ser.reset_input_buffer()
        if answered:
            raise FlashError("the device answers, but not as the boot loader. Is the firmware still running? "
                             "Otherwise disconnect the power supply and try again, or use a lower baud rate.")
        raise FlashError("no answer from the boot loader")

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
        """Write one row. Only a NACK is retried: the boot loader sends it after the whole frame, when the check byte
        was wrong, and then waits for the next command. Without an answer a byte may have been lost, and the boot
        loader is still waiting for the rest of the frame (iBL.s waits very long for each byte). Sending the frame
        again would then be read partly as commands, so stop instead: the device has to be restarted."""
        frame = bytearray(addr_bytes(pc))
        frame.append(len(data) + 1)
        frame += data
        frame.append(-sum(frame) & 0xFF)
        for attempt in range(3):
            self.ser.write(bytes([C_WRITE]) + frame)
            ans = self.read_exact(1)
            if ans == bytes([ACK]):
                return
            if ans != bytes([NACK]):
                raise FlashError(f"no answer when writing 0x{pc:06X} ({ans.hex() or 'timeout'}). Disconnect the power "
                                 "supply, start the boot loader again and flash again (the boot loader is not affected).")
            time.sleep(0.05)
            self.ser.reset_input_buffer()
        raise FlashError(f"write error at 0x{pc:06X} (NACK 3 times). Try a lower baud rate.")

    def run(self):
        self.ser.write(bytes([C_USER]))
        self.ser.flush()


def progress(text, i, n):
    print(f"\r{text} {i}/{n} rows", end="", flush=True)


def verify(ldr, rows, present):
    bad = []
    for i, (pc, data) in enumerate(rows.items(), 1):
        got = ldr.read_words([pc + 2 * k for k in range(ROW_ADDR // 2)])
        if not same(got, data, present[pc]):
            bad.append(pc)
        progress("Verifying", i, len(rows))
    print()
    return bad


def main():
    ap = argparse.ArgumentParser(description="Flash JUMA firmware through the Ingenia dsPIC boot loader.")
    ap.add_argument("hexfile", help="firmware HEX file, e.g. 'firmware/Juma PA-100D v4.03.hex'")
    ap.add_argument("-p", "--port", help="serial port, e.g. COM3 or /dev/ttyUSB0 (omit to list ports)")
    ap.add_argument("-b", "--baud", type=int, default=115200, help="baud rate (default 115200)")
    ap.add_argument("--wait", type=float, default=60, help="seconds to wait for the boot loader (default 60)")
    ap.add_argument("--dry-run", action="store_true", help="only check the HEX file and show what would be written")
    ap.add_argument("--verify-only", action="store_true", help="compare the flash with the HEX file, do not write")
    ap.add_argument("--run", action="store_true", help="start the firmware afterwards instead of a power cycle")
    args = ap.parse_args()

    try:
        rows, present, skipped = load_hex(args.hexfile)
    except (OSError, ValueError, FlashError) as e:
        sys.exit(f"Error: {e}")

    last = max(rows) + ROW_ADDR - 2
    print(f"{args.hexfile}: {len(rows)} rows, 0x000000 - 0x{last:06X}")
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
        if devid not in DEVICES:
            raise FlashError(f"unknown device ID 0x{devid:04X}, stopping")
        name, flash_end = DEVICES[devid]
        print(f"Boot loader v{major}.{minor}, {name}")

        # Row 0 holds the reset vector and the interrupt vectors. The boot loader runs because the reset vector
        # points to it, so its target is the boot loader address. It has to be in the last part of the program
        # memory and at the start of a row, so that no firmware row overlaps it. The firmware's reset vector is
        # replaced by the device's own. Row 0 is only rewritten if it differs: between its erase and write (a few
        # mS) a power failure would leave no reset vector, and the boot loader could then only be restored with
        # a programmer.
        row0 = ldr.read_words([2 * k for k in range(ROW_ADDR // 2)])
        boot = goto_target(row0[0], row0[1])
        if boot is None or boot % ROW_ADDR or not (flash_end - 0x1000 <= boot < flash_end):
            raise FlashError(f"unexpected reset vector in the device ({row0[0]:06X} {row0[1]:06X}). "
                             "Nothing has been written.")
        print(f"Boot loader at 0x{boot:06X} - 0x{flash_end - 2:06X} (protected)")
        if max(rows) >= boot:
            raise FlashError(f"HEX file contains data in the boot loader area (from 0x{boot:06X}). "
                             "Nothing has been written.")
        set_reset(rows, present, row0[0], row0[1])

        if not args.verify_only:
            todo = rows if not same(row0, rows[0], present[0]) else {pc: d for pc, d in rows.items() if pc}
            if 0 not in todo:
                print("Row 0 (reset and interrupt vectors) unchanged, not rewritten")
            t = time.monotonic()
            for i, (pc, data) in enumerate(todo.items(), 1):
                ldr.write_row(pc, data)
                progress("Writing", i, len(todo))
            print(f" ({time.monotonic() - t:.0f} s)")

        bad = verify(ldr, rows, present)
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

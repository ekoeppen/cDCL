#!/usr/bin/env python3
"""Exercise the actual CLI/server/link wiring over a private PTY, not a Newton."""

import os
from pathlib import Path
import pty
import select
import struct
import subprocess
import sys
import tempfile
import time
import tty


NEWTON_LR = bytes.fromhex(
    "26 01 02 01 06 01 00 00 00 00 ff 02 01 02 03 01 08 04 02 40 00 "
    "08 01 03 09 01 01 0e 04 03 04 00 fa c5 06 01 04 00 00 e1 00"
)
DESKTOP_LR = bytes.fromhex(
    "17 01 02 01 06 01 00 00 00 00 ff 02 01 02 03 01 02 04 02 40 00 08 01 03"
)
PEER_LD = bytes.fromhex("07 02 01 01 ff 02 01 00")


def crc16(data):
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = (crc >> 1) ^ (0xA001 if crc & 1 else 0)
    return crc


def encode_frame(body):
    crc = crc16(body + b"\x03")
    return (b"\x16\x10\x02" + body.replace(b"\x10", b"\x10\x10")
            + b"\x10\x03" + struct.pack("<H", crc))


class Peer:
    def __init__(self, fd):
        self.fd = fd
        self.buffer = bytearray()
        self.dock = bytearray()
        self.tx_seq = self.rx_seq = self.acked = 0

    def send(self, body):
        wire = encode_frame(body)
        while wire:
            written = os.write(self.fd, wire)
            wire = wire[written:]

    def receive(self):
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            # Decode escaped frames and verify their CRC independently of cDCL.
            start = self.buffer.find(b"\x16\x10\x02")
            if start >= 0:
                body = bytearray()
                index = start + 3
                while index < len(self.buffer):
                    byte = self.buffer[index]
                    if byte != 0x10:
                        body.append(byte)
                        index += 1
                        continue
                    if index + 1 >= len(self.buffer):
                        break
                    following = self.buffer[index + 1]
                    if following == 0x10:
                        body.append(0x10)
                        index += 2
                        continue
                    if following != 0x03:
                        raise AssertionError("Invalid MNP frame escape")
                    if index + 4 > len(self.buffer):
                        break
                    crc = struct.unpack("<H", self.buffer[index + 2:index + 4])[0]
                    if crc != crc16(body + b"\x03"):
                        raise AssertionError("Invalid MNP reply CRC")
                    del self.buffer[:index + 4]
                    return bytes(body)
            if select.select([self.fd], [], [], 0.05)[0]:
                self.buffer.extend(os.read(self.fd, 4096))
        raise AssertionError("No MNP reply within 5 seconds")

    def pump(self):
        frame = self.receive()
        if frame[:2] == bytes([3, 5]):
            assert frame[2:] == bytes([self.tx_seq, 1]), "Incorrect MNP ACK"
            self.acked = frame[2]
        elif frame[:2] == bytes([2, 4]):
            self.rx_seq = (self.rx_seq + 1) % 256
            assert frame[2] == self.rx_seq, "Incorrect MNP LT sequence"
            self.dock.extend(frame[3:])
            self.send(bytes([3, 5, self.rx_seq, 1]))
        else:
            raise AssertionError("Unexpected MNP response: " + frame.hex())

    def send_command(self, command, payload=b""):
        wire = b"newtdock" + command + struct.pack(">I", len(payload)) + payload
        wire += bytes((-len(payload)) % 4)
        for offset in range(0, len(wire), 253):
            self.tx_seq = (self.tx_seq + 1) % 256
            self.send(bytes([2, 4, self.tx_seq]) + wire[offset:offset + 253])
            while self.acked != self.tx_seq:
                self.pump()

    def command(self, expected):
        while len(self.dock) < 16:
            self.pump()
        assert self.dock[:8] == b"newtdock", "Invalid Dock header"
        command = bytes(self.dock[8:12])
        length = struct.unpack(">I", self.dock[12:16])[0]
        padded = (length + 3) & ~3
        while len(self.dock) < 16 + padded:
            self.pump()
        payload = bytes(self.dock[16:16 + length])
        del self.dock[:16 + padded]
        assert command == expected, f"Expected {expected!r}, received {command!r}"
        return payload


# Small independent NSOF encoder: no precedents needed for these fixtures.
def xlong(value):
    return bytes([value]) if value < 255 else b"\xff" + struct.pack(">I", value)


def nsof(value):
    if isinstance(value, str):
        text = (value + "\0").encode("utf-16-be")
        return b"\x08" + xlong(len(text)) + text
    if isinstance(value, int):
        return b"\x00" + xlong(value << 2)
    if isinstance(value, list):
        return b"\x05" + xlong(len(value)) + b"".join(nsof(item) for item in value)
    if isinstance(value, dict):
        keys = b"".join(b"\x07" + xlong(len(key)) + key.encode("ascii") for key in value)
        return b"\x06" + xlong(len(value)) + keys + b"".join(nsof(item) for item in value.values())
    raise AssertionError("Unsupported test NSOF value")


def encrypt(helper, challenge):
    # Use cDCL's DES implementation for authentication, not an external crypto
    # dependency. The protocol/state/NSOF fixtures themselves are independent.
    result = subprocess.check_output([helper, "--encrypt-challenge", challenge.hex()], timeout=5)
    assert len(result) == 8, "DES helper emitted diagnostics instead of exactly eight bytes"
    return result


def export_soup(peer, helper):
    # Newton OS 2.1: version info has 18 words following its length word.
    peer.send_command(b"name", struct.pack(">19I", 72, *([0] * 18))
                      + "Test Newton\0".encode("utf-16-be"))
    desktop = peer.command(b"dinf")
    assert struct.unpack(">I", desktop[:4])[0] == 10, "Incorrect Dock protocol version"
    challenge = bytes.fromhex("0123456789abcdef")
    peer.send_command(b"ninf", struct.pack(">I", 10) + challenge)
    assert peer.command(b"wicn") == bytes(4), "Exporter should hide operation buttons"
    peer.send_command(b"dres", bytes(4))
    assert len(peer.command(b"stim")) == 4
    peer.send_command(b"pass", encrypt(helper, desktop[8:16]))
    assert peer.command(b"pass") == encrypt(helper, challenge), "Incorrect password response"

    assert peer.command(b"cvbo") == bytes(4), "VBO compression not disabled"
    # Crucial regression: the Newton does NOT send a dres for cvbo.
    assert peer.command(b"gsto") == b"", "Store enumeration must follow cvbo immediately"
    peer.send_command(b"stor", b"\x02" + nsof([{"name": "Internal", "kind": "Internal", "signature": 123}]))
    peer.command(b"ssto")
    peer.send_command(b"dres", bytes(4))
    assert peer.command(b"gets") == b""
    peer.send_command(b"soup", b"\x02" + nsof(["Names"]) + nsof([42]))
    assert peer.command(b"ssou") == "Names\0".encode("utf-16-be")
    peer.send_command(b"dres", bytes(4))
    assert peer.command(b"gsin") == b""
    peer.send_command(b"sinf", b"\x02" + nsof({}))
    assert peer.command(b"gind") == b""
    peer.send_command(b"indx", b"\x02" + nsof([]))
    assert peer.command(b"snds") == b""
    # Span several LTs, including escaped DLE bytes and non-four-byte length.
    entry = b"\x02" + nsof({"_uniqueID": 7, "title": "Newton\x10" * 100})
    peer.send_command(b"entr", entry)
    peer.send_command(b"bsdn")
    assert peer.command(b"disc") == b""
    return entry


def check_cli(executable, helper, complete=True):
    with tempfile.TemporaryDirectory(prefix="cdcl-soup-pty-") as temporary:
        root = Path(temporary)
        master, slave = pty.openpty()
        process = None
        try:
            tty.setraw(slave)
            args = [executable, "--serial", os.ttyname(slave), "--soup", "Names",
                    "--output", str(root / "export")]
            environment = os.environ.copy()
            environment.pop("NEWTON_PASSWORD", None)  # Test the empty password, independent of the caller.
            with (root / "stderr.log").open("w") as log, (root / "stdout.log").open("w") as stdout:
                process = subprocess.Popen(args, stdout=stdout, stderr=log, env=environment)
                deadline = time.monotonic() + 5
                while "Waiting for Newton Dock connection" not in (root / "stderr.log").read_text():
                    if process.poll() is not None or time.monotonic() >= deadline:
                        raise AssertionError("CLI did not start listening")
                    time.sleep(0.01)

                peer = Peer(master)
                peer.send(NEWTON_LR)
                assert peer.receive() == DESKTOP_LR, "Incorrect LR negotiation reply"
                peer.send(bytes([3, 5, 0, 1]))
                peer.send_command(b"rtdk", struct.pack(">I", 9))
                assert peer.command(b"dock") == bytes(4), "Incorrect initiate-docking message"
                entry = export_soup(peer, helper) if complete else None
                peer.send(PEER_LD)
                expected_status = 0 if complete else 1
                assert process.wait(timeout=10) == expected_status, "Incorrect exit status or abnormal termination"

            text = (root / "stderr.log").read_text()
            assert (root / "stdout.log").read_bytes() == b"", "Unexpected stdout (DES diagnostics?)"
            if complete:
                assert "Export complete" in text
                assert (root / "export" / "COMPLETE").exists(), "Missing completion marker"
                saved = root / "export" / "store-000001" / "entry-000001.nsof"
                assert saved.read_bytes() == entry, "NSOF entry changed or padding exported"
            else:
                assert not (root / "export" / "COMPLETE").exists(), "Incomplete export marked complete"
                assert "Export incomplete" in text
            assert "[MNP " not in text, "MNP logging enabled by default"
        except Exception as error:
            text = (root / "stderr.log").read_text() if (root / "stderr.log").exists() else ""
            raise AssertionError(f"{error}\nCLI stderr:\n{text}") from error
        finally:
            if process is not None and process.poll() is None:
                process.kill()
                process.wait()
            os.close(master)
            os.close(slave)


if __name__ == "__main__":
    check_cli(sys.argv[1], sys.argv[2], complete=True)
    check_cli(sys.argv[1], sys.argv[2], complete=False)
    print("SoupDownloader PTY integration tests passed")

#!/usr/bin/env python3
"""Audit actual linked native strong-operation seams; does not execute hardware."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-results", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--objdump", type=Path, default=Path.home() / ".platformio/packages/toolchain-xtensa-esp-elf/bin/xtensa-esp-elf-objdump")
    parser.add_argument("--elf", action="append", default=[], help="Additional profile=ELF path")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    profiles = []
    for row in json.loads(args.build_results.read_text()):
        assert row["exit_code"] == 0, row
        profiles.append((
            row["form"] + "-" + row["target"],
            Path(row["project"]) / ".pio/build" / row["target"] / "firmware.elf",
        ))
    for value in args.elf:
        name, path = value.split("=", 1)
        profiles.append((name, Path(path)))
    allowed = {"entry", "memw", "l32i", "l32i.n", "s32i", "s32i.n", "retw", "retw.n", "ret", "ret.n", "nop", "nop.n", "mov", "mov.n"}
    rows = []
    for name, elf in profiles:
        symbols = {}
        for symbol, required in [("NativeStrongLoad", "l32i"), ("NativeStrongStore", "s32i")]:
            raw = subprocess.check_output([
                str(args.objdump), "-d", "--disassemble=" + symbol, str(elf)
            ], text=True)
            (args.output / (name + "-" + symbol + ".asm")).write_text(raw)
            match = re.search(r"<" + symbol + r">:\n(.*?)(?:\n\s*\n|\Z)", raw, re.S)
            assert match, (name, symbol, "missing linked operation")
            operations = [
                line.split("\t")[2].strip().split()[0]
                for line in match[1].splitlines()
                if re.match(r"^\s*[0-9a-f]+:", line)
            ]
            assert operations and all(op in allowed for op in operations), (name, symbol, operations)
            positions = [i for i, op in enumerate(operations) if op.split(".")[0] == required]
            assert len(positions) == 1, (name, symbol, operations)
            index = positions[0]
            assert "memw" in operations[:index] and "memw" in operations[index + 1:], (name, symbol, operations)
            symbols[symbol] = operations
        row = {
            "profile": name, "elf": str(elf),
            "elf_sha256": hashlib.sha256(elf.read_bytes()).hexdigest(),
            "symbols": symbols, "no_call_branch_retry_or_masking": True,
        }
        rows.append(row)
        print(json.dumps(row), flush=True)
    (args.output / "results.json").write_text(json.dumps(rows, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

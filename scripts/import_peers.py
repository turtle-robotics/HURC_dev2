#!/usr/bin/env python3
"""
import_peers.py — reads a CSV exported from the HURC ESP32 Mac Address Google Sheet
and updates the team_names[] and address_list[] arrays in main.cpp.

Usage:
    python import_peers.py <responses.csv> [main.cpp]

If main.cpp is not specified, it defaults to "main.cpp" in the current directory.
The original file is backed up as main.cpp.bak before any changes are made.
"""

import csv
import sys
import re
import shutil
from pathlib import Path
from datetime import datetime

COL_TIMESTAMP = "Timestamp"
COL_TEAM_NUM  = "What is your team number? SOMTECH members put \"SOMTECH\""
COL_TEAM_NAME = "Create a team name. SOMTECH members put SOMTECH <team name>"
MAC_COL_HINT  = "xx:xx:xx:xx:xx:xx"

DEBUG_ENTRY = {
    "label":   "debug",
    "name":    "debug test",
    "address": [0x08, 0x00, 0x00, 0x00, 0x00, 0x00],
}


def parse_timestamp(ts: str) -> datetime:
    for fmt in ("%Y-%m-%d %H:%M:%S.%f", "%Y-%m-%d %H:%M:%S", "%m/%d/%Y %H:%M:%S"):
        try:
            return datetime.strptime(ts.strip(), fmt)
        except ValueError:
            continue
    return datetime.min


def find_mac_indices(headers: list[str]) -> list[int]:
    """
    Return the indices of the 6 MAC byte columns.
    Works whether Google Sheets exports them all with the same name
    or with numbered suffixes — we just find the first matching column
    and take 6 consecutive columns from there.
    """
    first = None
    for i, h in enumerate(headers):
        if MAC_COL_HINT in h:
            first = i
            break
    if first is None:
        sys.exit(f"ERROR: Could not find MAC address columns (looking for '{MAC_COL_HINT}' in headers).\n"
                 f"Headers found: {headers}")
    indices = list(range(first, first + 6))
    return indices


def parse_mac(row_values: list[str], mac_indices: list[int]) -> list[int] | None:
    try:
        return [int(row_values[i].strip(), 16) for i in mac_indices]
    except (IndexError, ValueError):
        return None


def mac_to_c(bytes_: list[int]) -> str:
    return "{" + ", ".join(f"0x{b:02x}" for b in bytes_) + "}"


def team_key(team_num: str) -> tuple:
    t = team_num.strip().upper()
    if t == "SOMTECH":
        return (1, 0, "")
    try:
        return (0, int(t), "")
    except ValueError:
        return (0, 999, t)


def load_csv(path: Path) -> tuple[list[str], list[list[str]]]:
    with path.open(newline="", encoding="utf-8-sig") as f:
        reader = csv.reader(f)
        headers = next(reader)
        rows = list(reader)
    return headers, rows


def deduplicate(headers, rows, mac_indices) -> list[dict]:
    num_idx  = headers.index(COL_TEAM_NUM)
    name_idx = headers.index(COL_TEAM_NAME)
    ts_idx   = headers.index(COL_TIMESTAMP)

    best: dict[tuple, dict] = {}
    for row in rows:
        if len(row) <= max(num_idx, name_idx, ts_idx):
            continue
        num  = row[num_idx].strip()
        name = row[name_idx].strip()
        mac  = parse_mac(row, mac_indices)
        if not num or not name or mac is None:
            continue

        key = (num.upper(), name.lower()) if num.upper() == "SOMTECH" else (num, "")
        ts  = parse_timestamp(row[ts_idx])
        if key not in best or ts > parse_timestamp(best[key]["row"][ts_idx]):
            best[key] = {"row": row, "num": num, "name": name, "mac": mac}

    return list(best.values())


def build_entries(headers, rows, mac_indices) -> list[dict]:
    deduped = deduplicate(headers, rows, mac_indices)
    entries = []
    for e in deduped:
        num   = e["num"]
        name  = e["name"]
        mac   = e["mac"]
        label = f"SOMTECH ({name})" if num.upper() == "SOMTECH" else str(num)
        entries.append({"label": label, "name": name, "address": mac, "_num": num})

    entries.sort(key=lambda e: team_key(e["_num"]))
    return [DEBUG_ENTRY] + entries


def build_c_arrays(entries: list[dict]) -> tuple[str, str]:
    count = len(entries)
    names_lines = [
        f"const int address_count = {count}; // total number of addresses in the list\n",
        "\n",
        f"const char* team_names[address_count] PROGMEM = {{\n",
    ]
    addr_lines = [f"const uint8_t address_list[address_count][6] PROGMEM = {{\n"]

    for e in entries:
        label = e["label"]
        name  = e["name"].replace('"', '\\"')
        mac   = mac_to_c(e["address"])
        names_lines.append(f'  "{name}", // {label}\n')
        addr_lines.append( f"  {mac}, // {label}\n")

    names_lines.append("};\n")
    addr_lines.append( "};\n")
    return "".join(names_lines), "".join(addr_lines)


_COUNT_RE = re.compile(r"const int address_count\s*=\s*\d+;[^\n]*\n")
_NAMES_RE = re.compile(
    r"const char\*\s+team_names\[address_count\]\s+PROGMEM\s*=\s*\{.*?\};\n",
    re.DOTALL,
)
_ADDR_RE  = re.compile(
    r"const uint8_t\s+address_list\[address_count\]\[6\]\s+PROGMEM\s*=\s*\{.*?\};\n",
    re.DOTALL,
)


def patch_cpp(source: str, names_block: str, addr_block: str) -> str:
    source = _COUNT_RE.sub("", source, count=1)
    if not _NAMES_RE.search(source):
        sys.exit("ERROR: Could not locate team_names[] in main.cpp")
    source = _NAMES_RE.sub(names_block, source, count=1)
    if not _ADDR_RE.search(source):
        sys.exit("ERROR: Could not locate address_list[] in main.cpp")
    source = _ADDR_RE.sub(addr_block, source, count=1)
    return source


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    csv_path = Path(sys.argv[1])
    cpp_path = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("main.cpp")

    if not csv_path.exists():
        sys.exit(f"ERROR: CSV file not found: {csv_path}")
    if not cpp_path.exists():
        sys.exit(f"ERROR: main.cpp not found: {cpp_path}")

    headers, rows = load_csv(csv_path)
    mac_indices   = find_mac_indices(headers)
    entries       = build_entries(headers, rows, mac_indices)
    names_block, addr_block = build_c_arrays(entries)

    backup = cpp_path.with_suffix(".cpp.bak")
    shutil.copy2(cpp_path, backup)
    print(f"Backed up original to {backup}")

    source  = cpp_path.read_text(encoding="utf-8")
    patched = patch_cpp(source, names_block, addr_block)
    cpp_path.write_text(patched, encoding="utf-8")

    print(f"Updated {cpp_path} with {len(entries)} entries:")
    for e in entries:
        mac_str = ":".join(f"{b:02x}" for b in e["address"])
        print(f"  [{e['label']:>12}]  {e['name']:<40}  {mac_str}")


if __name__ == "__main__":
    main()
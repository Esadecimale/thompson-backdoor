#!/usr/bin/env python3
"""Generate thompson payload """
from pathlib import Path

HERE = Path(__file__).resolve().parent
tail = (HERE / "thompson-tail.c").read_text()
if "\0" in tail:
    raise SystemExit("tail.c must not contain NUL bytes")

# Fix cstr_cat call in generated mental model — tail already written.
lines = ["static const char thompson_payload[] = {"]
for c in tail:
    lines.append(f"\t{ord(c)},")
lines.append("\t0")
lines.append("};")
lines.append(tail)
chunk = "\n".join(lines)
if not chunk.endswith("\n"):
    chunk += "\n"

(HERE / "thompson-chunk.c").write_text(chunk)
print(f"wrote chunk.c ({len(chunk)} bytes, payload {len(tail)} bytes)")

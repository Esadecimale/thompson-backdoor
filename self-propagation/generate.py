#!/usr/bin/env python3
"""Write thompson-chunk.c as thompson_rebuild_chunk() would emit it.
The bytes after the array terminator are thompson_payload[], exactly.
"""
from pathlib import Path

HERE = Path(__file__).resolve().parent

def build_chunk(tail: str) -> str:
    if "\0" in tail:
        raise SystemExit("thompson-tail.c must not contain NUL bytes")
    body = "".join(f"\t{ord(c)},\n" for c in tail)
    return (
        "static const char thompson_payload[] = {\n"
        + body
        + "\t0\n};\n"
        + tail
    )

def main() -> None:
    tail = (HERE / "thompson-tail.c").read_text()
    chunk = build_chunk(tail)
    marker = "\t0\n};\n"
    encoded, _, suffix = chunk.partition(marker)
    if not encoded.startswith("static const char thompson_payload[] = {\n"):
        raise SystemExit("chunk header mismatch")
    if suffix != tail:
        raise SystemExit("chunk suffix is not the tail; refusing to write")
    out = HERE / "thompson-chunk.c"
    out.write_text(chunk)
    print(f"wrote {out.name} ({len(chunk)} bytes, payload {len(tail)} bytes)")

if __name__ == "__main__":
    main()

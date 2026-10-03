#!/usr/bin/env python3
"""Build patch/2-self-propagation.patch from thompson-chunk.c.

The patch can be applied to a working version of tinyCC to modify the
tcc_add_file_internal(...) function in order to replace the original call
to tcc_compile(...) with the call to thompson_hook_add_file(...).
"""

from difflib import unified_diff
from pathlib import Path

HERE = Path(__file__).resolve().parent
LIBTCC = HERE.parent / "tinycc" / "libtcc.c"
CHUNK = HERE / "thompson-chunk.c"
OUTPUT = HERE.parent / "patch" / "2-self-propagation.patch"
ANCHOR = "ST_FUNC int tcc_add_file_internal"
RET_OLD = "    return tcc_compile(s1, flags, filename, fd);"
RET_NEW = "    return thompson_hook_add_file(s1, filename, flags, fd);"

def poison(lib: str, chunk: str) -> str:
    if "thompson_payload[" in lib:
        raise SystemExit("libtcc.c already contains thompson_payload")
    if ANCHOR not in lib:
        raise SystemExit("anchor not found in libtcc.c")
    if RET_OLD not in lib:
        raise SystemExit("compile return site not found in libtcc.c")
    if not chunk.endswith("\n"):
        raise SystemExit("thompson-chunk.c must end with a newline")
    lib = lib.replace(RET_OLD, RET_NEW, 1)
    return lib.replace(ANCHOR, chunk + ANCHOR, 1)

def main() -> None:
    clean = LIBTCC.read_text()
    chunk = CHUNK.read_text()
    poisoned = poison(clean, chunk)
    diff = "".join(
        unified_diff(
            clean.splitlines(keepends=True),
            poisoned.splitlines(keepends=True),
            fromfile="a/libtcc.c",
            tofile="b/libtcc.c",
            n=3,
        )
    )
    if not diff.endswith("\n"):
        diff += "\n"
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(diff)
    print(f"wrote {OUTPUT} ({diff.count(chr(10))} lines)")

if __name__ == "__main__":
    main()

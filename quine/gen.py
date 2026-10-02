#!/usr/bin/env python3
"""
Assemble the full Thompson quine (thompson.c) from its algorithmic part.

A quine of this shape is three pieces glued together:

    HEADER        the fixed preamble, up to and including "char s[] = {\n"
    <byte lines>  one "\t<n>,\n" line per byte of the TAIL
    TAIL          the array terminator, the "};" that closes it, and main()

The trick: the byte lines are emitted by main()'s for-loop, which always
appends a comma. The array's terminating 0 is therefore NOT a byte line --
it is the first line of the TAIL ("\t0") and is the only element written
without a comma. main() then prints the TAIL verbatim via printf("%s", s).

So s[] must hold exactly the bytes of TAIL, and the output is simply

    HEADER + "".join("\t%d,\n" % b for b in TAIL_bytes) + TAIL

thompson-tail.c holds only the algorithmic part (the main() function). This
script prepends the fixed array-closing boilerplate to form the full TAIL,
encodes it into s[], and writes the complete quine to thompson.c.

These three constants MUST stay consistent with the strings that main()
prints inside thompson-tail.c, or the result will not reproduce itself.
"""

HEADER = "#include <stdio.h>\n\nchar s[] = {\n"
ARRAY_CLOSE = "\t0\n};\n\n"          # terminating 0 + "};" + blank line
LINE = "\t%d,\n"                     # how each s[] byte is laid out

TAIL_SRC = "thompson-tail.c"
OUT = "thompson.c"


def build(tail_src: str) -> str:
    tail = ARRAY_CLOSE + tail_src
    body = "".join(LINE % b for b in tail.encode("ascii"))
    return HEADER + body + tail


def main() -> None:
    with open(TAIL_SRC, "r", encoding="ascii") as f:
        tail_src = f.read()

    quine = build(tail_src)

    with open(OUT, "w", encoding="ascii") as f:
        f.write(quine)

    print(f"wrote {OUT} ({len(quine)} bytes) from {TAIL_SRC}")


if __name__ == "__main__":
    main()

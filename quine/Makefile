# 
# Thompson quine for the RomHack Camp 2026 Thompson workshop.
#
# assume we have a working tinycc compiler already built in ../tinycc
#
CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -O2
LDLIBS  := -lcrypt

TCC     ?= ../tinycc/tcc
TCCFLAGS := -B../tinycc -I../tinycc/include

.PHONY: all clean tcc

all: quine

quine: thompson.c
	$(CC) $(CFLAGS) -o thompson $< $(LDLIBS)

quine2: thompson2.c
	$(CC) $(CFLAGS) -o thompson2 $< $(LDLIBS)

# build with the compiled tinycc in ../tinycc
tcc: thompson.c
	$(TCC) $(TCCFLAGS) -o thompson $< $(LDLIBS)

tcc2: thompson2.c
	$(TCC) $(TCCFLAGS) -o thompson2 $< $(LDLIBS)

clean:
	rm -f thompson

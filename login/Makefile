# 
# compile login for the RomHack Thompson workshop.
#
# assume we have a working tinycc compiler already built in ../tinycc
#
CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -O2
LDLIBS  := -lcrypt

TCC     ?= ../tinycc/tcc
TCCFLAGS := -B../tinycc -I../tinycc/include

.PHONY: all clean tcc

all: login

login: login.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

# build with the compiled tinycc in ../tinycc
tcc: login.c
	$(TCC) $(TCCFLAGS) -o login $< $(LDLIBS)

clean:
	rm -f login login-tcc

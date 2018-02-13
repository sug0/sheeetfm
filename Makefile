# config
CC=cc
CFLAGS=-Wall -O2
LDFLAGS=-ltermbox
DEPS=$(wildcard src/*.h)
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)
OBJ_D=src/sheeetfmd.o
OBJ_C=$(filter-out $(OBJ_D), $(OBJ))
PREFIX=/usr/local

all: executables

src/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

executables: sheeetfm sheeetfmd

sheeetfm: $(OBJ_C)
	$(CC) -o $@ $(OBJ_C) $(LDFLAGS)

sheeetfmd: $(OBJ_D)
	$(CC) -o $@ $(OBJ_D)

install: all uninstall
	cp -f sheeetfm* $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/sheeetfm*

.PHONY: clean

clean:
	rm -f sheeetfm*
	rm -f src/*.o

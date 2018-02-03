# config
CC=cc
CFLAGS=-Wall -O2
LDFLAGS=-ltermbox
LDFLAGS2=-lmill
DEPS=$(wildcard src/*.h)
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)
OBJ_C=$(filter-out src/sheeetfmd.o, $(OBJ))
OBJ_D=src/sheeetfmd.o
PREFIX=/usr/local

all: executables
	mkdir -p bin
	mv sheeetfm sheeetfmd bin

src/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

executables: sheeetfm sheeetfmd

sheeetfm: $(OBJ_C)
	$(CC) -o sheeetfm $(OBJ_C) $(LDFLAGS)

sheeetfmd: $(OBJ_D)
	$(CC) -o sheeetfmd $(OBJ_D) $(LDFLAGS2)

install: all
	cp bin/* $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/sheeetfm*

.PHONY: clean

clean:
	rm -rf bin/
	rm -f src/*.o

version = 1

srcdir = .
VPATH = $(srcdir)

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

ldlibs = $(LDLIBS) -lImlib2 -lmagic -lm -lexif
exes = bin/stiv_draw bin/stiv_clear bin/fifo_write_nonblock

all: release

clang: clean
clang: CFLAGS += -Weverything -Wno-unsafe-buffer-usage -Wno-format-nonliteral
clang:
	$(MAKE) CC=clang CFLAGS="$(CFLAGS)" release

debug: clean
debug: CFLAGS += -g
debug:
	$(MAKE) CFLAGS="$(CFLAGS)" $(exes)

CFLAGS += -std=c99 -D_DEFAULT_SOURCE
release: CFLAGS += -O2 -flto -Wall -Wextra
release: bin/stiv_draw bin/stiv_clear bin/fifo_write_nonblock
release: $(exes)

.PHONY: all clean install uninstall release debug clang
.SUFFIXES:
.SUFFIXES: .c .o

depends = Makefile stiv.h util.c bin

bin:
	mkdir bin

bin/stiv_draw: $(depends) stiv_draw.c
	-ctags --kinds-C=+l *.h *.c
	-vtags.sed tags > .tags.vim
	$(CC) $(CFLAGS) $(LDFLAGS) $(ldlibs) -o $@ stiv_draw.c util.c

bin/stiv_clear: $(depends) stiv_clear.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(ldlibs) -o $@ stiv_clear.c util.c 

bin/fifo_write_nonblock: $(depends) fifo_write_nonblock.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(ldlibs) -o $@ fifo_write_nonblock.c util.c

clean:
	rm -f bin/*

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f bin/stiv_draw $(DESTDIR)$(PREFIX)/bin/
	cp -f bin/stiv_clear $(DESTDIR)$(PREFIX)/bin/
	cp -f bin/fifo_write_nonblock $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/stiv_draw
	chmod 755 $(DESTDIR)$(PREFIX)/bin/stiv_clear
	chmod 755 $(DESTDIR)$(PREFIX)/bin/fifo_write_nonblock

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/stiv_draw
	rm -f $(DESTDIR)$(PREFIX)/bin/stiv_clear
	rm -f $(DESTDIR)$(PREFIX)/bin/fifo_write_nonblock

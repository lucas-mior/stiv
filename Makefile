version = 1

srcdir = .
VPATH = $(srcdir)

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC=clang

ldlibs = $(LDLIBS) -lImlib2 -lmagic -lm

src = main.c util.c

all: release

CFLAGS += -std=c99 -D_DEFAULT_SOURCE
release: CFLAGS += -O2 -flto -Weverything -Wno-unsafe-buffer-usage
release: stiv

clang: CC=clang
clang: clean release
clang: CFLAGS += -Weverything -Wno-unsafe-buffer-usage

debug: CFLAGS += -g -Weverything
debug: clean stiv

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

stiv: $(src) Makefile stiv.h clear.c fifo_write_nonblock.c
	ctags --kinds-C=+l *.h *.c
	vtags.sed tags > .tags.vim
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(src) $(ldlibs)
	$(CC) $(CFLAGS) $(LDFLAGS) -o stiv_clear clear.c $(ldlibs)
	$(CC) $(CFLAGS) $(LDFLAGS) -o fifo_write_nonblock fifo_write_nonblock.c $(ldlibs)

clean:
	rm -f *.o stiv

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	rm -f $(DESTDIR)$(PREFIX)/bin/stiv
	rm -f $(DESTDIR)$(PREFIX)/bin/stiv_clear
	cp stiv $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/stiv
	cp stiv_clear $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/stiv_clear
	cp fifo_write_nonblock $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/fifo_write_nonblock
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/stiv.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/stiv
	rm -f $(DESTDIR)$(MANPREFIX)/man1/stiv.1

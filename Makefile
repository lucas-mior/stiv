version = 1

srcdir = .
VPATH = $(srcdir)

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC=clang

ldlibs = $(LDLIBS) -lImlib2 -lmagic -lm -lexif

all: release

clang: CC=clang
clang: clean release
clang: CFLAGS += -Weverything -Wno-unsafe-buffer-usage

debug: CFLAGS += -g -Weverything
debug: clean bin/stiv_draw bin/stiv_clear bin/fifo_write_nonblock

CFLAGS += -std=c99 -D_DEFAULT_SOURCE
release: CFLAGS += -O2 -flto -Weverything -Wno-unsafe-buffer-usage -Wno-format-nonliteral
release: bin/stiv_draw bin/stiv_clear bin/fifo_write_nonblock

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

bin/stiv_draw: Makefile stiv.h stiv_draw.c
	-ctags --kinds-C=+l *.h *.c
	-vtags.sed tags > .tags.vim
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ stiv_draw.c util.c $(ldlibs)

bin/stiv_clear: Makefile stiv.h stiv_clear.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ stiv_clear.c util.c $(ldlibs)

bin/fifo_write_nonblock: Makefile stiv.h stiv_clear.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ fifo_write_nonblock.c util.c $(ldlibs)

clean:
	rm -f *.o stiv_draw stiv_clear fifo_write_nonblock

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

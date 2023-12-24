version = 1

srcdir = .
VPATH = $(srcdir)

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

ldlibs = $(LDLIBS) -lImlib2 -lmagic -lm -lexif

all: release

clang: clean
clang: CFLAGS += -Weverything -Wno-unsafe-buffer-usage -Wno-format-nonliteral
clang:
	$(MAKE) CC=clang CFLAGS="$(CFLAGS)" release

debug: clean
debug: CC=clang
debug: CFLAGS += -g -Weverything -Wno-unsafe-buffer-usage -Wno-format-nonliteral
debug:
	$(MAKE) CC=clang CFLAGS="$(CFLAGS)" bin/stiv_draw bin/stiv_clear bin/fifo_write_nonblock

CFLAGS += -std=c99 -D_DEFAULT_SOURCE
release: CFLAGS += -O2 -flto -Wall -Wextra
release: bin/stiv_draw bin/stiv_clear bin/fifo_write_nonblock

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

depends = Makefile stiv.h bin/util.o

bin/util.o: Makefile stiv.h util.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c util.c

bin/stiv_clear_lib.o: $(depends) stiv_clear_lib.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c stiv_clear_lib.c

bin/stiv_draw: $(depends) stiv_draw.c bin/stiv_clear_lib.o
	-ctags --kinds-C=+l *.h *.c
	-vtags.sed tags > .tags.vim
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ stiv_draw.c bin/stiv_clear_lib.o bin/util.o $(ldlibs)

bin/stiv_clear: $(depends) stiv_draw.c bin/stiv_clear_lib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ stiv_clear.c bin/stiv_clear_lib.o bin/util.o $(ldlibs)

bin/fifo_write_nonblock: $(depends) stiv_draw.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ fifo_write_nonblock.c bin/util.o $(ldlibs)

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

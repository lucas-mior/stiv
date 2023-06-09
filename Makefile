version = 1

srcdir = .
VPATH = $(srcdir)

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC=clang
# CC=gcc
cflags = $(CFLAGS)
cppflags = $(CPPFLAGS)

ldlibs = $(LDLIBS) -lImlib2 -lmagic -lm

objs = image.o main.o util.o cursor.o clear.o

all: stiv

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

stiv: $(objs)
	ctags --kinds-C=+l *.h *.c
	vtags.sed tags > .tags.vim
	$(CC) $(cflags) $(LDFLAGS) -o $@ $(objs) $(ldlibs)

$(objs): Makefile stiv.h

.c.o:
	$(CC) $(cflags) $(cppflags) -c -o $@ $<

clean:
	rm -f *.o stiv

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	rm -f $(DESTDIR)$(PREFIX)/bin/stiv
	cp stiv $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/stiv
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed 's!PREFIX!$(PREFIX)!g; s!VERSION!$(version)!g' stiv.1 > $(DESTDIR)$(MANPREFIX)/man1/stiv.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/stiv.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/stiv
	rm -f $(DESTDIR)$(MANPREFIX)/man1/stiv.1

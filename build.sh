#!/bin/sh

target="${1:-build}"
PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-/}"

main1="fifo_write_nonblock.c"
main2="stiv_clear.c"
main3="stiv_draw.c"
program1="fifo_write_nonblock"
program2="stiv_clear"
program3="stiv_draw"

CFLAGS="$CFLAGS -std=c99 -D_DEFAULT_SOURCE "
CFLAGS="$CFLAGS -Wextra -Wall -Wno-unused-macros -Wno-unused-function"
CFLAGS="$CFLAGS -Wno-constant-logical-operand"
LDFLAGS="$LDFLAGS -lm -lImlib2 -lmagic -lm -lexif "

CC=${CC:-cc}
if [ $CC = "clang" ]; then
    CFLAGS="$CFLAGS -Weverything "
    CFLAGS="$CFLAGS -Wno-unsafe-buffer-usage -Wno-format-nonliteral "
    CFLAGS="$CFLAGS -Wno-disabled-macro-expansion "
fi

if [ "$target" = "debug" ]; then
    CFLAGS="$CFLAGS -g -fsanitize=undefined "
    CPPFLAGS="$CPPFLAGS "
else
    CFLAGS="$CFLAGS -g -O2 -flto"
    CPPFLAGS="$CPPFLAGS "
fi

if [ "$target" = "benchmark" ]; then
    CPPFLAGS="$CPPFLAGS "
fi

case "$target" in
"uninstall")
    set -x
    rm -f ${DESTDIR}${PREFIX}/bin/${program1}
    rm -f ${DESTDIR}${PREFIX}/bin/${program2}
    rm -f ${DESTDIR}${PREFIX}/bin/${program3}
    ;;
"install")
    if [ ! -f $program1 ] || [ ! -f $program2 ] || [ ! -f $program3 ]; then
        build
    fi
    set -x
    install -Dm755 ${program1} ${DESTDIR}${PREFIX}/bin/${program1}
    install -Dm755 ${program2} ${DESTDIR}${PREFIX}/bin/${program2}
    install -Dm755 ${program3} ${DESTDIR}${PREFIX}/bin/${program3}
    ;;
"build"|"debug"|"benchmark")
    set -x
    ctags --kinds-C=+l *.h *.c 2> /dev/null || true
    vtags.sed tags > .tags.vim 2> /dev/null || true
    $CC $CPPFLAGS $CFLAGS -o ${program1} "$main1" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o ${program2} "$main2" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o ${program3} "$main3" $LDFLAGS
    ;;
*)
    echo "usage: $0 [ uninstall / install / build / debug / benchmark ]"
    ;;
esac

#!/bin/sh -e

# shellcheck disable=SC2086

dir=$(dirname "$(readlink -f "$0")")
cd "$dir" || exit

# shellcheck source=./cbase/common.sh
. "./cbase/common.sh"

script=$(basename "$0")
common_build_parse_args "$@"

case "$mode" in
build|check|debug|debug-fast|fast_feedback|install|test|uninstall)
    ;;
*)
    common_build_unknown_mode
    ;;
esac

common_build_print_invocation "$script"

PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-/}"

main1="fifo_write_nonblock.c"
main2="stiv_clear.c"
main3="stiv_draw.c"
program1="fifo_write_nonblock"
program2="stiv_clear"
program3="stiv_draw"
mkdir -p bin

CC=$(common_get_compiler "$mode")

CPPFLAGS="$CPPFLAGS -Icbase"

CFLAGS="$CFLAGS -std=c11"
CFLAGS="$CFLAGS -Wfatal-errors"

PKG_CONFIG="${PKG_CONFIG:-pkg-config}"
build_deps="imlib2 libmagic libexif"

case "$mode" in
build|check|debug|debug-fast|fast_feedback|test)
    CPPFLAGS="$CPPFLAGS $($PKG_CONFIG --cflags $build_deps)"
    LDFLAGS="$LDFLAGS -lm $($PKG_CONFIG --libs $build_deps)"
    ;;
install|uninstall)
    ;;
esac

case "$mode" in
debug)
    CFLAGS="$CFLAGS -g3 -Og"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
    ;;
debug-fast)
    CFLAGS="$CFLAGS -g2 -O2 -flto -march=native -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
    ;;
build)
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    ;;
fast_feedback)
    ;;
test|install|uninstall)
    ;;
build|check|debug|debug-fast|fast_feedback|install|test|uninstall)
    ;;
*)
    common_build_unknown_mode
    ;;
esac

case "$mode" in
test)
    TEST_EXCLUDE_PATTERN='(^|/)cbase/' common_test "$target"
    exit
    ;;
uninstall)
    trace_on
    rm -f "${DESTDIR}${PREFIX}/bin/${program1}"
    rm -f "${DESTDIR}${PREFIX}/bin/${program2}"
    rm -f "${DESTDIR}${PREFIX}/bin/${program3}"
    trace_off
    ;;
install)
    if [ ! -f "bin/$program1" ] || [ ! -f "bin/$program2" ] || [ ! -f "bin/$program3" ]; then
        "$0" build
    fi
    trace_on
    install -Dm755 "bin/${program1}" "${DESTDIR}${PREFIX}/bin/${program1}"
    install -Dm755 "bin/${program2}" "${DESTDIR}${PREFIX}/bin/${program2}"
    install -Dm755 "bin/${program3}" "${DESTDIR}${PREFIX}/bin/${program3}"
    trace_off
    ;;
check)
    common_build_run_analyzers build
    ;;
build|debug|debug-fast|fast_feedback)
    common_build_tags
    trace_on
    $CC $CPPFLAGS $CFLAGS -o "bin/${program1}" "$main1" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o "bin/${program2}" "$main2" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o "bin/${program3}" "$main3" $LDFLAGS
    trace_off
    ;;
esac

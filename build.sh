#!/bin/sh -e

# shellcheck disable=SC2086

dir=$(dirname "$(readlink -f "$0")")
cd "$dir" || exit

# shellcheck source=./cbase/common.sh
. "./cbase/common.sh"

script=$(basename "$0")
common_build_parse_args "$@"

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

CPPFLAGS="$CPPFLAGS -I$dir/cbase"

CFLAGS="$CFLAGS -std=c11"
CFLAGS="$CFLAGS -Wfatal-errors"
CFLAGS="$CFLAGS -Wextra -Wall"
CFLAGS="$CFLAGS -Werror=all -Werror=extra"
CFLAGS="$CFLAGS -Werror"  # Only uncomment occasionally, keep this line

if [ "$CC" = "clang" ]; then
    CFLAGS="$CFLAGS -Weverything"
    CFLAGS="$CFLAGS -Wno-assign-enum"
    CFLAGS="$CFLAGS -Wno-c++-keyword"
    CFLAGS="$CFLAGS -Wno-cast-qual"
    CFLAGS="$CFLAGS -Wno-constant-logical-operand"
    CFLAGS="$CFLAGS -Wno-covered-switch-default"
    CFLAGS="$CFLAGS -Wno-disabled-macro-expansion"
    CFLAGS="$CFLAGS -Wno-float-equal"
    CFLAGS="$CFLAGS -Wno-format-nonliteral"
    CFLAGS="$CFLAGS -Wno-implicit-int-enum-cast"
    CFLAGS="$CFLAGS -Wno-implicit-void-ptr-cast"
    CFLAGS="$CFLAGS -Wno-padded"
    CFLAGS="$CFLAGS -Wno-pre-c11-compat"
    CFLAGS="$CFLAGS -Wno-unsafe-buffer-usage"
    CFLAGS="$CFLAGS -Wno-unused-macros"
    CFLAGS="$CFLAGS -Wno-used-but-marked-unused"
fi

LDFLAGS="$LDFLAGS -lm -lImlib2 -lmagic -lexif"

case "$mode" in
debug)
    CFLAGS="$CFLAGS -g3 -Og -fsanitize=undefined"
    CPPFLAGS="$CPPFLAGS -DDEBUGGING=1"
    ;;
benchmark)
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    ;;
build)
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    ;;
fast_feedback)
    ;;
test|install|uninstall)
    ;;
*)
    CFLAGS="$CFLAGS -O2"
    ;;
esac

build_programs () {
    common_build_tags
    trace_on
    $CC $CPPFLAGS $CFLAGS -o "bin/${program1}" "$main1" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o "bin/${program2}" "$main2" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o "bin/${program3}" "$main3" $LDFLAGS
    trace_off
}

case "$mode" in
fast_feedback)
    build_programs
    ;;
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
    set +e

    CC=gcc CFLAGS="-fanalyzer -fdiagnostics-color=never" "$0" build

    CFLAGS="--analyze -Xanalyzer -analyzer-output=text"
    CFLAGS="$CFLAGS -Xanalyzer -analyzer-werror"
    CFLAGS="$CFLAGS -Xanalyzer -analyzer-opt-analyze-headers"
    CFLAGS="$CFLAGS -Wno-unused-command-line-argument"
    CFLAGS="$CFLAGS -fno-color-diagnostics"
    CC=clang CFLAGS="$CFLAGS" "$0" build

    exit
    ;;
*)
    build_programs
    ;;
esac

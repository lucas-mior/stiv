#!/bin/sh -e

# shellcheck disable=SC2086

dir=$(dirname "$(readlink -f "$0")")
# shellcheck source=/dev/null
. "$dir/cbase/common.sh"

cd "$dir" || exit
script=$(basename "$0")
target="${1:-debug}"

printf "\n${script} ${RED}${1:-} ${2:-}$RES\n"

PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-/}"

main1="fifo_write_nonblock.c"
main2="stiv_clear.c"
main3="stiv_draw.c"
program1="fifo_write_nonblock"
program2="stiv_clear"
program3="stiv_draw"
mkdir -p bin

case "$target" in
debug|test)
    CC="${CC:-tcc}"
    ;;
fast_feedback)
    CC="${CC:-clang}"
    ;;
*)
    CC="${CC:-cc}"
    ;;
esac

if ! command -v "$CC" > /dev/null 2>&1; then
    CC=cc
fi

CPPFLAGS="$CPPFLAGS -I$dir/cbase"
CPPFLAGS="$CPPFLAGS -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700"

CFLAGS="$CFLAGS -std=c11"
CFLAGS="$CFLAGS -Wfatal-errors"
CFLAGS="$CFLAGS -Wextra -Wall"
CFLAGS="$CFLAGS -Werror=all -Werror=extra"
# CFLAGS="$CFLAGS -Werror"  # Only uncomment occasionally, keep this line
CFLAGS="$CFLAGS -Wno-cast-qual"
CFLAGS="$CFLAGS -Wno-constant-logical-operand"
CFLAGS="$CFLAGS -Wno-float-equal"
CFLAGS="$CFLAGS -Wno-format-pedantic"
CFLAGS="$CFLAGS -Wno-gnu-union-cast"
CFLAGS="$CFLAGS -Wno-padded"
CFLAGS="$CFLAGS -Wno-undefined-internal"
CFLAGS="$CFLAGS -Wno-unknown-pragmas"
CFLAGS="$CFLAGS -Wno-unknown-warning-option"
CFLAGS="$CFLAGS -Wno-unused-macros"

if [ "$CC" = "clang" ]; then
    CFLAGS="$CFLAGS -Weverything"
    CFLAGS="$CFLAGS -Wno-assign-enum"
    CFLAGS="$CFLAGS -Wno-bad-function-cast"
    CFLAGS="$CFLAGS -Wno-c++-keyword"
    CFLAGS="$CFLAGS -Wno-covered-switch-default"
    CFLAGS="$CFLAGS -Wno-disabled-macro-expansion"
    CFLAGS="$CFLAGS -Wno-format-nonliteral"
    CFLAGS="$CFLAGS -Wno-ignored-attributes"
    CFLAGS="$CFLAGS -Wno-implicit-int-enum-cast"
    CFLAGS="$CFLAGS -Wno-implicit-void-ptr-cast"
    CFLAGS="$CFLAGS -Wno-pre-c11-compat"
    CFLAGS="$CFLAGS -Wno-unsafe-buffer-usage"
    CFLAGS="$CFLAGS -Wno-used-but-marked-unused"
fi

LDFLAGS="$LDFLAGS -lm -lImlib2 -lmagic -lexif"

OS=$(uname -a)
GNUSOURCE=
if echo "$OS" | grep -q "Linux"; then
    if echo "$OS" | grep -q "GNU"; then
        GNUSOURCE="-D_GNU_SOURCE"
    fi
fi

case "$target" in
debug)
    CFLAGS="$CFLAGS -g3 -O0 -fsanitize=undefined"
    CPPFLAGS="$CPPFLAGS $GNUSOURCE -DDEBUGGING=1"
    ;;
benchmark)
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS $GNUSOURCE"
    ;;
build)
    CFLAGS="$CFLAGS $GNUSOURCE -O2 -flto -march=native -ftree-vectorize"
    ;;
fast_feedback)
    CFLAGS="$CFLAGS $GNUSOURCE"
    ;;
test|install|uninstall)
    ;;
*)
    CFLAGS="$CFLAGS -O2"
    ;;
esac

build_programs () {
    build_tags
    trace_on
    $CC $CPPFLAGS $CFLAGS -o "bin/${program1}" "$main1" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o "bin/${program2}" "$main2" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o "bin/${program3}" "$main3" $LDFLAGS
    trace_off
}

case "$target" in
fast_feedback)
    build_programs
    ;;
test)
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

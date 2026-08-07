#!/bin/sh -e

# shellcheck disable=SC2086

set -e

error () {
    >&2 printf "$@"
    return
}

if [ -n "$BASH_VERSION" ]; then
    # shellcheck disable=SC3044
    shopt -s expand_aliases
fi

alias trace_on='set -x'
alias trace_off='{ set +x; } 2>/dev/null'

dir=$(dirname "$(readlink -f "$0")")
CPPFLAGS="$CPPFLAGS -I$dir/cbase"
cd "$dir" || exit
program=$(basename "$(readlink -f "$(dirname "$0")")")
script=$(basename "$0")
target="${1:-build}"

if [ "$target" = "test" ]; then
    exit
fi

printf "
${script} ${RED}${1:-} ${2:-}$RES
"

PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-/}"

main1="fifo_write_nonblock.c"
main2="stiv_clear.c"
main3="stiv_draw.c"
program1="fifo_write_nonblock"
program2="stiv_clear"
program3="stiv_draw"
mkdir -p bin

CPPFLAGS="$CPPFLAGS -D_DEFAULT_SOURCE"
CFLAGS="$CFLAGS -std=c11"
CFLAGS="$CFLAGS -Wfatal-errors"
CFLAGS="$CFLAGS -Wextra -Wall"
CFLAGS="$CFLAGS -Werror"
CFLAGS="$CFLAGS -Wno-format-pedantic"
CFLAGS="$CFLAGS -Wno-unknown-warning-option"
CFLAGS="$CFLAGS -Wno-gnu-union-cast"
CFLAGS="$CFLAGS -Wno-unused-macros"
CFLAGS="$CFLAGS -Wno-constant-logical-operand"
CFLAGS="$CFLAGS -Wno-float-equal"
CFLAGS="$CFLAGS -Wno-undefined-internal"
CFLAGS="$CFLAGS -Wno-cast-qual"
CFLAGS="$CFLAGS -Wno-unknown-pragmas"
CPPFLAGS="$CPPFLAGS -D_XOPEN_SOURCE=700"
CFLAGS="$CFLAGS -Wno-padded"
LDFLAGS="$LDFLAGS -lm -lImlib2 -lmagic -lexif"

OS=$(uname -a)
GNUSOURCE=
if echo "$OS" | grep -q "Linux"; then
    if echo "$OS" | grep -q "GNU"; then
        GNUSOURCE="-D_GNU_SOURCE"
    fi
fi

requested_cc=${CC:-}
case "$target" in
"debug"|"test"|"fast_feedback")
    CC="${requested_cc:-tcc}"
    ;;
*)
    CC="${requested_cc:-cc}"
    ;;
esac
if [ "$CC" = "clang" ]; then
    CFLAGS="$CFLAGS -Weverything"
    CFLAGS="$CFLAGS -Wno-unsafe-buffer-usage"
    CFLAGS="$CFLAGS -Wno-format-nonliteral"
    CFLAGS="$CFLAGS -Wno-disabled-macro-expansion"
    CFLAGS="$CFLAGS -Wno-c++-keyword"
    CFLAGS="$CFLAGS -Wno-pre-c11-compat"
    CFLAGS="$CFLAGS -Wno-implicit-void-ptr-cast"
    CFLAGS="$CFLAGS -Wno-ignored-attributes"
    CFLAGS="$CFLAGS -Wno-covered-switch-default"
    CFLAGS="$CFLAGS -Wno-used-but-marked-unused"
    CFLAGS="$CFLAGS -Wno-implicit-int-enum-cast"
    CFLAGS="$CFLAGS -Wno-assign-enum"
    CFLAGS="$CFLAGS -Wno-cast-function-type-strict"
    CFLAGS="$CFLAGS -Wno-bad-function-cast"
fi
case "$target" in
"debug")
    CFLAGS="$CFLAGS -g3 -O0 -fsanitize=undefined"
    CPPFLAGS="$CPPFLAGS $GNUSOURCE -DDEBUGGING=1"
    ;;
"benchmark")
    CFLAGS="$CFLAGS -O2 -flto -march=native -ftree-vectorize"
    CPPFLAGS="$CPPFLAGS $GNUSOURCE"
    ;;
"build")
    CFLAGS="$CFLAGS $GNUSOURCE -g3 -O2 -flto -march=native -ftree-vectorize"
    ;;
"fast_feedback")
    CFLAGS="$CFLAGS $GNUSOURCE -Werror"
    ;;
"test"|"install"|"uninstall")
    ;;
*)
    CFLAGS="$CFLAGS -O2"
    ;;
esac

build_tags () {
    if command -v ctags >/dev/null 2>&1; then
        find . -iname "*.[ch]" -print0             | xargs -0 ctags --kinds-C=+l+d 2> /dev/null || true
    fi

    if [ -f tags ] && command -v vtags.sed >/dev/null 2>&1; then
        vtags.sed tags | sort | uniq > .tags.vim 2> /dev/null || true
    fi
}

install_opt () {
    mode="$1"
    file="$2"
    dest="$3"

    if [ -f "$file" ]; then
        install "$mode" "$file" "$dest"
    elif [ -d "$file" ]; then
        install "$mode" "$dest"
        cp -rp "$file/." "$dest/"
    fi
}

uninstall_opt () {
    file="$1"
    dest="$2"

    if [ -e "$file" ]; then
        rm -rf "$dest"
    fi
}
build_programs () {
    build_tags
    trace_on
    $CC $CPPFLAGS $CFLAGS -o "bin/${program1}" "$main1" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o "bin/${program2}" "$main2" $LDFLAGS
    $CC $CPPFLAGS $CFLAGS -o "bin/${program3}" "$main3" $LDFLAGS
    trace_off
}

case "$target" in
"fast_feedback")
    build_programs
    ;;
"test")
    exit
    ;;
"uninstall")
    trace_on
    rm -f "${DESTDIR}${PREFIX}/bin/${program1}"
    rm -f "${DESTDIR}${PREFIX}/bin/${program2}"
    rm -f "${DESTDIR}${PREFIX}/bin/${program3}"
    trace_off
    ;;
"install")
    if [ ! -f "bin/$program1" ] || [ ! -f "bin/$program2" ] || [ ! -f "bin/$program3" ]; then
        "$0" build
    fi
    trace_on
    install -Dm755 "bin/${program1}" "${DESTDIR}${PREFIX}/bin/${program1}"
    install -Dm755 "bin/${program2}" "${DESTDIR}${PREFIX}/bin/${program2}"
    install -Dm755 "bin/${program3}" "${DESTDIR}${PREFIX}/bin/${program3}"
    trace_off
    ;;
"check")
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

#if !defined(STIV_H)
#define STIV_H

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <linux/limits.h>
#include <magic.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define MAX_CACHE_WIDTH 1024.0
#define MAX_IMG_WIDTH 2048
#define MAX_PNG_WIDTH 1536
#define HEIGHT_SHELL 11

typedef struct File {
    FILE *file;
    char *name;
    int fd;
    int unused;
} File;

typedef union Number {
    char *string;
    int number;
} Number;

#define UEBERZUG_CLEAR "{\"action\": \"remove\", \"identifier\": \"preview\"}\n"

#endif /* STIV_H */

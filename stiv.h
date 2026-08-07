#if !defined(STIV_H)
#define STIV_H

#include "cbase.h"

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

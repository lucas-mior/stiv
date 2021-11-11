#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <sys/sysmacros.h>

char * cache(char *filename) {
    struct stat file;
    char *cache = NULL;

    if (lstat(filename, &file) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    if (!(cache = malloc(256)))
        return NULL;

    snprintf(cache, 256, "%li_%ld_%ld", file.st_size, file.st_mtim.tv_sec, file.st_mtim.tv_nsec);
    return cache;
}

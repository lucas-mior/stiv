#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "stiv.h"
#include "util.h"

const char *program;

int estrtol(char *string) {
    int number = 0;
    char *end_pointer;
    number = (int) strtol(string, &end_pointer, 10);
    if ((number < 0) || (end_pointer == string)) {
        fprintf(stderr, "%i != stroul(\"%s\")\n", number, string);
        exit(1);
    }
    return number;
}

void *emalloc(size_t size) {
    void *ptr;
    if (!(ptr = malloc(size)))
        error(strerror(errno));
    return ptr;
}

char *egetenv(char *variable) {
    char *pointer;
    if (!(pointer = getenv(variable))) {
        fprintf(stderr, "%s is not set, exiting\n", variable);
        exit(1);
    }
    return pointer;
}

FILE *efopen(char *filename, char *mode) {
    FILE *file;
    if (!(file = fopen(filename, mode))) {
        fprintf(stderr, "Could not open file %s : %s\n", filename, strerror(errno));
        exit(1);
    }
    return file;
}

int ends_with(const char *str, const char *end) {
    const char *ldot = strrchr(str, '.');
    int length = 0;
    if (ldot != NULL) {
        length = strlen(end);
        return !strncmp(ldot + 1, end, length);
    }
    return 0;
}

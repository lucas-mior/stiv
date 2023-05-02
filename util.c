#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "stiv.h"

const char *program;

int32 util_string_int32(const char *string) {
    char *endptr;
	const int base = 10;	
    long x;
    errno = 0;
    x = strtol(string, &endptr, base);
    if ((errno != 0) || (string == endptr) || (*endptr != 0)) {
        return false;
    } else if ((x > INT32_MAX) || (x < INT32_MIN)) {
		fprintf(stderr, "Error converting to integer: %s\n", string);
		exit(EXIT_FAILURE);
    }
    return (int32) x;
}

void *util_realloc(void *old, const size_t size) {
    void *p;
    if ((p = realloc(old, size)) == NULL) {
        fprintf(stderr, "Failed to allocate %zu bytes.\n", size);
        if (old)
            fprintf(stderr, "Reallocating from: %p\n", old);
        exit(EXIT_FAILURE);
    }
    return p;
}

void *util_calloc(const size_t nmemb, const size_t size) {
    void *p;
    if ((p = calloc(nmemb, size)) == NULL) {
        fprintf(stderr, "Failed to allocate %zu members of %zu bytes each.\n",
                        nmemb, size);
        exit(EXIT_FAILURE);
    }
    return p;
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

bool ends_with(const char *str, const char *end) {
    const char *ldot = strrchr(str, '.');
    size_t length = 0;
    if (ldot != NULL) {
        length = strlen(end);
        return !strncmp(ldot + 1, end, length);
    }
    return false;
}

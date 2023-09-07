/* This file is part of stiv. */
/* Copyright (C) 2022 Lucas Mior */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include "stiv.h"

int32 util_string_int32(const char *string) {
    char *endptr;
	const int base = 10;	
    long x;
    errno = 0;
    x = strtol(string, &endptr, base);
    if ((errno != 0) || (string == endptr) || (*endptr != 0)) {
		fprintf(stderr, "Error converting to integer: %s\n", string);
		exit(EXIT_FAILURE);
    } else if ((x > INT32_MAX) || (x < INT32_MIN)) {
		fprintf(stderr, "Error converting to integer: %s\n", string);
		exit(EXIT_FAILURE);
    }
    return (int32) x;
}

void *util_malloc(const size_t size) {
    void *p;
    if ((p = malloc(size)) == NULL) {
        fprintf(stderr, "Failed to allocate %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    return p;
}

void *util_realloc(void *old, const size_t size) {
    void *p;
    if ((p = realloc(old, size)) == NULL) {
        fprintf(stderr, "Failed to allocate %zu bytes.\n", size);
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

bool ends_with(const char *str, const char *end) {
    const char *ldot = strrchr(str, '.');
    size_t length = 0;
    if (ldot != NULL) {
        length = strlen(end);
        return !strncmp(ldot + 1, end, length);
    }
    return false;
}

void util_close(File *f) {
    if (f->fd >= 0) {
        if (close(f->fd) < 0) {
            fprintf(stderr, "Error closing %s: %s\n",
                            f->name, strerror(errno));
        }
        f->fd = -1;
    }
    if (f->file != NULL) {
        if (fclose(f->file) != 0) {
            fprintf(stderr, "Error closing %s: %s\n",
                            f->name, strerror(errno));
        }
        f->file = NULL;
    }
    return;
}

bool util_open(File *f, const int flag) {
    if ((f->fd = open(f->name, flag)) < 0) {
        fprintf(stderr, "Error opening %s: %s\n",
                        f->name, strerror(errno));
        return false;
    } else {
        return true;
    }
}

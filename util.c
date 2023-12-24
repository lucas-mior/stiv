/* This file is part of stiv.
 * Copyright (C) 2022 Lucas Mior

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stiv.h"

int32
util_string_int32(const char *string) {
    char *endptr;
	const int base = 10;	
    long x;
    errno = 0;
    x = strtol(string, &endptr, base);
    if ((errno != 0) || (string == endptr) || (*endptr != 0)) {
		error("Error converting to integer: %s\n", string);
		exit(EXIT_FAILURE);
    } else if ((x > INT32_MAX) || (x < INT32_MIN)) {
		error("Error converting to integer: %s\n", string);
		exit(EXIT_FAILURE);
    }
    return (int32) x;
}

void *
util_malloc(const usize size) {
    void *p;
    if ((p = malloc(size)) == NULL) {
        error("Error allocating %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    return p;
}

char *
util_strdup(const char *str) {
    void *p;
    size_t size = strlen(str) + 1;

    if ((p = malloc(size)) == NULL) {
        error("Error allocating %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    memcpy(p, str, size);

    return p;
}

void *
util_realloc(void *old, const usize size) {
    void *p;
    if ((p = realloc(old, size)) == NULL) {
        error("Error allocating %zu bytes.\n", size);
        error("Reallocating from: %p\n", old);
        exit(EXIT_FAILURE);
    }
    return p;
}

void *
util_calloc(const usize nmemb, const usize size) {
    void *p;
    if ((p = calloc(nmemb, size)) == NULL) {
        error("Error allocating %zu members of %zu bytes each.\n", nmemb, size);
        exit(EXIT_FAILURE);
    }
    return p;
}

bool
ends_with(const char *str, const char *end) {
    const char *ldot = strrchr(str, '.');
    usize length = 0;
    if (ldot != NULL) {
        length = strlen(end);
        return !strncmp(ldot + 1, end, length);
    }
    return false;
}

void
util_close(File *f) {
    if (f->fd >= 0) {
        if (close(f->fd) < 0)
            error("Error closing %s: %s\n", f->name, strerror(errno));
        f->fd = -1;
    }
    if (f->file != NULL) {
        if (fclose(f->file) != 0)
            error("Error closing %s: %s\n", f->name, strerror(errno));
        f->file = NULL;
    }
    return;
}

bool
util_open(File *f, const int flag) {
    if ((f->fd = open(f->name, flag)) < 0) {
        error("Error opening %s: %s\n", f->name, strerror(errno));
        return false;
    } else {
        return true;
    }
}

void error(char *format, ...) {
    int n;
    va_list args;
    char buffer[BUFSIZ];

    va_start(args, format);
    n = vsnprintf(buffer, sizeof (buffer) - 1, format, args);
    va_end(args);

    if (n < 0) {
        error("Error in vsnprintf()\n");
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0';
    (void) write(STDERR_FILENO, buffer, (usize) n);

#ifdef DEBUGGING
    switch (fork()) {
        char *notifiers[2] = { "dunstify", "notify-send" };
        case -1:
            error("Error forking: %s\n", strerror(errno));
            break;
        case 0:
            for (uint i = 0; i < LENGTH(notifiers); i += 1) {
                execlp(notifiers[i], notifiers[i], "-u", "critical", 
                                     program, buffer, NULL);
            }
            error("Error trying to exec dunstify.\n");
            break;
        default:
            break;
    }
    exit(EXIT_FAILURE);
#endif
}

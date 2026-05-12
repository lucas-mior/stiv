/*
 * Copyright (C) 2024 Lucas Mior

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

static const double MAX_CACHE_WIDTH = 1024;
static const int MAX_IMG_WIDTH = 2048;
static const int MAX_PNG_WIDTH = 1536;
static const int HEIGHT_SHELL = 11;

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

static void util_close(File *f);
static bool util_open(File *f, const int flag);
void
util_close(File *f) {
    if (f->fd >= 0) {
        if (close(f->fd) < 0)
            fprintf(stderr, "Error closing %s: %s\n", f->name, strerror(errno));
        f->fd = -1;
    }
   if (f->file != NULL) {
        if (fclose(f->file) != 0)
            fprintf(stderr, "Error closing %s: %s\n", f->name, strerror(errno));
        f->file = NULL;
   }
   return;
}

#endif /* STIV_H */

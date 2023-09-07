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

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <magic.h>
#include <math.h>
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
#include <time.h>
#include <unistd.h>

#ifndef STIV_H
#define STIV_H

enum {
    CLEAR_DEFAULT,
    CLEAR_ALL,
    CLEAR_PREVIEW,
};

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

static const double CACHE_IMG_WIDTH = 1024;
static const int MAX_IMG_WIDTH = 2048;
static const int MAX_PNG_WIDTH = 1536;
static const int HEIGHT_SHELL = 11;

typedef struct Options {
    int32 w, h, H;
    int32 x, y;
    bool preview;
    bool clear;
    bool print_dim;
    bool unused;
} Options;

typedef struct Image {
    char *basename;
    char *fullpath;
    char *cachename;
    int width, height;
} Image;

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

int32 util_string_int32(const char *);
void *util_realloc(void *, size_t);
bool ends_with(const char *, const char *);
void util_close(File *);
bool util_open(File *, const int);

void clear_display(int);
int cursor_getx(void);

void image_get_size(Image *);
void image_reduce_size(Image *, double);

#endif /* STIV_H */

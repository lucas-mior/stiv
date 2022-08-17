#ifndef STIV_H
#define STIV_H

#define S(x...) #x
#define CLEAR_ALL 0

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

static const uint MAX_IMG_WIDTH = 2000;
static const uint MAX_PNG_WIDTH =  1600;
static const uint CACHE_IMG_WIDTH = 1024;
static const uint HEIGHT_SHELL = 11;
static const uint CLEAR_PREVIEW = 1;

typedef struct {
    uint w, h, H;
    uint x, y;
    bool preview;
    bool clear;
    bool print_dim;
} Options;

typedef struct {
    char *filename;
    char *path;
    char cache[100];
    uint w, h;
} Image;

#endif /* STIV_H */

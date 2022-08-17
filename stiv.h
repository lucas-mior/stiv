#ifndef STIV_H
#define STIV_H

#define S(x...) #x

static const int MAX_IMG_WIDTH = 2000;
static const int MAX_PNG_WIDTH =  1600;
static const int CACHE_IMG_WIDTH = 1024;
static const int HEIGHT_SHELL = 11;
static const int CLEAR_PREVIEW = 1;
static const int CLEAR_ALL = 0;

typedef struct {
    int w, h, H;
    int x, y;
    bool preview;
    bool clear;
    bool print_dim;
} Options;

typedef struct {
    char *filename;
    char *path;
    char cache[100];
    int w, h;
} Image;

#endif /* STIV_H */

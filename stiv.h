#ifndef STIV_H
#define STIV_H

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))
#define HEIGHT_SHELL 11
#define CLEAR_PREVIEW 1
#define CLEAR_ALL 0
#define S(x...) #x
#define MAX_IMG_WIDTH 2000
#define MAX_PNG_WIDTH 1600
#define CACHE_IMG_WIDTH 1024

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

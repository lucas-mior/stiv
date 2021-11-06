#ifndef STIV_H
#define STIV_H

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))
#define HEIGHT_SHELL 12
#define S(x...) #x

typedef struct {
    int w, h, H;
    int x, y;
    bool preview;
    bool clear;
} Options;

typedef struct {
    char *filename;
    char *path;
    char *hash;
    int w, h;
} Image;

#endif /* STIV_H */

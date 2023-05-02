#ifndef STIV_H
#define STIV_H
#include <stdbool.h>
#include <stdio.h>

#define S(x...) #x

typedef enum ClearOption {
    CLEAR_ALL,
    CLEAR_PREVIEW,
    CLEAR,
} ClearOption;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

static const double CACHE_IMG_WIDTH = 1024;
static const int MAX_IMG_WIDTH = 2048;
static const int MAX_PNG_WIDTH =  1536;
static const int HEIGHT_SHELL = 11;

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

int estrtol(char *);
void *util_realloc(void *, size_t);
char *egetenv(char *);
FILE *efopen(char *, char *);
bool ends_with(const char *, const char *);
void display_clear(ClearOption);
int cursor_getx(void);

void get_img_size(Image *);
void reduce_img_size(Image *, double);

#endif /* STIV_H */

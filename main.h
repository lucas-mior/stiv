#include "stiv.h"

static void parse_args(Options *options, int argc, char *argv[]);
static void display_img(Image *img, Options *options);
static void display_clear(int clear_what);
static void cache_name(Image *img);

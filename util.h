#include <stdio.h>
#include "stiv.h"

uint estrtoul(char *string);
void *emalloc(size_t size);
char *egetenv(char *variable);
FILE *efopen(char *filename, char *mode);
bool ends_with(const char *str, const char *end);

extern const char *program;

#define error(x) { fprintf(stderr, "%s : %s\n", program, x); exit(errno); }

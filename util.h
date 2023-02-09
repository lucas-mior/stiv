#include <stdio.h>
#include "stiv.h"

int estrtol(char *string);
void *emalloc(size_t size);
char *egetenv(char *variable);
FILE *efopen(char *filename, char *mode);
bool ends_with(const char *str, const char *end);

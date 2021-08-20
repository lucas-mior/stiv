int estrtol(char *string);
void *emalloc(size_t size);
char *egetenv(char *variable);
FILE *efopen(char *filename, char *mode);

extern const char *program;

#define error(x) { fprintf(stderr, "%s : %s\n", program, x); exit(errno); }

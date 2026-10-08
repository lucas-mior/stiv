#if !defined(FIFO_WRITE_NONBLOCK_C)
#define FIFO_WRITE_NONBLOCK_C

#define CBASE_IMPLEMENT 1
#include "cbase.h"

int
main(int argc, char **argv) {
    int32 fd;
    int32 fifo_len;
    int32 string_len;
    char *string = NULL;
    char *fifo = NULL;

    for (int32 i = 1; i < argc; i += 1) {
        PARSE_OPTION(argv[i], string)
        PARSE_OPTION(argv[i], fifo)
        error("Invalid argument: %s\n", argv[i]);
        fatal(EXIT_FAILURE);
    }

    if ((string == NULL) || (fifo == NULL)) {
        error2("usage: %s string=<string> fifo=<fifo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    string_len = strlen32(string);
    fifo_len = strlen32(fifo);

    if (DEBUGGING) {
        printf("string=%.*s=\n", string_len, string);
        printf("fifo=%.*s=\n", fifo_len, fifo);
    }

    if ((fd = open(fifo, O_WRONLY |O_NONBLOCK)) < 0) {
        error("Error opening %s: %s.\n", fifo, strerror(errno));
        fatal(EXIT_FAILURE);
    }

    write_all(fd, string, string_len);
    write_all(fd, STRLIT("\n"));
    XCLOSE(&fd, fifo);

    exit(EXIT_SUCCESS);
}

#endif /* FIFO_WRITE_NONBLOCK_C */

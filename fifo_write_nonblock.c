#define CBASE_IMPLEMENT
#include "cbase.h"

int
main(int argc, char **argv) {
    int fd;
    char *string = NULL;
    char *fifo = NULL;

    program = basename(argv[0]);

    for (int i = 1; i < argc; i += 1) {
        PARSE_OPTION(argv[i], string)
        PARSE_OPTION(argv[i], fifo)
        error("%s: Invalid argument: %s\n", program, argv[i]);
        exit(EXIT_FAILURE);
    }

    if (DEBUGGING) {
        printf("string=%s=\n", string);
        printf("fifo=%s=\n", fifo);
    }

    if ((string == NULL) || (fifo == NULL)) {
        error("usage: %s string=<string> fifo=<fifo>\n", program);
        exit(EXIT_FAILURE);
    }

    if ((fd = open(fifo, O_WRONLY | O_NONBLOCK)) < 0) {
        error("Error opening %s: %s.\n", fifo, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (dprintf(fd, "%s\n", string) < 0) {
        error("Error in dprintf: %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(fd) < 0) {
        error("Error closing %s: %s.\n", fifo, strerror(errno));
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

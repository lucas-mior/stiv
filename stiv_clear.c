#if !defined(STIV_CLEAR_C)
#define STIV_CLEAR_C

#define ERROR_NOTIFY 1
#define CBASE_IMPLEMENT 1
#include "cbase.h"

#include "stiv.h"

int
main(int argc, char **argv) {
    struct timespec started;
    enum StivFifoAction action = STIV_FIFO_CLEAR;
    int32 status;

    if (argc == 2) {
        int32 arg_len = strlen32(argv[1]);

        if (STREQUAL(argv[1], arg_len, "pause")) {
            action = STIV_FIFO_PAUSE;
        } else if (STREQUAL(argv[1], arg_len, "resume")) {
            action = STIV_FIFO_RESUME;
        }
    }

    /* Filename and geometry arguments do not determine overlay presence. */
    if (clock_gettime(CLOCK_MONOTONIC, &started) < 0) {
        error("Error reading clock: %s\n", strerror(errno));
        fatal(EXIT_FAILURE);
    }
    status = stiv_fifo_send(action, started, STRLIT(UEBERZUG_CLEAR));
    if (status < 0) {
        error("Error sending ueberzug request: %s\n", strerror(-status));
        fatal(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

#endif /* STIV_CLEAR_C */

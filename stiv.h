#if !defined(STIV_H)
#define STIV_H

#include "cbase.h"

#define MAX_CACHE_WIDTH 1024.0
#define MAX_IMG_WIDTH 2048
#define MAX_PNG_WIDTH 1536
#define HEIGHT_SHELL 11

typedef struct File {
    FILE *file;
    char *name;
    int fd;
    int unused;
} File;

enum StivFifoAction {
    STIV_FIFO_DRAW = 1,
    STIV_FIFO_CLEAR,
    STIV_FIFO_PAUSE,
    STIV_FIFO_RESUME,
};

int32 stiv_fifo_send(enum StivFifoAction action, struct timespec started,
                     char *message, int32 message_len);

#define UEBERZUG_CLEAR "{\"action\": \"remove\", \"identifier\": \"preview\"}\n"

#endif /* STIV_H */

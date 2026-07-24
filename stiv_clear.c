#include "stiv.h"

#define ERROR_NOTIFY 1
#define CBASE_IMPLEMENT
#include "cbase.h"

static int is_image_preview(char *, int32);
static magic_t magic;

int
main(int argc, char **argv) {
    char *UEBERZUG_FIFO;
    int ueberzug_fd;

    char *last_filename = NULL;
    char *next_filename = NULL;
    program = basename(argv[0]);

    // $1 previous file name
    // $2 width $3 height
    // $4 horizontal position
    // $5 vertical position of preview pane
    // $6 current filename to be previewed
    if (argc >= 7) {
        last_filename = realpath(argv[1], NULL);
        next_filename = realpath(argv[6], NULL);
    }

    if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
        error("Error in magic_open(MAGIC_MIME_TYPE): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (magic_load(magic, NULL) != 0) {
        error("Error in magic_load(): %s\n", magic_error(magic));
        exit(EXIT_FAILURE);
    }

    if (last_filename && next_filename) {
        int32 last_filename_len = strlen32(last_filename);
        int32 next_filename_len = strlen32(next_filename);

        if (!is_image_preview(last_filename, last_filename_len)) {
            exit(EXIT_SUCCESS);
        }
        if (is_image_preview(next_filename, next_filename_len)) {
            exit(EXIT_SUCCESS);
        }
    }

    GETENV(UEBERZUG_FIFO);
    if (UEBERZUG_FIFO == NULL) {
        exit(EXIT_FAILURE);
    }
    if ((ueberzug_fd = open(UEBERZUG_FIFO, O_WRONLY | O_NONBLOCK)) < 0) {
        error("Error opening %s in non blocking mode: %s",
              UEBERZUG_FIFO, strerror(errno));
        exit(EXIT_FAILURE);
    }

    dprintf(ueberzug_fd , UEBERZUG_CLEAR);
    XCLOSE(&ueberzug_fd, UEBERZUG_FIFO);
    exit(EXIT_SUCCESS);
}

int
is_image_preview(char *filename, int32 filename_len) {
    const char *mime_type;
    int32 mime_type_len;
    (void)filename_len;

    if ((mime_type = magic_file(magic, filename)) == NULL) {
        return false;
    }
    mime_type_len = strlen32((char *)mime_type);

    if (BEGINS_WITH((char *)mime_type, mime_type_len, "image/")) {
        return true;
    }
    if (BEGINS_WITH((char *)mime_type, mime_type_len, "application/pdf")) {
        return true;
    }
    if (BEGINS_WITH((char *)mime_type, mime_type_len, "application/csv")) {
        return true;
    }
    if (ENDS_WITH(filename, filename_len, ".csv")) {
        return true;
    }
    if (BEGINS_WITH((char *)mime_type, mime_type_len, "audio/")) {
        return true;
    }
    if (BEGINS_WITH((char *)mime_type, mime_type_len, "video/")) {
        return true;
    }

    return false;
}

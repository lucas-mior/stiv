#if !defined(STIV_CLEAR_C)
#define STIV_CLEAR_C

#define ERROR_NOTIFY 1
#define CBASE_IMPLEMENT 1
#include "cbase.h"

#include "stiv.h"
#include <magic.h>

static int32
stiv_preview_check(magic_t magic, char *filename, int32 filename_len,
                   bool *is_preview) {
    char *mime_type;
    int32 mime_type_len;

    ASSERT(is_preview != NULL);
    *is_preview = false;

    if ((mime_type = (char *)magic_file(magic, filename)) == NULL) {
        return -EIO;
    }
    mime_type_len = strlen32(mime_type);

    if (BEGINS_WITH(mime_type, mime_type_len, "image/")
        || BEGINS_WITH(mime_type, mime_type_len, "application/pdf")
        || BEGINS_WITH(mime_type, mime_type_len, "application/csv")
        || ENDS_WITH(filename, filename_len, ".csv")
        || BEGINS_WITH(mime_type, mime_type_len, "audio/")
        || BEGINS_WITH(mime_type, mime_type_len, "video/")) {
        *is_preview = true;
    }
    return 0;
}

int
main(int argc, char **argv) {
    char *ueberzug_fifo;
    int32 ueberzug_fd;

    /*
     * argv[1]: previous filename
     * argv[2]: width
     * argv[3]: height
     * argv[4]: horizontal preview position
     * argv[5]: vertical preview position
     * argv[6]: current filename
     */
    if (argc >= 7) {
        char last_filename[PATH_MAX];
        char next_filename[PATH_MAX];
        char *last_filename_result;
        char *next_filename_result;
        magic_t magic;

        if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
            error("Error in magic_open(MAGIC_MIME_TYPE): %s\n",
                  strerror(errno));
            fatal(EXIT_FAILURE);
        }
        if (magic_load(magic, NULL) != 0) {
            error("Error in magic_load(): %s\n", magic_error(magic));
            magic_close(magic);
            fatal(EXIT_FAILURE);
        }

        last_filename_result = realpath(argv[1], last_filename);
        next_filename_result = realpath(argv[6], next_filename);
        if ((last_filename_result != NULL) && (next_filename_result != NULL)) {
            int32 last_filename_len = strlen32(last_filename);
            int32 next_filename_len = strlen32(next_filename);
            bool last_is_preview;
            bool next_is_preview;

            if ((stiv_preview_check(magic,
                                    last_filename, last_filename_len,
                                    &last_is_preview) < 0)
                || (stiv_preview_check(magic,
                                       next_filename, next_filename_len,
                                       &next_is_preview) < 0)) {
                error("Error detecting preview type: %s.\n",
                      magic_error(magic));
                magic_close(magic);
                fatal(EXIT_FAILURE);
            }
            if (!last_is_preview) {
                magic_close(magic);
                exit(EXIT_SUCCESS);
            }
            if (next_is_preview) {
                magic_close(magic);
                exit(EXIT_SUCCESS);
            }
        }
        magic_close(magic);
    }

    ueberzug_fifo = getenv("UEBERZUG_FIFO");
    if (ueberzug_fifo == NULL) {
        exit(EXIT_FAILURE);
    }
    if ((ueberzug_fd = open(ueberzug_fifo, O_WRONLY |O_NONBLOCK)) < 0) {
        error("Error opening %s in non blocking mode: %s",
              ueberzug_fifo, strerror(errno));
        fatal(EXIT_FAILURE);
    }

    write_all(ueberzug_fd, STRLIT(UEBERZUG_CLEAR));
    XCLOSE(&ueberzug_fd, ueberzug_fifo);
    exit(EXIT_SUCCESS);
}

#endif /* STIV_CLEAR_C */

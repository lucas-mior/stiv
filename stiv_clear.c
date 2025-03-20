/* This file is part of stiv.
 * Copyright (C) 2024 Lucas Mior

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stiv.h"
#include "util.c"

static int is_image_preview(char *);
static inline int literal_match(const char *, char *);
static magic_t magic;

char *program;

int
main(int argc, char **argv) {
    File UEBERZUG_FIFO = {
        .file = NULL,
        .fd = -1,
        .name = NULL
    };

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
        if (!is_image_preview(last_filename))
            exit(EXIT_SUCCESS);
        if (is_image_preview(next_filename))
            exit(EXIT_SUCCESS);
    }

    if ((UEBERZUG_FIFO.name = getenv("UEBERZUG_FIFO")) == NULL) {
        error("UEBERZUG_FIFO environment variable is not set.\n");
        exit(EXIT_FAILURE);
    }
    if ((UEBERZUG_FIFO.fd = open(UEBERZUG_FIFO.name,
                                 O_WRONLY | O_NONBLOCK)) < 0) {
        error("Error opening %s: %s", UEBERZUG_FIFO.name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    dprintf(UEBERZUG_FIFO.fd, UEBERZUG_CLEAR);
    util_close(&UEBERZUG_FIFO);
    exit(EXIT_SUCCESS);
}

int
is_image_preview(char *filename) {
    const char *mime_type;

    if ((mime_type = magic_file(magic, filename)) == NULL)
        return false;

    if (!literal_match(mime_type, "image/"))
        return true;
    if (!literal_match(mime_type, "application/pdf"))
        return true;
    if (!literal_match(mime_type, "audio/"))
        return true;
    if (!literal_match(mime_type, "video/"))
        return true;

    return false;
}

int literal_match(const char *mime, char *literal) {
    size_t n = strlen(literal);
    return strncmp(literal, mime, n);
}

/* This file is part of stiv.
 * Copyright (C) 2022 Lucas Mior

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

static int is_image(char *);

int
is_image(char *filename) {
    do {
        magic_t magic;
        const char *mime_type;
        if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
            break;
        }
        if (magic_load(magic, NULL) != 0) {
            magic_close(magic);
            break;
        }

        if ((mime_type = magic_file(magic, filename)) == NULL) {
            magic_close(magic);
            break;
        }
        if (!strncmp(mime_type, "image/", 6)) {
            magic_close(magic);
            return true;
        } else if (!strncmp(mime_type, "application/pdf", 6)) {
            magic_close(magic);
            return true;
        }
        magic_close(magic);
    } while (0);

    return false;
}

int
main(int argc, char **argv) {
    File UEBERZUG_FIFO = {
        .file = NULL,
        .fd = -1,
        .name = NULL
    };
    char *last_filename = NULL;
    char *next_filename = NULL;

    if (argc >= 7) {
        last_filename = argv[1];
        next_filename = argv[6];
    }

    if (last_filename && next_filename) {
        if (!is_image(last_filename)) {
            fprintf(stderr, "Last file was not image\n");
            exit(EXIT_SUCCESS);
        }
        if (is_image(next_filename)) {
            fprintf(stderr, "Next file is image\n");
            exit(EXIT_SUCCESS);
        }
    }


    if ((UEBERZUG_FIFO.name = getenv("UEBERZUG_FIFO")) == NULL) {
        fprintf(stderr, "UEBERZUG_FIFO environment variable is not set.\n");
        return 0;
    }
    if ((UEBERZUG_FIFO.file = fopen(UEBERZUG_FIFO.name, "w")) == NULL) {
        fprintf(stderr, "Error opening %s: %s", UEBERZUG_FIFO.name, strerror(errno));
        return 0;
    }

    fprintf(UEBERZUG_FIFO.file, "{\"action\": \"remove\", \"identifier\": \"preview\"}\n");
    fclose(UEBERZUG_FIFO.file);
    return 0;
}

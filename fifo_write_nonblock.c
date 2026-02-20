/*
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

#include "util.c"

static bool
parse_option(char **parsed, char *arg, char *name, int32 length) {
    char name_equal[256];
    SNPRINTF(name_equal, "%s=", name);

    if (strncmp64(arg, name_equal, length + 1) == 0) {
        *parsed = arg + length + 1;
        return true;
    }
    return false;
}

#define PARSE_OPTION(arg, name) \
    if (parse_option(&name, arg, #name, strlen(#name))) { \
        continue; \
    }

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
        error("Error opening %s: %s\n", fifo, strerror(errno));
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

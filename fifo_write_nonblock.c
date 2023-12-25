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
char *program;

int
main(int argc, char **argv) {
    int fd;
    char *string;
    char *fifo;

    program = basename(argv[0]);
    string = argv[1];
    fifo = argv[2];

    if (argc < 3) {
        error("usage: <string> <fifo>\n", program);
        exit(EXIT_FAILURE);
    }

    if ((fd = open(fifo, O_WRONLY | O_NONBLOCK)) < 0) {
        error("Error opening %s: %s", fifo, strerror(errno));
        exit(EXIT_FAILURE);
    }

    dprintf(fd, "%s\n", string);

    close(fd);
    exit(EXIT_SUCCESS);
}

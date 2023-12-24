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
    char *fifo = argv[argc - 1];
    program = argv[0];

    if (argc < 3) {
        error("Minimum 3 arguments.\n");
        exit(EXIT_FAILURE);
    }

    if ((fd = open(fifo, O_WRONLY | O_NONBLOCK)) < 0) {
        error("Error opening %s: %s", fifo, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < (argc - 1); i += 1)
        dprintf(fd, "%s", argv[1]);
    dprintf(fd, "\n");

    close(fd);
    exit(EXIT_SUCCESS);
}

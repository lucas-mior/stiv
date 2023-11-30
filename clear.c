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

void clear_display(int clear_option) {
    File UEBERZUG_FIFO = {
        .file = NULL,
        .fd = -1,
        .name = NULL
    };
    File ueberzug_drawed = {
        .file = NULL,
        .fd = -1,
        .name = NULL
    };
    const char *suffix = ".drawed";
    usize length;
    char line[PATH_MAX];

    if ((UEBERZUG_FIFO.name = getenv("UEBERZUG_FIFO")) == NULL) {
        fprintf(stderr, "UEBERZUG_FIFO environment variable is not set.\n");
        return;
    }
    if ((UEBERZUG_FIFO.file = fopen(UEBERZUG_FIFO.name, "w")) == NULL) {
        fprintf(stderr, "Error opening %s: %s", UEBERZUG_FIFO.name, strerror(errno));
        return;
    }

    switch (clear_option) {
    case CLEAR_DEFAULT:
        fprintf(UEBERZUG_FIFO.file, "{\"action\": \"remove\"}\n");
        break;
    case CLEAR_ALL:
        length = strlen(UEBERZUG_FIFO.name) + strlen(suffix);
        ueberzug_drawed.name = util_realloc(NULL, length + 1); 
        sprintf(ueberzug_drawed.name, "%s.drawed", UEBERZUG_FIFO.name);
        if ((ueberzug_drawed.file = fopen(ueberzug_drawed.name, "r"))) {
            while (fgets(line, sizeof (line), ueberzug_drawed.file)) {
                line[strcspn(line, "\n")] = 0;
                fprintf(UEBERZUG_FIFO.file,
                        "{\"action\": \"remove\", \"identifier\": \"%s\"}\n", line);
            }
            util_close(&ueberzug_drawed);
        }
        // clean the file
        if ((ueberzug_drawed.file = fopen(ueberzug_drawed.name, "w")))
            util_close(&ueberzug_drawed);
    __attribute__((fallthrough));
    case CLEAR_PREVIEW:
    default:
        fprintf(UEBERZUG_FIFO.file,
                "{\"action\": \"remove\", \"identifier\": \"preview\"}\n");
        break;
    }
    fclose(UEBERZUG_FIFO.file);
    return;
}

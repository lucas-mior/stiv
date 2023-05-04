/* This file is part of stiv. */
/* Copyright (C) 2022 Lucas Mior */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include "stiv.h"
#include <stdlib.h>
#include <limits.h>

void clear_display(int clear_option) {
    File ueberzug_fifo;
    File ueberzug_drawed;
    char line[PATH_MAX];

    if ((ueberzug_fifo.name = getenv("UEBERZUG_FIFO")) == NULL) {
        fprintf(stderr, "UEBERZUG_FIFO environment variable is not set.\n");
        return;
    }
    if ((ueberzug_fifo.file = fopen(ueberzug_fifo.name, "w")) == NULL) {
        fprintf(stderr, "Error opening %s: %s", ueberzug_fifo.name, errno);
        return;
    }

    switch (clear_option) {
    case CLEAR_DEFAULT:
        fprintf(ueberzug_fifo.file, S({"action": "remove"}\n));
        break;
    case CLEAR_ALL:
        const char *suffix = ".drawed";
        size_t length = strlen(ueberzug_fifo.name) + strlen(suffix);
        ueberzug_drawed.name = util_realloc(NULL, length + 1); 
        sprintf(ueberzug_drawed.name, "%s.drawed", ueberzug_fifo.name);
        if ((ueberzug_drawed.file = fopen(ueberzug_drawed.name, "r"))) {
            while (fgets(line, sizeof(line), ueberzug_drawed.file)) {
                line[strcspn(line, "\n")] = 0;
                fprintf(ueberzug_fifo.file, S({"action": "remove", "identifier": "%s"}\n), line);
            }
            if ((ueberzug_drawed.file = freopen(ueberzug_drawed.name, "w", ueberzug_drawed.file)))
                fclose(ueberzug_drawed.file);
        }
    case CLEAR_PREVIEW:
    default:
        fprintf(ueberzug_fifo.file, S({"action": "remove", "identifier": "preview"}\n));
        break;
    }
    fclose(ueberzug_fifo.file);

    return;
}

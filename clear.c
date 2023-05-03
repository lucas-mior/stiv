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
    char *UEBERZUG_FIFO = NULL;
    char drawed_file[PATH_MAX];
    FILE *ueberzug = NULL;
    FILE *DRAWED = NULL;
    char line[PATH_MAX];

    if ((UEBERZUG_FIFO = getenv("UEBERZUG_FIFO")) == NULL) {
        fprintf(stderr, "UEBERZUG_FIFO environment variable is not set.\n");
        return;
    }
    if ((ueberzug = fopen(UEBERZUG_FIFO, "w")) == NULL) {
        fprintf(stderr, "Error opening %s: %s", UEBERZUG_FIFO, errno);
        return;
    }

    switch (clear_option) {
    case CLEAR_DEFAULT:
        fprintf(ueberzug, S({"action": "remove"}\n));
        break;
    case CLEAR_ALL:
        snprintf(drawed_file, sizeof(drawed_file), "%s.drawed", UEBERZUG_FIFO);
        if ((DRAWED = fopen(drawed_file, "r"))) {
            while (fgets(line, sizeof(line), DRAWED)) {
                line[strcspn(line, "\n")] = 0;
                fprintf(ueberzug, S({"action": "remove", "identifier": "%s"}\n), line);
            }
            if ((DRAWED = freopen(drawed_file, "w", DRAWED)))
                fclose(DRAWED);
        }
    case CLEAR_PREVIEW:
    default:
        fprintf(ueberzug, S({"action": "remove", "identifier": "preview"}\n));
        break;
    }
    fclose(ueberzug);

    return;
}

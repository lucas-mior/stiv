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

void display_clear(ClearOption clear_what) {
    char *ueberzug = NULL;
    char drawed_file[128];
    FILE *UZUG = NULL;
    FILE *DRAWED = NULL;
    char line[128];

    ueberzug = egetenv("UZUG");

    if (!(UZUG = fopen(ueberzug, "w"))) {
        fprintf(stderr, "fopen() failed!\n");
        return;
    }
    switch(clear_what) {
    case CLEAR:
        fprintf(UZUG, S({"action": "remove"}\n));
        break;
    case CLEAR_ALL:
        snprintf(drawed_file, sizeof(drawed_file), "%s.drawed", ueberzug);
        if ((DRAWED = fopen(drawed_file, "r"))) {
            while (fgets(line, (int) sizeof(line), DRAWED)) {
                line[strcspn(line, "\n")] = 0;
                fprintf(UZUG, S({"action": "remove", "identifier": "%s"}\n), line);
            }
            if ((DRAWED = freopen(drawed_file, "w", DRAWED)))
                fclose(DRAWED);
        }
    // fall through
    default:
        fprintf(UZUG, S({"action": "remove", "identifier": "preview"}\n));
        break;
    }
    fclose(UZUG);

    return;
}

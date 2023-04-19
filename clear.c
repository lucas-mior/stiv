#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stiv.h"
#include "clear.h"
#include "util.h"

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

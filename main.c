#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <magic.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "stiv.h"
#include "util.h"
#include "cursor.h"
#include "image.h"
#include "main.h"

int exit_code = 1;

int main(int argc, char *argv[]) {
    program = argv[0];
    Options options = {
        .w = 100, .h = HEIGHT_SHELL, .H = -1,
        .x = 0, .y = 1,
        .preview = true,
        .clear = false,
        .print_dim = true,
    };

    Image img = {
        .filename = NULL,
        .path = NULL,
        .w = 0,
        .h = 0,
    };

    img.filename = argv[1];

    parse_args(&options, argc, argv);

    if (options.print_dim) {
        get_img_size(&img);
        printf("\033[01;31m%u\033[0;mx\033[01;31m%u\033[0;m\n", img.w, img.h);
    }

    if (img.w > MAX_IMG_WIDTH) {
        cache_name(&img);
        reduce_img_size(&img, CACHE_IMG_WIDTH);
    } else if (img.w > MAX_PNG_WIDTH) {
        magic_t my_magic;
        my_magic = magic_open(MAGIC_MIME_TYPE);
        magic_load(my_magic, NULL);
        if(!strcmp(magic_file(my_magic, img.filename), "image/png")) {
            cache_name(&img);
            reduce_img_size(&img, CACHE_IMG_WIDTH);
        }
        magic_close(my_magic);
    } else if (ends_with(img.filename, "ff")) {
        cache_name(&img);
        reduce_img_size(&img, img.w);
    }

    display_img(&img, &options);

    return exit_code; // it should always return error so that programs will call it again to redraw
}

void usage() {
    printf("usage: %s IMAGE W H [X Y]\n", program);
    printf("       %s -h | --help\n", program);
    printf("       %s -c | --clear 0 | 1\n", program);
    printf("Be sure to have ueberzug running in the terminal and UZUG env variable set\n");
    exit(EXIT_FAILURE);
}

void parse_args(Options *options, int argc, char *argv[]) {
    char *lines = NULL;
    char *columns = NULL;
    uint l = 0;
    uint c = 0;

    if (argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
        usage();

    if (!strcmp(argv[1], "-c") || !strcmp(argv[1], "--clear")) {
        if (argc == 3)
            display_clear(CLEAR_PREVIEW);
        else
            display_clear(CLEAR_ALL);
        exit(EXIT_FAILURE);
    }

    if (argc >= 6) {
        if (argc >= 7) {
            options->print_dim = false;
        }
        // chamado por `lf > pistol > stiv`
        options->w = estrtoul(argv[2]);
        options->h = estrtoul(argv[3]) - 1;
        options->x = estrtoul(argv[4]);
        options->y = estrtoul(argv[5]) + 1;

        options->w -= 2;
        options->x += 2;
    } else if ((columns = getenv("FZF_PREVIEW_COLUMNS"))
            && (lines = getenv("FZF_PREVIEW_LINES"))) {
        // chamado por `fzf > pistol > stiv`
        options->w = estrtoul(columns);
        options->h = estrtoul(lines);

        options->x = options->w + (options->w % 2);
        options->y = 1;
    } else if ((columns = getenv("COLUMNS"))
            && (lines = getenv("LINES"))) {
        // chamado por `skim > pistol > stiv`
        options->w = estrtoul(columns);
        options->h = estrtoul(lines);

        options->x = options->w + 1 + ((options->w + 1) % 2) + 1;
        options->y = 1;
        exit_code = 0; //skim won't print anything if we exit with an error
    } else if (argc == 4) {
        // chamado por `zsh > stiv`
        columns = argv[2];
        lines = argv[3];
        c = estrtoul(columns);
        l = estrtoul(lines);

        options->w = c;
        options->h = HEIGHT_SHELL;
        options->x = 0;
        options->y = getx();

        options->preview = false;

        if (HEIGHT_SHELL > (l - options->y)) {
            options->y = 1;
            options->clear = true;
        }

        return;
    } else {
        usage();
    }
    return;
}

void display_img(Image *img, Options *options) {
    char *aux = NULL;
    char instance[20] = "preview";

    char drawed_file[200];
    char *ueberzug = NULL;
    FILE *UZUG, *DRAWED;

    ueberzug = egetenv("UZUG");
    UZUG = efopen(ueberzug, "w");

    if (options->clear) {
        printf("\033[2J\033[H"); // clear terminal and jump to first line
        printf("\033[01;31m%u\033[0;mx\033[01;31m%u\033[0;m\n", img->w, img->h);
        display_clear(CLEAR_ALL);
    }

    if (!options->preview) {
        if (!(aux = basename(img->filename))) {
            fprintf(stderr, "basename(%s) : %s\n", img->filename, strerror(errno));
        } else {
            srand((uint) time(NULL));
            snprintf(instance, sizeof(instance), "%d%s", rand(), aux);
        }
    }
    if(img->path == NULL) {
        if (!(img->path = realpath(img->filename, NULL)))
            error(strerror(errno));
    }

    fprintf(UZUG, S({"action": "add", "identifier": "%s", "scaler": "fit_contain",
                     "x": %u, "y": %u, "width": %u, "height": %u, "path": "%s"}\n), instance,
                     options->x, options->y, options->w, options->h, img->path);

    if (!options->preview) {
        printf("\n\n\n\n\n\n\n\n\n\n\n");

        snprintf(drawed_file, sizeof(drawed_file), "%s.drawed", ueberzug);
        if (!(DRAWED = fopen(drawed_file, "a"))) {
            free(img->path);
            img->path = NULL;
            return;
        }
        fprintf(DRAWED, "%s\n", instance);
    }

    free(img->path);
    img->path = NULL;
    return;
}

void display_clear(int clear_what) {
    char *ueberzug = NULL;
    char drawed_file[200];
    FILE *UZUG = NULL;
    FILE *DRAWED = NULL;
    char line[200];

    ueberzug = egetenv("UZUG");

    if ((UZUG = fopen(ueberzug, "w"))) {
        switch(clear_what) {
            case CLEAR_ALL:
                snprintf(drawed_file, 200, "%s.drawed", ueberzug);
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
    }

    return;
}

void cache_name(Image *img) {
    struct stat file;

    if (stat(img->filename, &file) == -1)
        error(strerror(errno));

    snprintf(img->cache, sizeof(img->cache), "%li_%ld_%ld", file.st_size, file.st_mtim.tv_sec, file.st_mtim.tv_nsec);
    return;
}

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

extern int exit_code;
int exit_code = 1;

static void main_usage(FILE *);
static void main_parse_args(Options *, int, char *[]);
static void main_cache_name(Image *);
static void main_display_img(Image *, Options *);

int main(int argc, char *argv[]) {
    Options options = {
        .w = 100, .h = HEIGHT_SHELL, .H = -1,
        .x = 0, .y = 1,
        .preview = true,
        .clear = false,
        .print_dim = true,
    };

    Image image = {
        .basename = NULL,
        .fullpath = NULL,
        .width = 0,
        .height = 0,
    };

    image.basename = argv[1];

    main_parse_args(&options, argc, argv);

    image_get_size(&image);
    if (options.print_dim)
        printf("\033[01;31m%u\033[0;mx\033[01;31m%u\033[0;m\n",
               image.width, image.height);

    if (image.width > MAX_IMG_WIDTH) {
        main_cache_name(&image);
        image_reduce_size(&image, CACHE_IMG_WIDTH);
    } else if (image.width > MAX_PNG_WIDTH) {
        magic_t my_magic;
        my_magic = magic_open(MAGIC_MIME_TYPE);
        magic_load(my_magic, NULL);
        if (!strcmp(magic_file(my_magic, image.basename), "image/png")) {
            main_cache_name(&image);
            image_reduce_size(&image, CACHE_IMG_WIDTH);
        }
        magic_close(my_magic);
    } else if (ends_with(image.basename, "ff")) {
        main_cache_name(&image);
        image_reduce_size(&image, image.width);
    }

    main_display_img(&image, &options);
    // it should return error so that programs will call it again to redraw
    return exit_code;
}

void main_usage(FILE *stream) {
    fprintf(stream, "usage: stiv IMAGE W H [X Y]\n");
    fprintf(stream, "       stiv -h | --help\n");
    fprintf(stream, "       stiv -c | --clear 0 | 1\n");
    fprintf(stream, "Be sure to have ueberzug running in the terminal "
                    "and UEBERZUG_FIFO env variable set\n");
    exit((int) (stream != stdout));
}

void main_parse_args(Options *options, int argc, char *argv[]) {
    Number lines;
    Number columns;

    if (argc <= 2) {
        clear_display(CLEAR_ALL);
        exit(EXIT_FAILURE);
    } else if (argc == 3) {
        clear_display(CLEAR_PREVIEW);
        exit(EXIT_FAILURE);
    }

    if (argc >= 6) {
        if (argc >= 7) {
            options->print_dim = false;
        }
        // chamado por `lf > pistol > stiv`
        options->w = util_string_int32(argv[2]);
        options->h = util_string_int32(argv[3]) - 1;
        options->x = util_string_int32(argv[4]);
        options->y = util_string_int32(argv[5]) + 1;

        options->w -= 2;
        options->x += 2;
    } else if ((columns.string = getenv("FZF_PREVIEW_COLUMNS"))
            && (lines.string = getenv("FZF_PREVIEW_LINES"))) {
        // chamado por `fzf > pistol > stiv`
        options->w = util_string_int32(columns.string);
        options->h = util_string_int32(lines.string);

        options->x = options->w + (options->w % 2);
        options->y = 1;
    } else if ((columns.string = getenv("COLUMNS"))
            && (lines.string = getenv("LINES"))) {
        // chamado por `skim > pistol > stiv`
        options->w = util_string_int32(columns.string);
        options->h = util_string_int32(lines.string);

        options->x = options->w + 1 + ((options->w + 1) % 2) + 1;
        options->y = 1;
        exit_code = 0; //skim won't print anything if we exit with an error
    } else if (argc == 4) {
        // chamado por `zsh > stiv`
        columns.string = argv[2];
        lines.string = argv[3];
        columns.number = util_string_int32(columns.string);
        lines.number = util_string_int32(lines.string);

        options->w = columns.number;
        options->h = HEIGHT_SHELL;
        options->x = 0;
        options->y = cursor_getx();

        options->preview = false;

        if (HEIGHT_SHELL > (lines.number - options->y)) {
            options->y = 1;
            options->clear = true;
        }

        return;
    } else {
        main_usage(stderr);
    }
    return;
}

void main_display_img(Image *image, Options *options) {
    char instance[20] = "preview";

    File ueberzug_fifo = {.file = NULL, .fd = -1, .name = NULL};
    File ueberzug_drawed = {.file = NULL, .fd = -1, .name = NULL};

    if ((ueberzug_fifo.name = getenv("UEBERZUG_FIFO")) == NULL) {
        fprintf(stderr, "UEBERZUG_FIFO environment variable is not set.\n");
        return;
    }
    if ((ueberzug_fifo.file = fopen(ueberzug_fifo.name, "w")) == NULL) {
        fprintf(stderr, "Error opening %s: %s", ueberzug_fifo.name, errno);
        return;
    }

    if (options->clear) {
        printf("\033[2J\033[H"); // clear terminal and jump to first line
        printf("\033[01;31m%u\033[0;mx\033[01;31m%u\033[0;m\n",
               image->width, image->height);
        clear_display(CLEAR_ALL);
    }

    if (!options->preview) {
        char *aux;
        if (!(aux = basename(image->basename))) {
            fprintf(stderr, "Error getting basename of %s: %s\n",
                            image->basename, strerror(errno));
        } else {
            srand((uint) time(NULL));
            snprintf(instance, sizeof(instance), "%d%s", rand(), aux);
        }
    }
    if (!(image->fullpath = realpath(image->basename, NULL))) {
        fprintf(stderr, "Error getting realpath of %s: %s",
                        image->fullpath, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(ueberzug_fifo.file, 
            S({"action": "add", "identifier": "%s",), instance);
    fprintf(ueberzug_fifo.file, S("scaler": "fit_contain",));
    fprintf(ueberzug_fifo.file,
            S("x": %u, "y": %u, "width": %u, "height": %u, "path": "%s"}\n), 
            options->x, options->y, options->w, options->h, image->fullpath);

    if (!options->preview) {
        printf("\n\n\n\n\n\n\n\n\n\n\n");
        const char *suffix = ".drawed";
        size_t length = strlen(ueberzug_fifo.name) + strlen(suffix);
        ueberzug_drawed.name = util_realloc(NULL, length + 1); 
        sprintf(ueberzug_drawed.name, "%s.drawed", ueberzug_fifo.name);
        if (!(ueberzug_drawed.file = fopen(ueberzug_drawed.name, "a"))) {
            free(image->fullpath);
            image->fullpath = NULL;
            return;
        }
        fprintf(ueberzug_drawed.file, "%s\n", instance);
    }

    util_close(&ueberzug_fifo);
    util_close(&ueberzug_drawed);
    free(image->fullpath);
    image->fullpath = NULL;
    return;
}

void main_cache_name(Image *image) {
    struct stat file;
    char buffer[PATH_MAX];

    if (stat(image->basename, &file) < 0) {
        fprintf(stderr, "Error calling stat on %s: %s.",
                        image->basename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    snprintf(buffer, sizeof(buffer), "%li_%ld_%ld",
             file.st_size, file.st_mtim.tv_sec, file.st_mtim.tv_nsec);
    image->cachename = strdup(buffer);
    return;
}

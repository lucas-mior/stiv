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
#include <stdlib.h>
#include <Imlib2.h>

typedef struct Options {
    int32 w, h, H;
    int32 x, y;
    bool preview;
    bool clear;
    bool print_dim;
    bool unused;
} Options;

typedef struct Image {
    char *basename;
    char *fullpath;
    char *cachename;
    int width, height;
} Image;

extern int exit_code;
int exit_code = EXIT_FAILURE;

static void main_usage(FILE *) __attribute__((noreturn));
static void main_parse_args(Options *, int, char *[]);
static void main_cache_name(Image *);
static void main_display_img(Image *, Options *);

static void image_get_size(Image *image) {
    Imlib_Image imlib_image;

    imlib_image = imlib_load_image(image->basename);
    imlib_context_set_image(imlib_image);

    image->width = imlib_image_get_width();
    image->height = imlib_image_get_height();

    imlib_free_image();
    /* imlib_free_image_and_decache(); */
    return;
}

static void image_reduce_size(Image *image, double new_width) {
    FILE *cache_img;
    char *XDG_CACHE_HOME = NULL;

    const char *previewer = "previewer/stiv";
    const char *jpg = "jpg";
    char buffer[PATH_MAX];
	int n;

    double new_height;
    if (new_width > MAX_IMG_WIDTH)
        new_width = CACHE_IMG_WIDTH;

    if ((XDG_CACHE_HOME = getenv("XDG_CACHE_HOME")) == NULL) {
        fprintf(stderr, "XDG_CACHE_HOME is not set. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    n = snprintf(buffer, sizeof (buffer), 
                "%s/%s/%s.%s", XDG_CACHE_HOME, previewer, image->cachename, jpg);
	if (n < 0) {
		fprintf(stderr, "Error printing cache name.\n");
		exit(EXIT_FAILURE);
	}
    image->fullpath = util_strdup(buffer);

    if ((cache_img = fopen(image->fullpath, "r"))) {
        fclose(cache_img);
        return;
    }
    if (errno == ENOENT) {
        Imlib_Image imlib_image;
        Imlib_Load_Error err;

        double z = image->width / new_width;
        new_height = round(((double) image->height / z));

        imlib_image = imlib_load_image(image->basename);
        imlib_context_set_image(imlib_image);
        imlib_context_set_anti_alias(1);
        imlib_image = imlib_create_cropped_scaled_image(
                      0, 0, image->width, image->height, 
                      (int) new_width, (int) new_height
                      );
        if (imlib_image == NULL)
            goto dontcache;

        imlib_context_set_image(imlib_image);

        if (imlib_image_has_alpha()) {
            imlib_image_set_format("png");
        } else {
            imlib_image_set_format("jpg");
            imlib_image_attach_data_value("quality", NULL, 90, NULL);
        }
        imlib_save_image_with_error_return(image->fullpath, &err);
        if (err)
            goto dontcache;

        imlib_free_image_and_decache();
        return;
    } else {
        fprintf(stderr, "Error opening %s: %s\n",
                        image->fullpath, strerror(errno));
    }
    dontcache:
    free(image->fullpath);
    image->fullpath = NULL;
    return;
}
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
    exit(exit_code);
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
        // chamado por `lf > pistol > stiv`
        options->w = util_string_int32(argv[2]);
        options->h = util_string_int32(argv[3]) - 1;
        options->x = util_string_int32(argv[4]);
        options->y = util_string_int32(argv[5]) + 1;
        options->y += 1; // tmux bugs lf's Y by 1

        options->w -= 2;
        options->x += 2;
        if (argc >= 7) {
            options->print_dim = false;
            options->y -= 1;
        }
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
        exit_code = EXIT_SUCCESS; // skim won't print anything if we exit with an error
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

    File UEBERZUG_FIFO = {.file = NULL, .fd = -1, .name = NULL};
    File UEBERZUG_DRAWED = {.file = NULL, .fd = -1, .name = NULL};

    if ((UEBERZUG_FIFO.name = getenv("UEBERZUG_FIFO")) == NULL) {
        fprintf(stderr, "UEBERZUG_FIFO environment variable is not set.\n");
        return;
    }
    if ((UEBERZUG_FIFO.file = fopen(UEBERZUG_FIFO.name, "w")) == NULL) {
        fprintf(stderr, "Error opening %s: %s", UEBERZUG_FIFO.name, strerror(errno));
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
            int n;
            srand((uint) time(NULL));
            n = snprintf(instance, sizeof (instance), "%d%s", rand(), aux);
			if (n < 0) {
				fprintf(stderr, "Error printing instance.\n");
				exit(EXIT_FAILURE);
			}
        }
    }
    if (!(image->fullpath = realpath(image->basename, NULL))) {
        fprintf(stderr, "Error getting realpath of %s: %s",
                        image->fullpath, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(UEBERZUG_FIFO.file, 
            "{\"action\": \"add\", \"identifier\": \"%s\",", instance);
    fprintf(UEBERZUG_FIFO.file, "\"scaler\": \"fit_contain\",");
    fprintf(UEBERZUG_FIFO.file,
            "\"x\": %u, \"y\": %u, \"width\": %u, \"height\": %u, \"path\": \"%s\"}\n", 
            options->x, options->y, options->w, options->h, image->fullpath);

    if (!options->preview) {
        usize length;
        const char *suffix = ".drawed";
        printf("\n\n\n\n\n\n\n\n\n\n\n");
        length = strlen(UEBERZUG_FIFO.name) + strlen(suffix);
        UEBERZUG_DRAWED.name = util_realloc(NULL, length + 1); 
        sprintf(UEBERZUG_DRAWED.name, "%s.drawed", UEBERZUG_FIFO.name);
        if (!(UEBERZUG_DRAWED.file = fopen(UEBERZUG_DRAWED.name, "a"))) {
            free(image->fullpath);
            image->fullpath = NULL;
            return;
        }
        fprintf(UEBERZUG_DRAWED.file, "%s\n", instance);
    }

    util_close(&UEBERZUG_FIFO);
    util_close(&UEBERZUG_DRAWED);
    free(image->fullpath);
    image->fullpath = NULL;
    return;
}

void main_cache_name(Image *image) {
    struct stat file;
    char buffer[PATH_MAX];
	int n;

    if (stat(image->basename, &file) < 0) {
        fprintf(stderr, "Error calling stat on %s: %s.",
                        image->basename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    n = snprintf(buffer, sizeof (buffer),
                 "%li_%ld_%ld",
                 file.st_size, file.st_mtim.tv_sec, file.st_mtim.tv_nsec);
	if (n < 0) {
		fprintf(stderr, "Error printing cache name.\n");
		exit(EXIT_FAILURE);
	}
    image->cachename = util_strdup(buffer);
    return;
}

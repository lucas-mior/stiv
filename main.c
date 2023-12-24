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
#include <sys/param.h>
#include <stdlib.h>
#include <Imlib2.h>
#include <libexif/exif-data.h>

typedef struct Terminal {
    int width;
    int height;
    int x;
    int y;
} Terminal;

static Terminal terminal = {
    .width = 100,
    .height = HEIGHT_SHELL,
    .x = 0,
    .y = 1,
};

static bool print_dimensions = true;

typedef struct Image {
    char *basename;
    char *fullpath;
    char *cachename;
    int width, height;
} Image;

static Image image = {
    .basename = NULL,
    .fullpath = NULL,
    .width = 0,
    .height = 0,
};

static int exit_code = EXIT_FAILURE;

static void usage(FILE *) __attribute__((noreturn));
static void get_cache_name(void);
static void cache_image(double);

int main(int argc, char *argv[]) {
    Number lines;
    Number columns;
    int needs_rotation = 1;

    image.basename = argv[1];

    if (argc >= 6) {
        // chamado por `lf > piscou > stiv`
        terminal.width = util_string_int32(argv[2]);
        terminal.height = util_string_int32(argv[3]) - 1;
        terminal.x = util_string_int32(argv[4]);
        terminal.y = util_string_int32(argv[5]) + 1;
        terminal.y += 1; // tmux bugs lf's Y by 1

        terminal.width -= 2;
        terminal.x += 2;
        if (argc >= 7) {
            print_dimensions = false;
            terminal.y -= 1;
        }
    } else if ((columns.string = getenv("FZF_PREVIEW_COLUMNS"))
            && (lines.string = getenv("FZF_PREVIEW_LINES"))) {
        // chamado por `fzf > piscou > stiv`
        terminal.width = util_string_int32(columns.string);
        terminal.height = util_string_int32(lines.string);

        terminal.x = terminal.width + (terminal.width % 2);
        terminal.y = 1;
    } else if ((columns.string = getenv("COLUMNS"))
            && (lines.string = getenv("LINES"))) {
        // chamado por `skim > piscou > stiv`
        terminal.width = util_string_int32(columns.string);
        terminal.height = util_string_int32(lines.string);

        terminal.x = terminal.width + 1 + ((terminal.width + 1) % 2) + 1;
        terminal.y = 1;

        // skim won't print anything if we exit with an error
        exit_code = EXIT_SUCCESS;
    } else if (argc == 4) {
        // chamado por `zsh > stiv`
        columns.string = argv[2];
        lines.string = argv[3];
        columns.number = util_string_int32(columns.string);
        lines.number = util_string_int32(lines.string);

        terminal.width = columns.number;
        terminal.height = HEIGHT_SHELL;
        terminal.x = 0;
        terminal.y = 1;
    } else {
        usage(stderr);
    }

    do {
        Imlib_Image imlib_image;

        imlib_image = imlib_load_image(image.basename);
        imlib_context_set_image(imlib_image);
        ExifData *ed;
        ExifEntry *entry;
        int byte_order, orientation = 0;

        if ((ed = exif_data_new_from_file(image.basename)) == NULL) {
            needs_rotation = 0;
            break;
        }
        byte_order = exif_data_get_byte_order(ed);
        entry = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
        if (entry)
            orientation = exif_get_short(entry->data, byte_order);

        exif_data_unref(ed);

        switch (orientation) {
        case 5:
            imlib_image_orientate(1);
            /* fall through */
        case 2:
            imlib_image_flip_vertical();
            break;
        case 3:
            imlib_image_orientate(2);
            break;
        case 7:
            imlib_image_orientate(1);
            /* fall through */
        case 4:
            imlib_image_flip_horizontal();
            break;
        case 6:
            imlib_image_orientate(1);
            break;
        case 8:
            imlib_image_orientate(3);
            break;
        default:
            needs_rotation = 0;
            break;
        }

    } while (0);

    image.width = imlib_image_get_width();
    image.height = imlib_image_get_height();

    if (print_dimensions)
        printf("\033[01;31m%u\033[0;mx\033[01;31m%u\033[0;m\n",
               image.width, image.height);

    if (needs_rotation) {
        get_cache_name();
        cache_image(MIN(image.width, MAX_IMG_WIDTH));
    } else if (image.width > MAX_IMG_WIDTH) {
        get_cache_name();
        cache_image(CACHE_IMG_WIDTH);
    } else if (image.width > MAX_PNG_WIDTH) {
        magic_t my_magic;
        my_magic = magic_open(MAGIC_MIME_TYPE);
        magic_load(my_magic, NULL);
        if (!strcmp(magic_file(my_magic, image.basename), "image/png")) {
            get_cache_name();
            cache_image(CACHE_IMG_WIDTH);
        }
        magic_close(my_magic);
    } else if (ends_with(image.basename, "ff")) {
        get_cache_name();
        cache_image(image.width);
    } 

    do {
        File UEBERZUG_FIFO = {
            .file = NULL,
            .fd = -1,
            .name = NULL
        };

        if ((UEBERZUG_FIFO.name = getenv("UEBERZUG_FIFO")) == NULL) {
            fprintf(stderr, "stiv: UEBERZUG_FIFO environment variable is not set.\n");
            break;
        }
        if ((UEBERZUG_FIFO.fd = open(UEBERZUG_FIFO.name,
                                     O_WRONLY | O_NONBLOCK)) < 0) {
            fprintf(stderr, "stiv: Error opening %s: %s",
                            UEBERZUG_FIFO.name, strerror(errno));
            break;
        }

        if (image.fullpath == NULL) { 
            if (!(image.fullpath = realpath(image.basename, NULL))) {
                fprintf(stderr, "stiv: Error getting realpath of %s: %s",
                                image.fullpath, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        dprintf(UEBERZUG_FIFO.fd,
                "{\"action\": \"add\", \"identifier\": \"preview\","
                "\"scaler\": \"fit_contain\","
                "\"x\": %u, \"y\": %u, \"width\": %u, \"height\": %u,",
                terminal.x, terminal.y, terminal.width, terminal.height);
        dprintf(UEBERZUG_FIFO.fd, "\"path\": \"%s\"}\n", image.fullpath);

        util_close(&UEBERZUG_FIFO);
        free(image.fullpath);
        image.fullpath = NULL;
    } while (0);

    // it should return error so that programs will call it again to redraw
    exit(exit_code);
}

void
usage(FILE *stream) {
    fprintf(stream, "usage: stiv IMAGE W H [X Y]\n");
    fprintf(stream, "Be sure to have ueberzug running in the terminal "
                    "and UEBERZUG_FIFO env variable set\n");
    exit((int) (stream != stdout));
}

void
get_cache_name(void) {
    struct stat file;
    char buffer[PATH_MAX];
	int n;

    if (stat(image.basename, &file) < 0) {
        fprintf(stderr, "stiv: Error calling stat on %s: %s.",
                        image.basename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    n = snprintf(buffer, sizeof (buffer),
                 "%li_%ld_%ld",
                 file.st_size, file.st_mtim.tv_sec, file.st_mtim.tv_nsec);
	if (n < 0) {
		fprintf(stderr, "Error printing cache name.\n");
		exit(EXIT_FAILURE);
	}
    image.cachename = util_strdup(buffer);
    return;
}

void
cache_image(double new_width) {
    FILE *cache_img;
    char *XDG_CACHE_HOME = NULL;

    const char *preview = "preview/stiv";
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
                 "%s/%s/%s.jpg", XDG_CACHE_HOME, preview, image.cachename);
	if (n < 0) {
		fprintf(stderr, "Error printing cache name.\n");
		exit(EXIT_FAILURE);
	}
    image.fullpath = util_strdup(buffer);

    if ((cache_img = fopen(image.fullpath, "r"))) {
        fclose(cache_img);
        return;
    }
    if (errno == ENOENT) {
        Imlib_Image imlib_image;
        Imlib_Load_Error err;

        double z = image.width / new_width;
        new_height = round(((double) image.height / z));

        imlib_context_set_anti_alias(1);
        imlib_image = imlib_create_cropped_scaled_image(
                      0, 0, image.width, image.height,
                      (int) new_width, (int) new_height
                      );
        if (imlib_image == NULL) {
            fprintf(stderr, "Error in imlib_create_cropped_scaled_image()\n");
            goto dontcache;
        }

        imlib_context_set_image(imlib_image);

        if (imlib_image_has_alpha()) {
            imlib_image_set_format("png");
        } else {
            imlib_image_set_format("jpg");
            imlib_image_attach_data_value("quality", NULL, 90, NULL);
        }
        imlib_save_image_with_error_return(image.fullpath, &err);
        if (err) {
            fprintf(stderr, "Error caching image %s: %d\n", image.basename, err);
            goto dontcache;
        }

        imlib_free_image_and_decache();
        return;
    } else {
        fprintf(stderr, "Error opening %s: %s\n",
                        image.fullpath, strerror(errno));
    }
    dontcache:
    free(image.fullpath);
    image.fullpath = NULL;
    return;
}

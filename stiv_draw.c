/* This file is part of stiv.
 * Copyright (C) 2024 Lucas Mior

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

#include "util.c"

typedef struct Pane {
    int width;
    int height;
    int x;
    int y;
} Pane;

static Pane pane = {
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
    int width;
    int height;
} Image;

static Image image = {
    .basename = NULL,
    .fullpath = NULL,
    .width = 0,
    .height = 0,
};

typedef enum ImageType {
    IMAGE_TYPE_PNG,
    IMAGE_TYPE_WEBP,
    IMAGE_TYPE_OTHER,
} ImageType;

static int exit_code = EXIT_FAILURE;

static void usage(FILE *) __attribute__((noreturn));
static int cache_image(void);
static int exif_orientation(void);
static char * snprintf2(char *, size_t, char *, ...);
char *program;

int main(int argc, char *argv[]) {
    Number lines;
    Number columns;
    bool caching = false;
    FILE *cache_img;
    struct stat file;
    char *XDG_CACHE_HOME = NULL;
    const char *preview = "preview/stiv";

    program = basename(argv[0]);

    image.basename = argv[1];
    if ((argc == 3) && !strcmp(argv[2], "cache"))
        caching = true;


    if (stat(image.basename, &file) < 0) {
        error("Error calling stat on %s: %s.", image.basename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    {
        char buffer[PATH_MAX];
        snprintf2(buffer, sizeof(buffer), 
                  "%li_%ld_%ld",
                  file.st_size, file.st_mtim.tv_sec, file.st_mtim.tv_nsec);
        image.cachename = util_strdup(buffer);

        if ((XDG_CACHE_HOME = getenv("XDG_CACHE_HOME")) == NULL) {
            error("XDG_CACHE_HOME is not set. Exiting...\n");
            exit(EXIT_FAILURE);
        }

        snprintf2(buffer, sizeof (buffer),
                  "%s/%s/%s.jpg", XDG_CACHE_HOME, preview, image.cachename);
        image.fullpath = util_strdup(buffer);
    }

    if ((cache_img = fopen(image.fullpath, "r")) == NULL) {
        bool needs_rotation;

        if (errno != ENOENT) {
            error("Error opening %s: %s\n", image.fullpath, strerror(errno));
            image.fullpath = NULL;
        } else {
            const char *mime_type;
            magic_t magic;
            ImageType image_type = IMAGE_TYPE_OTHER;

            needs_rotation = exif_orientation();
            imlib_image_set_changes_on_disk();

            image.width = imlib_image_get_width();
            image.height = imlib_image_get_height();

            if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
                error("Error opening magic: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (magic_load(magic, NULL) < 0) {
                error("Error loading magic: %s.\n", magic_error(magic));
                exit(EXIT_FAILURE);
            }
            if ((mime_type = magic_file(magic, image.basename)) == NULL) {
                error("Error in magic_file: %s.\n", magic_error(magic));
                exit(EXIT_FAILURE);
            }
            if (!strcmp(mime_type, "image/png"))
                image_type = IMAGE_TYPE_PNG;
            if (!strcmp(mime_type, "image/webp"))
                image_type = IMAGE_TYPE_WEBP;

            magic_close(magic);

            if (needs_rotation 
                || (image.width > MAX_IMG_WIDTH)
                || ((image.width > MAX_PNG_WIDTH) && (image_type == IMAGE_TYPE_PNG))
                || (ends_with(image.basename, "ff"))
                || (image_type == IMAGE_TYPE_WEBP)) {
                if (cache_image() < 0)
                    image.fullpath = NULL;
            } else {
                image.fullpath = NULL;
            }
        }
    } else {
        Imlib_Image imlib_image;
        imlib_image = imlib_load_image(image.basename);
        imlib_context_set_image(imlib_image);
        image.width = imlib_image_get_width();
        image.height = imlib_image_get_height();
    }

    if (caching)
        exit(EXIT_FAILURE);

    if (print_dimensions) {
        printf("\033[01;31m%d\033[0;mx\033[01;31m%d\033[0;m\n",
               image.width, image.height);
    }

    if (argc >= 6) {
        // chamado por `lf > piscou > stiv`
        pane.width = util_string_int32(argv[2]);
        pane.height = util_string_int32(argv[3]) - 1;
        pane.x = util_string_int32(argv[4]);
        pane.y = util_string_int32(argv[5]) + 1;
        /* pane.y += 1; // tmux bugs lf's Y by 1 */

        pane.width -= 2;
        pane.x += 2;
        if (argc >= 7) {
            print_dimensions = false;
            pane.y -= 1;
        }
    } else if ((columns.string = getenv("FZF_PREVIEW_COLUMNS"))
            && (lines.string = getenv("FZF_PREVIEW_LINES"))) {
        // chamado por `fzf > piscou > stiv`
        pane.width = util_string_int32(columns.string);
        pane.height = util_string_int32(lines.string);

        pane.x = pane.width + (pane.width % 2);
        pane.y = 1;
    } else if ((columns.string = getenv("COLUMNS"))
            && (lines.string = getenv("LINES"))) {
        // chamado por `skim > piscou > stiv`
        pane.width = util_string_int32(columns.string);
        pane.height = util_string_int32(lines.string);

        pane.x = pane.width + 1 + ((pane.width + 1) % 2) + 1;
        pane.y = 1;

        // skim won't print anything if we exit with an error
        exit_code = EXIT_SUCCESS;
    } else if (argc == 4) {
        // chamado por `zsh > stiv`
        columns.string = argv[2];
        lines.string = argv[3];
        columns.number = util_string_int32(columns.string);
        lines.number = util_string_int32(lines.string);

        pane.width = columns.number;
        pane.height = HEIGHT_SHELL;
        pane.x = 0;
        pane.y = 1;
    } else {
        usage(stderr);
    }

    do {
        File UEBERZUG_FIFO = {
            .file = NULL,
            .fd = -1,
            .name = NULL
        };

        if ((UEBERZUG_FIFO.name = getenv("UEBERZUG_FIFO")) == NULL) {
            error("UEBERZUG_FIFO environment variable is not set.\n");
            break;
        }
        if ((UEBERZUG_FIFO.fd = open(UEBERZUG_FIFO.name,
                                     O_WRONLY | O_NONBLOCK)) < 0) {
            error("Error opening %s: %s", UEBERZUG_FIFO.name, strerror(errno));
            break;
        }

        if (image.fullpath == NULL) {
            if (!(image.fullpath = realpath(image.basename, NULL))) {
                error("Error getting realpath of %s: %s",
                      image.fullpath, strerror(errno));
                dprintf(UEBERZUG_FIFO.fd, UEBERZUG_CLEAR);
                util_close(&UEBERZUG_FIFO);
                break;
            }
        }

        dprintf(UEBERZUG_FIFO.fd,
                "{\"action\": \"add\", \"identifier\": \"preview\","
                "\"x\": %d, \"y\": %d, \"max_width\": %d, \"max_height\": %d,",
                pane.x, pane.y, pane.width, pane.height);
        dprintf(UEBERZUG_FIFO.fd, "\"path\": \"%s\"}\n", image.fullpath);

        util_close(&UEBERZUG_FIFO);
        free(image.fullpath);
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

int
cache_image(void) {
    Imlib_Image imlib_image;
    Imlib_Load_Error err;
    int new_width = image.width;
    double new_height;
    double resize_ratio;

    while (new_width > MAX_CACHE_WIDTH)
        new_width /= 2;

    resize_ratio = (double)image.width / (double)new_width;
    new_height = round(((double)image.height / resize_ratio));

    imlib_context_set_anti_alias(1);
    imlib_image = imlib_create_cropped_scaled_image(0, 0,
                                                    image.width, image.height,
                                                    (int)new_width,
                                                    (int)new_height);
    if (imlib_image == NULL) {
        error("Error in imlib_create_cropped_scaled_image()\n");
        return -1;
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
        error("Error caching image %s: %d\n", image.basename, err);
        return -1;
    }

    imlib_free_image_and_decache();
    return 0;
}

int
exif_orientation(void) {
    Imlib_Image imlib_image;
    ExifData *exif_data;
    ExifEntry *exif_entry;
    ExifByteOrder byte_order;
    int orientation = 0;

    imlib_image = imlib_load_image(image.basename);
    imlib_context_set_image(imlib_image);
    imlib_image_set_changes_on_disk();

    if ((exif_data = exif_data_new_from_file(image.basename)) == NULL)
        return 0;

    byte_order = exif_data_get_byte_order(exif_data);
    exif_entry = exif_content_get_entry(exif_data->ifd[EXIF_IFD_0],
                                        EXIF_TAG_ORIENTATION);
    if (exif_entry)
        orientation = exif_get_short(exif_entry->data, byte_order);

    exif_data_unref(exif_data);

    switch (orientation) {
    case 3:
        imlib_rotate_image_from_buffer(180, imlib_image);
        break;
    case 6:
        imlib_rotate_image_from_buffer(270, imlib_image);
        break;
    case 8:
        imlib_rotate_image_from_buffer(90, imlib_image);
        break;
    default:
        return false;
    }
    return true;
}

char *
snprintf2(char *buffer, size_t size, char *format, ...) {
    int n;
    va_list args;

    va_start(args, format);
    n = snprintf(buffer, size, format, args);
    va_end(args);

    if (n >= (int)size) {
        error("Error in snprintf: Too long string.\n");
        exit(EXIT_FAILURE);
    }
    if (n < 0) {
        error("Error printing cache name.\n");
        exit(EXIT_FAILURE);
    }
    return buffer;
}

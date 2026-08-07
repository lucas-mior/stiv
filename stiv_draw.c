#define ERROR_NOTIFY 1
#define CBASE_IMPLEMENT
#include "cbase.h"

#include "stiv.h"
#include <Imlib2.h>
#include <libexif/exif-data.h>
#include <magic.h>

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
    char *path;
    char *fullpath;
    int32 path_len;
    int32 fullpath_len;
    int32 width;
    int32 height;
} Image;

static Image image = {
    .path = NULL,
    .fullpath = NULL,
    .width = 0,
    .height = 0,
};

typedef enum ImageType {
    IMAGE_TYPE_PNG,
    IMAGE_TYPE_WEBP,
    IMAGE_TYPE_OTHER,
} ImageType;

enum StivBackend {
    STIV_BACKEND_CHAFA,
    STIV_BACKEND_UEBERZUG,
};

static int exit_code = EXIT_FAILURE;

static void usage(FILE *) __attribute__((noreturn));
static int cache_image(void);
static int exif_orientation(void);

int
main(int argc, char *argv[]) {
    Number lines;
    Number columns;
    bool caching = false;
    int cache_img;
    char *STIV_BACKEND;
    enum StivBackend stiv_backend = STIV_BACKEND_UEBERZUG;

    GETENV(STIV_BACKEND);
    if (STIV_BACKEND) {
        if (strequal(STIV_BACKEND, "chafa")) {
            stiv_backend = STIV_BACKEND_CHAFA;
        } else {
            stiv_backend = STIV_BACKEND_UEBERZUG;
        }
    }

    program = basename(argv[0]);

    if (argc <= 1) {
        usage(stderr);
    }

    image.path = argv[1];
    if ((argc == 3) && strequal(argv[2], "cache")) {
        caching = true;
    }
    image.path_len = strlen32(image.path);

    {
        const char *preview = "preview/stiv";
        char *XDG_CACHE_HOME = NULL;
        struct stat file;
        char buffer[PATH_MAX];
        int32 n;

        if (stat(image.path, &file) < 0) {
            error("Error calling stat on %s: %s.", image.path, strerror(errno));
            exit(EXIT_FAILURE);
        }

        GETENV(XDG_CACHE_HOME);
        if (XDG_CACHE_HOME == NULL) {
            exit(EXIT_FAILURE);
        }

        n = SNPRINTF(buffer,
                     "%s/%s/%ld_%ld_%ld.jpg",
                     XDG_CACHE_HOME, preview,
                     file.st_size, file.st_mtim.tv_sec, file.st_mtim.tv_nsec);
        ASSERT(n >= 0);

        image.fullpath = xmemdup(buffer, n + 1);
        image.fullpath_len = n;
    }

    if ((cache_img = open(image.fullpath, O_RDONLY)) >= 0) {
        Imlib_Image imlib_image;
        imlib_image = imlib_load_image_fd(cache_img, image.path);
        imlib_context_set_image(imlib_image);
        image.width = imlib_image_get_width();
        image.height = imlib_image_get_height();
    } else if (errno != ENOENT) {
        error("Error opening %s: %s\n", image.fullpath, strerror(errno));
        image.fullpath = NULL;
    } else {
        const char *mime_type0;
        char *mime_type;
        int32 mime_type_len;
        magic_t magic;
        ImageType image_type = IMAGE_TYPE_OTHER;
        bool needs_rotation = exif_orientation();
        bool is_gif = false;
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
        if ((mime_type0 = magic_file(magic, image.path)) == NULL) {
            error("Error in magic_file: %s.\n", magic_error(magic));
            exit(EXIT_FAILURE);
        }
        mime_type = (char *)mime_type0;
        mime_type_len = strlen32(mime_type);
        if (STREQUAL(mime_type, mime_type_len, "image/png")) {
            image_type = IMAGE_TYPE_PNG;
        }
        if (STREQUAL(mime_type, mime_type_len, "image/webp")) {
            image_type = IMAGE_TYPE_WEBP;
        }
        if (STREQUAL(mime_type, mime_type_len, "image/gif")) {
            is_gif = true;
        }

        magic_close(magic);

        if (!is_gif && (needs_rotation || (image.width > MAX_IMG_WIDTH)
            || ((image.width > MAX_PNG_WIDTH) && (image_type == IMAGE_TYPE_PNG))
            || (ENDS_WITH(image.path, image.path_len, "ff"))
            || (image_type == IMAGE_TYPE_WEBP))) {
            if (cache_image() < 0) {
                image.fullpath = NULL;
            }
        } else {
            image.fullpath = NULL;
        }
    }

    if (caching) {
        exit(EXIT_FAILURE);
    }

    if (print_dimensions) {
        printf("\033[01;31m%d\033[0;mx\033[01;31m%d\033[0;m\n",
               image.width, image.height);
    }
    fflush(stdout);

    if (argc >= 6) {
        // chamado por `lf > piscou > stiv`
        pane.width = atoi(argv[2]);
        pane.height = atoi(argv[3]) - 1;
        pane.x = atoi(argv[4]);
        pane.y = atoi(argv[5]) + 1;
        /* pane.y += 1; // tmux bugs lf's Y by 1 */

        pane.width -= 2;
        if (argc >= 7) {
            print_dimensions = false;
            pane.y -= 1;
        }
    } else if ((columns.string = getenv("FZF_PREVIEW_COLUMNS"))
               && (lines.string = getenv("FZF_PREVIEW_LINES"))) {
        // chamado por `fzf > piscou > stiv`
        pane.width = atoi(columns.string);
        pane.height = atoi(lines.string);

        pane.x = pane.width + (pane.width % 2);
        pane.y = 1;
    } else if ((columns.string = getenv("COLUMNS"))
               && (lines.string = getenv("LINES"))) {
        // chamado por `skim > piscou > stiv`
        pane.width = atoi(columns.string);
        pane.height = atoi(lines.string);

        pane.x = pane.width + 1 + ((pane.width + 1) % 2) + 1;
        pane.y = 1;

        // skim won't print anything if we exit with an error
        exit_code = EXIT_SUCCESS;
    } else if (argc == 4) {
        // chamado por `zsh > stiv`
        columns.string = argv[2];
        lines.string = argv[3];
        columns.number = atoi(columns.string);
        lines.number = atoi(lines.string);

        pane.width = columns.number;
        pane.height = HEIGHT_SHELL;
        pane.x = 0;
        pane.y = 1;
    } else {
        usage(stderr);
    }

    switch (stiv_backend) {
    case STIV_BACKEND_UEBERZUG:
        do {
            char *UEBERZUG_FIFO;
            int32 ueberzug_fd;

            GETENV(UEBERZUG_FIFO);
            if (UEBERZUG_FIFO == NULL) {
                break;
            }
            if ((ueberzug_fd
                    = open(UEBERZUG_FIFO, O_WRONLY | O_NONBLOCK)) < 0) {
                error("Error opening %s in non blocking mode: %s",
                      UEBERZUG_FIFO, strerror(errno));
                break;
            }

            if (image.fullpath == NULL) {
                ASSERT(image.path != NULL);
                if (!(image.fullpath = realpath(image.path, NULL))) {
                    error("Error getting realpath of %s: %s", image.path,
                          strerror(errno));
                    dprintf(ueberzug_fd, UEBERZUG_CLEAR);
                    XCLOSE(&ueberzug_fd, UEBERZUG_FIFO);
                    break;
                }
            }

            dprintf(ueberzug_fd,
                    "{\"action\": \"add\", \"identifier\": \"preview\","
                    "\"x\": %d, \"y\": %d, \"max_width\": %d, \"max_height\": %d,",
                    pane.x, pane.y, pane.width, pane.height);
            dprintf(ueberzug_fd, "\"path\": \"%s\"}\n", image.fullpath);
            if (DEBUGGING) {
                dprintf(STDERR_FILENO,
                        "{\"action\": \"add\", \"identifier\": \"preview\","
                        "\"x\": %d, \"y\": %d, \"max_width\": %d, \"max_height\": %d,",
                        pane.x, pane.y, pane.width, pane.height);
                dprintf(STDERR_FILENO, "\"path\": \"%s\"}\n", image.fullpath);
            }

            XCLOSE(&ueberzug_fd, UEBERZUG_FIFO);
        } while (0);
        break;
    case STIV_BACKEND_CHAFA:
        do {
            char size[64];
            char *chafa[64];
            int32 nargs = 0;

            if (image.fullpath == NULL) {
                ASSERT(image.path != NULL);
                if (!(image.fullpath = realpath(image.path, NULL))) {
                    error("Error getting realpath of %s: %s\n",
                          image.path, strerror(errno));
                    break;
                }
            }

            SNPRINTF(size, "--size=%dx%d", pane.width, pane.height);

            chafa[nargs++] = "chafa";
            /* chafa[nargs++] = "--clear"; */
            chafa[nargs++] = "--polite=on";
            chafa[nargs++] = "--animate=off";
            chafa[nargs++] = "--format=sixels";
            chafa[nargs++] = size;
            chafa[nargs++] = image.fullpath;
            chafa[nargs++] = NULL;

            switch (fork()) {
            case -1:
                error("Error forking: %s.\n", strerror(errno));
                fatal(EXIT_FAILURE);
            case 0:
                {
                    char cmd[4096];
                    execvp(chafa[0], chafa);
                    STRING_FROM_ARRAY(cmd, " ", chafa, nargs);
                    error("Error executing\n\n%s\n\n%s.\n",
                            cmd, strerror(errno));
                    exit(EXIT_FAILURE);
                }
            default:
                break;
            }

            free(image.fullpath);
        } while (0);
        break;
    default:
        error("This backend does not exist.\n");
        fatal(EXIT_FAILURE);
    }

    // it should return error so that programs will call it again to redraw
    exit(exit_code);
}

void
usage(FILE *stream) {
    fprintf(stream, "usage: stiv IMAGE W H [X Y]\n");
    fprintf(stream, "Be sure to have ueberzug running in the terminal "
                    "and UEBERZUG_FIFO env variable set\n");
    exit((int)(stream != stdout));
}

int
cache_image(void) {
    Imlib_Image imlib_image;
    Imlib_Load_Error err;
    int new_width = image.width;
    double new_height;
    double resize_ratio;

    while (new_width > MAX_CACHE_WIDTH) {
        new_width /= 2;
    }

    resize_ratio = (double)image.width / (double)new_width;
    new_height = round(((double)image.height / resize_ratio));

    imlib_context_set_anti_alias(1);
    imlib_image = imlib_create_cropped_scaled_image(
        0, 0, image.width, image.height, new_width, (int)new_height);
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
        error("Error caching image\n%s at %s:\n%s\n", image.path,
              image.fullpath, imlib_strerror((int)err));
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

    imlib_image = imlib_load_image(image.path);
    imlib_context_set_image(imlib_image);
    imlib_image_set_changes_on_disk();

    if ((exif_data = exif_data_new_from_file(image.path)) == NULL) {
        return 0;
    }

    byte_order = exif_data_get_byte_order(exif_data);
    exif_entry = exif_content_get_entry(exif_data->ifd[EXIF_IFD_0],
                                        EXIF_TAG_ORIENTATION);
    if (exif_entry) {
        orientation = exif_get_short(exif_entry->data, byte_order);
    }

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

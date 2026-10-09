#if !defined(STIV_DRAW_C)
#define STIV_DRAW_C

#define ERROR_NOTIFY 1
#define CBASE_IMPLEMENT 1
#include "cbase.h"

#include "stiv.h"
#include <Imlib2.h>
#include <libexif/exif-data.h>
#include <magic.h>

typedef struct Pane {
    int32 width;
    int32 height;
    int32 x;
    int32 y;
} Pane;

typedef struct Image {
    char *path;
    char *fullpath;

    int32 path_len;
    int32 fullpath_len;
    int32 width;
    int32 height;
} Image;

enum ImageType {
    IMAGE_TYPE_PNG = 1,
    IMAGE_TYPE_WEBP,
    IMAGE_TYPE_OTHER,
};

enum StivBackend {
    STIV_BACKEND_CHAFA = 1,
    STIV_BACKEND_UEBERZUG,
};

static Pane pane = {
    .width = 100,
    .height = HEIGHT_SHELL,
    .x = 0,
    .y = 1,
};

static Image image = {0};
static bool print_dimensions = true;
static int32 exit_code = EXIT_FAILURE;

static void
stiv_usage(void) {
    error2("usage: stiv IMAGE W H [X Y]\n");
    error2("Be sure to have ueberzug running in the terminal "
           "and UEBERZUG_FIFO env variable set\n");
    return;
}

static int32
stiv_parse_int32(char *string, int32 *result) {
    llong parsed_value;
    int32 parsed_len;
    int32 string_len;

    ASSERT(result != NULL);
    if (string == NULL) {
        return -EINVAL;
    }

    string_len = strlen32(string);
    if ((parsed_len = parse_integer(string, string_len, &parsed_value)) < 0) {
        return parsed_len;
    }
    if (parsed_len != string_len) {
        return -EINVAL;
    }
    if ((parsed_value <= MINOF((int32)0))
        || (parsed_value >= MAXOF((int32)0))) {
        return -ERANGE;
    }

    *result = (int32)parsed_value;
    return 0;
}

static void
stiv_fullpath_clear(void) {
    free2(image.fullpath, image.fullpath_len + 1);
    image.fullpath = NULL;
    image.fullpath_len = 0;
    return;
}

static int32
stiv_fullpath_resolve(void) {
    char resolved_path[PATH_MAX];
    int32 resolved_path_len;
    int32 err;

    ASSERT(image.path != NULL);
    ASSERT(image.fullpath == NULL);

    if (realpath(image.path, resolved_path) == NULL) {
        err = errno;
        return -err;
    }
    resolved_path_len = strlen32(resolved_path);
    image.fullpath = xmemdup(resolved_path, resolved_path_len + 1);
    image.fullpath_len = resolved_path_len;
    return 0;
}

static int32
stiv_image_load(bool *needs_rotation) {
    Imlib_Image imlib_image;
    ExifData *exif_data;
    ExifEntry *exif_entry;
    ExifByteOrder byte_order;
    int32 orientation = 0;

    ASSERT(needs_rotation != NULL);
    *needs_rotation = false;

    if ((imlib_image = imlib_load_image(image.path)) == NULL) {
        return -EIO;
    }
    imlib_context_set_image(imlib_image);
    imlib_image_set_changes_on_disk();

    if ((exif_data = exif_data_new_from_file(image.path)) == NULL) {
        return 0;
    }

    byte_order = exif_data_get_byte_order(exif_data);
    exif_entry = exif_content_get_entry(exif_data->ifd[EXIF_IFD_0],
                                        EXIF_TAG_ORIENTATION);
    if (exif_entry != NULL) {
        orientation = exif_get_short(exif_entry->data, byte_order);
    }
    exif_data_unref(exif_data);

    switch (orientation) {
    case 3:
        imlib_rotate_image_from_buffer(180, imlib_image);
        *needs_rotation = true;
        break;
    case 6:
        imlib_rotate_image_from_buffer(270, imlib_image);
        *needs_rotation = true;
        break;
    case 8:
        imlib_rotate_image_from_buffer(90, imlib_image);
        *needs_rotation = true;
        break;
    default:
        break;
    }
    return 0;
}

static int32
stiv_cache_image(void) {
    Imlib_Image imlib_image;
    Imlib_Load_Error imlib_error;
    int32 new_width = image.width;
    int32 new_height_int;
    double new_height;
    double resize_ratio;

    while (new_width > MAX_CACHE_WIDTH) {
        new_width /= 2;
    }

    resize_ratio = (double)image.width / (double)new_width;
    new_height = round((double)image.height / resize_ratio);
    if ((new_height <= MINOF((int32)0))
        || (new_height >= MAXOF((int32)0))) {
        error("Invalid scaled image height: %g\n", new_height);
        return -ERANGE;
    }
    new_height_int = (int32)new_height;

    imlib_context_set_anti_alias(1);
    imlib_image = imlib_create_cropped_scaled_image(0, 0,
                                                    image.width, image.height,
                                                    new_width, new_height_int);
    if (imlib_image == NULL) {
        error("Error in imlib_create_cropped_scaled_image()\n");
        return -EIO;
    }

    imlib_context_set_image(imlib_image);
    if (imlib_image_has_alpha()) {
        imlib_image_set_format("png");
    } else {
        imlib_image_set_format("jpg");
        imlib_image_attach_data_value("quality", NULL, 90, NULL);
    }

    imlib_save_image_with_error_return(image.fullpath, &imlib_error);
    if (imlib_error != 0) {
        error("Error caching image\n%s at %s:\n%s\n",
              image.path, image.fullpath,
              imlib_strerror((int32)imlib_error));
        imlib_free_image_and_decache();
        return -EIO;
    }

    imlib_free_image_and_decache();
    return 0;
}

int
main(int argc, char **argv) {
    char *fzf_columns;
    char *fzf_lines;
    char *terminal_columns;
    char *terminal_lines;
    char *stiv_backend_env;
    bool caching = false;
    int32 cache_img;
    enum StivBackend stiv_backend = STIV_BACKEND_UEBERZUG;

    stiv_backend_env = getenv("STIV_BACKEND");
    if (stiv_backend_env != NULL) {
        int32 stiv_backend_env_len = strlen32(stiv_backend_env);

        if (STREQUAL(stiv_backend_env, stiv_backend_env_len, "chafa")) {
            stiv_backend = STIV_BACKEND_CHAFA;
        } else {
            stiv_backend = STIV_BACKEND_UEBERZUG;
        }
    }

    if (argc <= 1) {
        stiv_usage();
        exit(EXIT_FAILURE);
    }

    image.path = argv[1];
    image.path_len = strlen32(image.path);
    if (argc == 3) {
        int32 argument_len = strlen32(argv[2]);

        if (STREQUAL(argv[2], argument_len, "cache")) {
            caching = true;
        }
    }

    {
        struct stat file;
        char buffer[PATH_MAX];
        char *xdg_cache_home;
        int32 xdg_cache_home_len;
        int64 mtime_sec;
        int64 mtime_nsec;
        int32 n;

        if (stat(image.path, &file) < 0) {
            error("Error calling stat on %s: %s.", image.path, strerror(errno));
            fatal(EXIT_FAILURE);
        }

        xdg_cache_home = getenv("XDG_CACHE_HOME");
        if (xdg_cache_home == NULL) {
            exit(EXIT_FAILURE);
        }
        xdg_cache_home_len = strlen32(xdg_cache_home);

#if OS_MAC || OS_NETBSD
        mtime_sec = (int64)file.st_mtimespec.tv_sec;
        mtime_nsec = (int64)file.st_mtimespec.tv_nsec;
#else
        mtime_sec = (int64)file.st_mtim.tv_sec;
        mtime_nsec = (int64)file.st_mtim.tv_nsec;
#endif

        n = SNPRINTF(buffer,
                     "%.*s/preview/stiv/%lld_%lld_%lld.jpg",
                     xdg_cache_home_len, xdg_cache_home,
                     (int64)file.st_size, mtime_sec, mtime_nsec);
        image.fullpath = xmemdup(buffer, n + 1);
        image.fullpath_len = n;
    }

    if ((cache_img = open(image.fullpath, O_RDONLY)) >= 0) {
        Imlib_Image imlib_image;

        if ((imlib_image = imlib_load_image_fd(cache_img,
                                               image.path)) == NULL) {
            error("Error loading cached image %s.\n", image.fullpath);
            XCLOSE(&cache_img, image.fullpath);
            fatal(EXIT_FAILURE);
        }
        XCLOSE(&cache_img, image.fullpath);

        imlib_context_set_image(imlib_image);
        image.width = imlib_image_get_width();
        image.height = imlib_image_get_height();
    } else if (errno != ENOENT) {
        error("Error opening %s: %s\n", image.fullpath, strerror(errno));
        stiv_fullpath_clear();
    } else {
        char *mime_type;
        int32 mime_type_len;
        int32 load_status;
        magic_t magic;
        enum ImageType image_type = IMAGE_TYPE_OTHER;
        bool needs_rotation;
        bool is_gif = false;

        if ((load_status = stiv_image_load(&needs_rotation)) < 0) {
            error("Error loading image %s.\n", image.path);
            fatal(EXIT_FAILURE);
        }

        image.width = imlib_image_get_width();
        image.height = imlib_image_get_height();

        if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
            error("Error opening magic: %s\n", strerror(errno));
            fatal(EXIT_FAILURE);
        }
        if (magic_load(magic, NULL) < 0) {
            error("Error loading magic: %s.\n", magic_error(magic));
            magic_close(magic);
            fatal(EXIT_FAILURE);
        }
        if ((mime_type = (char *)magic_file(magic, image.path)) == NULL) {
            error("Error in magic_file: %s.\n", magic_error(magic));
            magic_close(magic);
            fatal(EXIT_FAILURE);
        }
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

        if (!is_gif
            && (needs_rotation
                || (image.width > MAX_IMG_WIDTH)
                || ((image.width > MAX_PNG_WIDTH)
                    && (image_type == IMAGE_TYPE_PNG))
                || ENDS_WITH(image.path, image.path_len, "ff")
                || (image_type == IMAGE_TYPE_WEBP))) {
            if (stiv_cache_image() < 0) {
                stiv_fullpath_clear();
            }
        } else {
            stiv_fullpath_clear();
        }
    }

    if (caching) {
        stiv_fullpath_clear();
        exit(EXIT_FAILURE);
    }

    if (print_dimensions) {
        printf("\033[01;31m%d\033[0;mx\033[01;31m%d\033[0;m\n",
               image.width, image.height);
    }
    fflush(stdout);

    fzf_columns = getenv("FZF_PREVIEW_COLUMNS");
    fzf_lines = getenv("FZF_PREVIEW_LINES");
    terminal_columns = getenv("COLUMNS");
    terminal_lines = getenv("LINES");

    if (argc >= 6) {
        if ((stiv_parse_int32(argv[2], &pane.width) < 0)
            || (stiv_parse_int32(argv[3], &pane.height) < 0)
            || (stiv_parse_int32(argv[4], &pane.x) < 0)
            || (stiv_parse_int32(argv[5], &pane.y) < 0)) {
            error("Invalid pane geometry.\n");
            fatal(EXIT_FAILURE);
        }

        pane.height -= 1;
        pane.y += 1;
        pane.width -= 2;
        if (argc >= 7) {
            print_dimensions = false;
            pane.y -= 1;
        }
    } else if ((fzf_columns != NULL) && (fzf_lines != NULL)) {
        if ((stiv_parse_int32(fzf_columns, &pane.width) < 0)
            || (stiv_parse_int32(fzf_lines, &pane.height) < 0)) {
            error("Invalid fzf preview geometry.\n");
            fatal(EXIT_FAILURE);
        }

        pane.x = pane.width + (pane.width % 2);
        pane.y = 1;
    } else if ((terminal_columns != NULL) && (terminal_lines != NULL)) {
        if ((stiv_parse_int32(terminal_columns, &pane.width) < 0)
            || (stiv_parse_int32(terminal_lines, &pane.height) < 0)) {
            error("Invalid terminal geometry.\n");
            fatal(EXIT_FAILURE);
        }

        pane.x = pane.width + 1 + ((pane.width + 1) % 2) + 1;
        pane.y = 1;

        /* skim will not print anything if we exit with an error. */
        exit_code = EXIT_SUCCESS;
    } else if (argc == 4) {
        if ((stiv_parse_int32(argv[2], &pane.width) < 0)
            || (stiv_parse_int32(argv[3], &pane.height) < 0)) {
            error("Invalid shell geometry.\n");
            fatal(EXIT_FAILURE);
        }

        pane.height = HEIGHT_SHELL;
        pane.x = 0;
        pane.y = 1;
    } else {
        stiv_usage();
        exit(EXIT_FAILURE);
    }

    switch (stiv_backend) {
    case STIV_BACKEND_UEBERZUG:
        do {
            char *ueberzug_fifo;
            int32 ueberzug_fd;

            ueberzug_fifo = getenv("UEBERZUG_FIFO");
            if (ueberzug_fifo == NULL) {
                break;
            }
            if ((ueberzug_fd = open(ueberzug_fifo,
                                    O_WRONLY |O_NONBLOCK)) < 0) {
                error("Error opening %s in non blocking mode: %s",
                      ueberzug_fifo, strerror(errno));
                break;
            }

            if (image.fullpath == NULL) {
                int32 resolve_status;

                resolve_status = stiv_fullpath_resolve();
                if (resolve_status < 0) {
                    error("Error getting realpath of %s: %s",
                          image.path, strerror(-resolve_status));
                    write_all(ueberzug_fd, STRLIT(UEBERZUG_CLEAR));
                    XCLOSE(&ueberzug_fd, ueberzug_fifo);
                    break;
                }
            }

            {
                String message = {0};

                str_printf(&message,
                           "{\"action\": \"add\", "
                           "\"identifier\": \"preview\","
                           "\"x\": %d, \"y\": %d, "
                           "\"max_width\": %d, \"max_height\": %d,",
                           pane.x, pane.y, pane.width, pane.height);
                str_printf(&message, "\"path\": \"%.*s\"}\n",
                                     image.fullpath_len, image.fullpath);
                write_all(ueberzug_fd, message.data, message.len);
                if (DEBUGGING) {
                    write_all(STDERR_FILENO, message.data, message.len);
                }
                str_free(&message);
            }

            XCLOSE(&ueberzug_fd, ueberzug_fifo);
        } while (0);
        break;
    case STIV_BACKEND_CHAFA:
        do {
            Command command = {0};
            int32 command_status;

            if (image.fullpath == NULL) {
                int32 resolve_status;

                resolve_status = stiv_fullpath_resolve();
                if (resolve_status < 0) {
                    error("Error getting realpath of %s: %s\n",
                          image.path, strerror(-resolve_status));
                    break;
                }
            }

            CMD_PUSH(&command, "chafa", "--polite=on", "--animate=off",
                     "--format=sixels");
            cmd_printf(&command, "--size=%dx%d", pane.width, pane.height);
            cmd_push_length(&command, image.fullpath, image.fullpath_len);

            command_status = cmd_run_async(&command, CMD_NONE);
            if (command_status < 0) {
                error("Error starting chafa: %s.\n",
                      strerror(-command_status));
                cmd_free(&command);
                fatal(EXIT_FAILURE);
            }
            cmd_free(&command);
        } while (0);
        break;
    default:
        error("This backend does not exist.\n");
        fatal(EXIT_FAILURE);
    }

    stiv_fullpath_clear();

    /* Return an error so callers try again and redraw the preview. */
    exit(exit_code);
}

#endif /* STIV_DRAW_C */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <Imlib2.h>

#include "stiv.h"

void get_img_size(Image *img) {
    Imlib_Image image;

    image = imlib_load_image(img->filename);
    imlib_context_set_image(image);

    img->w = imlib_image_get_width();
    img->h = imlib_image_get_height();

    imlib_free_image();
    /* imlib_free_image_and_decache(); */
    return;
}

void reduce_img_size(Image *img) {
    static char cache_file[200];
    FILE *cache_img;
    char *cache = NULL;

    const char previewer[] = "previewer";
    const char jpg[] = "jpg";

    Imlib_Image image;
    Imlib_Load_Error err;

    int new_w = 1000, new_h;
    float z;

    cache = getenv("XDG_CACHE_HOME");

    sprintf(cache_file, "%s/%s/%s.%s", cache, previewer, img->hash, jpg);

    if ((cache_img = fopen(cache_file, "r"))) {
        fclose(cache_img);
        img->filename = cache_file;
        return;
    } else if (errno == ENOENT) {
        image = imlib_load_image(img->filename);
        imlib_context_set_image(image);
        z = (float) img->w / (float) new_w;
        new_h = (int) (img->h / z);

        imlib_context_set_anti_alias(1);
        image = imlib_create_cropped_scaled_image(0, 0, img->w, img->h, new_w, new_h);
        if (image == NULL)
            return;

        imlib_context_set_image(image);

        if (imlib_image_has_alpha()) {
            imlib_image_set_format("png");
        } else {
            imlib_image_set_format("jpg");
            imlib_image_attach_data_value("quality", NULL, 90, NULL);
        }
        imlib_save_image_with_error_return(cache_file, &err);
        if (err)
            return;

        imlib_free_image_and_decache();
        img->filename = cache_file;
    }
}

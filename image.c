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

#include <Imlib2.h>
#include <stdlib.h>

#include "stiv.h"

void image_get_size(Image *img) {
    Imlib_Image image;

    image = imlib_load_image(img->filename);
    imlib_context_set_image(image);

    img->w = imlib_image_get_width();
    img->h = imlib_image_get_height();

    imlib_free_image();
    /* imlib_free_image_and_decache(); */
    return;
}

void image_reduce_size(Image *img, double new_w) {
    FILE *cache_img;
    char *cache = NULL;

    const char previewer[] = "previewer/stiv";
    const char jpg[] = "jpg";

    Imlib_Image image;
    Imlib_Load_Error err;

    double new_h;
    if (new_w > MAX_IMG_WIDTH)
        new_w = CACHE_IMG_WIDTH;

    if ((cache = getenv("XDG_CACHE_HOME")) == NULL) {
        printf("XDG_CACHE_HOME is not set. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    img->path = util_realloc(NULL, 200);
    snprintf(img->path, 200, "%s/%s/%s.%s", cache, previewer, img->cache, jpg);

    if ((cache_img = fopen(img->path, "r"))) {
        fclose(cache_img);
        return;
    } else if (errno == ENOENT) {
        double z = img->w / new_w;
        new_h = round(((double) img->h / z));

        image = imlib_load_image(img->filename);
        imlib_context_set_image(image);
        imlib_context_set_anti_alias(1);
        image = imlib_create_cropped_scaled_image(0, 0, img->w, img->h, 
                                                        (int) new_w, (int) new_h);
        if (image == NULL)
            goto dontcache;

        imlib_context_set_image(image);

        if (imlib_image_has_alpha()) {
            imlib_image_set_format("png");
        } else {
            imlib_image_set_format("jpg");
            imlib_image_attach_data_value("quality", NULL, 90, NULL);
        }
        imlib_save_image_with_error_return(img->path, &err);
        if (err)
            goto dontcache;

        imlib_free_image_and_decache();
        return;
    }
    dontcache:
    free(img->path);
    img->path = NULL;
    return;
}

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

#include <Imlib2.h>

#include "stiv.h"

void image_get_size(Image *image) {
    Imlib_Image imlib_image;

    imlib_image = imlib_load_image(image->basename);
    imlib_context_set_image(imlib_image);

    image->width = imlib_image_get_width();
    image->height = imlib_image_get_height();

    imlib_free_image();
    /* imlib_free_image_and_decache(); */
    return;
}

void image_reduce_size(Image *image, double new_width) {
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

    n = snprintf(buffer, sizeof(buffer), 
                "%s/%s/%s.%s", XDG_CACHE_HOME, previewer, image->cachename, jpg);
	if (n < 0) {
		fprintf(stderr, "Error printing cache name.\n");
		exit(EXIT_FAILURE);
	}
    image->fullpath = strdup(buffer);

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

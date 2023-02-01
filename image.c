/*
 * image.c -- image definition
 *
 * Copyright (c) 2020-2023 David Demelier <markand@malikania.fr>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <magic.h>

#include "image.h"
#include "log.h"
#include "util.h"

static const char *mimes[] = {
	"image/bmp",
	"image/jpeg",
	"image/gif",
	"image/png",
	"image/svg+xml",
	"image/tiff",
	"image/webp"
};

void
image_finish(struct image *image)
{
	assert(image);

	free(image->id);
	free(image->title);
	free(image->author);
	free(image->data);
	free(image->filename);
	memset(image, 0, sizeof (*image));
}

bool
image_isvalid(const void *src, size_t srcsz)
{
	assert(src);

	magic_t cookie;
	bool ret = false;
	const char *mime;

	/*
	 * If we're unable to load the libmagic we assume it's valid image and
	 * let the browser show an invalid thing instead.
	 */
	if (!(cookie = magic_open(MAGIC_MIME_TYPE)))
		return true;

	magic_load(cookie, NULL);

	if ((mime = magic_buffer(cookie, src, srcsz))) {
		for (size_t i = 0; i < NELEM(mimes); ++i) {
			if (strcmp(mime, mimes[i]) == 0) {
				ret = true;
				break;
			}
		}
	}

	magic_close(cookie);

	return ret;
}

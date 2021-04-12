/*
 * page-index.c -- page /
 *
 * Copyright (c) 2020-2021 David Demelier <markand@malikania.fr>
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

#include <sys/types.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <kcgi.h>

#include "database.h"
#include "fragment-image.h"
#include "image.h"
#include "page-index.h"
#include "page.h"
#include "util.h"

struct template {
	struct kreq *req;
	const struct image *images;
	size_t imagesz;
};

static const char *keywords[] = {
	"images"
};

static int
template(size_t keyword, void *arg)
{
	struct template *tp = arg;

	switch (keyword) {
	case 0:
		for (size_t i = 0; i < tp->imagesz; ++i)
			fragment_image(tp->req, &tp->images[i]);
		break;
	default:
		break;
	}

	return 1;
}

static void
get(struct kreq *r)
{
	struct image images[10] = {0};
	size_t imagesz = NELEM(images);

	if (!database_recents(images, &imagesz))
		page(r, NULL, KHTTP_500, "pages/500.html");
	else
		page_index_render(r, images, imagesz);

	for (size_t i = 0; i < imagesz; ++i)
		image_finish(&images[i]);
}

void
page_index_render(struct kreq *r, const struct image *images, size_t imagesz)
{
	struct template data = {
		.req = r,
		.images = images,
		.imagesz = imagesz
	};

	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.arg = &data,
		.cb = template
	};

	page(r, &kt, KHTTP_200, "pages/index.html");
}

void
page_index(struct kreq *r)
{
	assert(r);

	switch (r->method) {
	case KMETHOD_GET:
		get(r);
		break;
	default:
		page(r, NULL, KHTTP_400, "pages/400.html");
		break;
	}
}

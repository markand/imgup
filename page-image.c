/*
 * page-image.c -- page /image/<id>
 *
 * Copyright (c) 2020 David Demelier <markand@malikania.fr>
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
#include <stddef.h>
#include <stdint.h>

#include <kcgi.h>

#include "database.h"
#include "image.h"
#include "page-image.h"
#include "page.h"
#include "util.h"

struct template {
	struct kreq *req;
	struct image *image;
};

static const char *keywords[] = {
	"id",
	"title",
	"author",
	"filename",
	"date",
	"visible",
	"expiration"
};

static int
template(size_t keyword, void *arg)
{
	const struct template *tp = arg;

	switch (keyword) {
	case 0:
		khttp_puts(tp->req, tp->image->id);
		break;
	case 1:
		khttp_puts(tp->req, tp->image->title);
		break;
	case 2:
		khttp_puts(tp->req, tp->image->author);
		break;
	case 3:
		khttp_puts(tp->req, tp->image->filename);
		break;
	case 4:
		khttp_puts(tp->req, bstrftime("%c", localtime(&tp->image->timestamp)));
		break;
	case 5:
		khttp_puts(tp->req, bprintf(tp->image->visible ? "Yes" : "No"));
		break;
	case 6:
		khttp_puts(tp->req, ttl(tp->image->timestamp, tp->image->duration));
		break;
	default:
		break;
	}

	return 1;
}

static void
get(struct kreq *r)
{
	struct image image = {0};
	struct template data = {
		.req = r,
		.image = &image
	};

	if (!database_get(&image, r->path))
		page(r, NULL, KHTTP_404, "pages/404.html");
	else {
		const struct ktemplate kt = {
			.key = keywords,
			.keysz = NELEM(keywords),
			.cb = template,
			.arg = &data
		};

		page(r, &kt, KHTTP_200, "pages/image.html");
		image_finish(&image);
	}
}

void
page_image(struct kreq *r)
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

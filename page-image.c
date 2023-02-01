/*
 * page-image.c -- page /image/<id>
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

#include <sys/types.h>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <kcgi.h>
#include <kcgihtml.h>

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
	"author",
	"date",
	"expiration",
	"filename",
	"id",
	"public",
	"title"
};

static int
template(size_t keyword, void *arg)
{
	const struct template *tp = arg;
	struct khtmlreq html;

	khtml_open(&html, tp->req, KHTML_PRETTY);

	switch (keyword) {
	case 0:
		khtml_puts(&html, tp->image->author);
		break;
	case 1:
		khtml_puts(&html, bstrftime("%c", localtime(&tp->image->timestamp)));
		break;
	case 2:
		khtml_puts(&html, ttl(tp->image->timestamp, tp->image->duration));
		break;
	case 3:
		khtml_puts(&html, tp->image->filename);
		break;
	case 4:
		khtml_puts(&html, tp->image->id);
		break;
	case 5:
		khtml_puts(&html, bprintf(tp->image->visible ? "Yes" : "No"));
		break;
	case 6:
		khtml_puts(&html, tp->image->title);
		break;
	default:
		break;
	}

	khtml_close(&html);

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
	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.cb = template,
		.arg = &data
	};

	if (!database_get(&image, r->path))
		page(r, NULL, KHTTP_404, "pages/404.html", "404");
	else {
		page(r, &kt, KHTTP_200, "pages/image.html", image.title);
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
		page(r, NULL, KHTTP_400, "pages/400.html", "400");
		break;
	}
}

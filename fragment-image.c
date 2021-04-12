/*
 * fragment-image.c -- image index renderer
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
#include <time.h>

#include <kcgi.h>
#include <kcgihtml.h>

#include "fragment-image.h"
#include "fragment.h"
#include "image.h"
#include "util.h"

struct template {
	struct kreq *req;
	const struct image *image;
};

static const char *keywords[] = {
	"id",
	"title",
	"author",
	"date",
	"expiration"
};

static int
template(size_t keyword, void *arg)
{
	struct template *tp = arg;
	struct khtmlreq html;

	khtml_open(&html, tp->req, KHTML_PRETTY);

	switch (keyword) {
	case 0:
		khtml_puts(&html, tp->image->id);
		break;
	case 1:
		khtml_puts(&html, tp->image->title);
		break;
	case 2:
		khtml_puts(&html, tp->image->author);
		break;
	case 3:
		khtml_puts(&html, bstrftime("%c", localtime(&tp->image->timestamp)));
		break;
	case 4:
		khtml_puts(&html, ttl(tp->image->timestamp, tp->image->duration));
		break;
	default:
		break;
	}

	khtml_close(&html);

	return 1;
}

void
fragment_image(struct kreq *r, const struct image *image)
{
	assert(r);
	assert(image);

	struct template data = {
		.req = r,
		.image = image,
	};
	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.cb = template,
		.arg = &data
	};

	fragment(r, &kt, "fragments/image.html");
}

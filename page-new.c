/*
 * page-new.c -- page /new
 *
 * Copyright (c) 2020-2022 David Demelier <markand@malikania.fr>
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
#include <string.h>

#include <kcgi.h>

#include "database.h"
#include "fragment-duration.h"
#include "image.h"
#include "page-new.h"
#include "page.h"
#include "util.h"

static const char *keywords[] = {
	"durations"
};

static const struct {
	const char *title;
	long long int secs;
} durations[] = {
	{ "day",        IMAGE_DURATION_DAY      },
	{ "hour",       IMAGE_DURATION_HOUR     },
	{ "week",       IMAGE_DURATION_WEEK     },
	{ "month",      IMAGE_DURATION_MONTH    },
};

static int
template(size_t keyword, void *arg)
{
	struct kreq *r = arg;

	switch (keyword) {
	case 0:
		for (size_t i = 0; i < NELEM(durations); ++i)
			fragment_duration(r, durations[i].title);
		break;
	default:
		break;
	}

	return 1;
}

static long long int
duration(const char *val)
{
	for (size_t i = 0; i < NELEM(durations); ++i)
		if (strcmp(val, durations[i].title) == 0)
			return durations[i].secs;

	/* Default to month. */
	return IMAGE_DURATION_MONTH;
}

static void
get(struct kreq *r)
{
	struct ktemplate kt = {
		.key = keywords,
		.keysz = NELEM(keywords),
		.cb = template,
		.arg = r
	};

	page(r, &kt, KHTTP_200, "pages/new.html", "Upload image");
}

static void
post(struct kreq *r)
{
	struct image image = {
		.author         = estrdup("Anonymous"),
		.title          = estrdup("Untitled"),
		.visible        = true,
		.duration       = IMAGE_DURATION_DAY
	};
	int raw = 0;

	for (size_t i = 0; i < r->fieldsz; ++i) {
		const char *key = r->fields[i].key;
		const char *val = r->fields[i].val;

		if (strcmp(key, "title") == 0)
			replace(&image.title, val);
		else if (strcmp(key, "author") == 0)
			replace(&image.author, val);
		else if (strcmp(key, "duration") == 0)
			image.duration = duration(val);
		else if (strcmp(key, "filename") == 0) {
			if (r->fields[i].file)
				replace(&image.filename, r->fields[i].file);

			image.data = ememdup(r->fields[i].val, r->fields[i].valsz);	
			image.datasz = r->fields[i].valsz;
		} else if (strcmp(key, "private") == 0)
			image.visible = strcmp(val, "on") != 0;
		else if (strcmp(key, "raw") == 0)
			raw = strcmp(val, "on") == 0;
	}

	/* TODO: image_isvalid should check for all stuff. */
	if (!image.data || !image_isvalid(image.data, image.datasz))
		page(r, NULL, KHTTP_400, "pages/400.html", "400");
	else if (!database_insert(&image))
		page(r, NULL, KHTTP_500, "pages/500.html", "500");
	else {
		if (raw) {
			/* For CLI users (e.g. imgup) just print the location. */
			khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_201]);
			khttp_body(r);
			khttp_printf(r, "%s://%s/image/%s\n",
			    r->scheme == KSCHEME_HTTP ? "http" : "https",
			    r->host, image.id);
			khttp_free(r);
		} else {
			/* Otherwise, redirect to image details. */
			khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_302]);
			khttp_head(r, kresps[KRESP_LOCATION], "/image/%s", image.id);
			khttp_body(r);
			khttp_free(r);
		}
	}

	image_finish(&image);
}

void
page_new(struct kreq *r)
{
	assert(r);

	switch (r->method) {
	case KMETHOD_GET:
		get(r);
		break;
	case KMETHOD_POST:
		post(r);
		break;
	default:
		break;
	}
}

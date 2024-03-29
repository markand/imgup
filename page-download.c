/*
 * page-download.c -- page /download/<id>
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
#include <stdint.h>

#include <kcgi.h>

#include "database.h"
#include "image.h"
#include "page.h"

static void
get(struct kreq *r)
{
	struct image image = {0};

	if (!database_get(&image, r->path))
		page(r, NULL, KHTTP_404, "pages/404.html", "404");
	else {
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_APP_OCTET_STREAM]);
		khttp_head(r, kresps[KRESP_CONTENT_LENGTH], "%llu", (unsigned long long)image.datasz);
		khttp_head(r, kresps[KRESP_CONNECTION], "keep-alive");
		khttp_head(r, kresps[KRESP_CONTENT_DISPOSITION],
		    "attachment; filename=\"%s\"", image.id);
		khttp_body(r);
		khttp_write(r, image.data, image.datasz);
		khttp_free(r);
		image_finish(&image);
	}
}

void
page_download(struct kreq *r)
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

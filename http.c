/*
 * http.c -- HTTP parsing and rendering
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
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <kcgi.h>
#include <kcgihtml.h>

#include "config.h"
#include "database.h"
#include "http.h"
#include "log.h"
#include "image.h"
#include "util.h"

#include "page-download.h"
#include "page-image.h"
#include "page-index.h"
#include "page-new.h"
#include "page-search.h"
#include "page-static.h"
#include "page.h"

enum page {
	PAGE_INDEX,
	PAGE_NEW,
	PAGE_IMAGE,
	PAGE_DOWNLOAD,
	PAGE_SEARCH,
	PAGE_STATIC,
	PAGE_NUM        /* Not used. */
};

static const char *pages[] = {
	[PAGE_INDEX]    = "",
	[PAGE_NEW]      = "new",
	[PAGE_IMAGE]    = "image",
	[PAGE_DOWNLOAD] = "download",
	[PAGE_SEARCH]   = "search",
	[PAGE_STATIC]   = "static"
};

static void (*handlers[])(struct kreq *req) = {
	[PAGE_INDEX]    = page_index,
	[PAGE_NEW]      = page_new,
	[PAGE_IMAGE]    = page_image,
	[PAGE_DOWNLOAD] = page_download,
	[PAGE_SEARCH]   = page_search,
	[PAGE_STATIC]   = page_static
};

static void
process(struct kreq *req)
{
	assert(req);

	log_debug("http: accessing page '%s'", req->path);

	if (req->page == PAGE_NUM)
		page(req, NULL, KHTTP_404, "pages/404.html");
	else
		handlers[req->page](req);
}

void
http_fcgi_run(void)
{
	struct kreq req;
	struct kfcgi *fcgi;

	if (khttp_fcgi_init(&fcgi, NULL, 0, pages, PAGE_NUM, 0) != KCGI_OK)
		return;

	while (khttp_fcgi_parse(fcgi, &req) == KCGI_OK)
		process(&req);

	khttp_fcgi_free(fcgi);
}

void
http_cgi_run(void)
{
	struct kreq req;

	if (khttp_parse(&req, NULL, 0, pages, PAGE_NUM, 0) == KCGI_OK)
		process(&req);
}

/*
 * util.c -- various utilities
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

#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <time.h>

#include "image.h"
#include "config.h"
#include "util.h"

noreturn void
die(const char *fmt, ...)
{
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(1);
}

char *
estrdup(const char *str)
{
	assert(str);

	char *ret;
	size_t length = strlen(str);

	if (!(ret = calloc(1, length + 1)))
		die(strerror(errno));

	return strcpy(ret, str);
}

void *
ememdup(const void *src, size_t length)
{
	assert(src);
	assert(length > 0);

	void *ptr;

	if (!(ptr = malloc(length)))
		die(strerror(errno));

	return memcpy(ptr, src, length);
}

const char *
bprintf(const char *fmt, ...)
{
	assert(fmt);

	static char buf[BUFSIZ];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	return buf;
}

const char *
bstrftime(const char *fmt, const struct tm *tm)
{
	assert(fmt);
	assert(tm);

	static char buf[BUFSIZ];

	strftime(buf, sizeof (buf), fmt, tm);

	return buf;
}

const char *
path(const char *filename)
{
	assert(filename);

	/* Build path to the template file. */
	static char path[PATH_MAX];

	snprintf(path, sizeof (path), "%s/%s", config.themedir, filename);

	return path;
}

void
replace(char **dst, const char *s)
{
	assert(dst);
	assert(s);

	/* Trim leading spaces. */
	while (*s && isspace(*s))
		s++;

	if (*s) {
		free(*dst);
		*dst = estrdup(s);
	}
}

const char *
ttl(time_t timestamp, long long int duration)
{
	const time_t now = time(NULL);
	const long long int left = duration - difftime(now, timestamp);

	if (left < IMAGE_DURATION_HOUR)
		return bprintf("%lld minute(s)", left / 60);
	if (left < IMAGE_DURATION_DAY)
		return bprintf("%lld hour(s)", left / 3600);

	/* Other in days. */
	return bprintf("%lld day(s)", left / 86400);
}

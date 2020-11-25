/*
 * util.h -- various utilities
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

#ifndef IMGPASTER_UTIL_H
#define IMGPASTER_UTIL_H

#include <stddef.h>
#include <stdnoreturn.h>
#include <time.h>

#define NELEM(x) (sizeof (x) / sizeof (x)[0])

struct tm;

noreturn void
die(const char *, ...);

char *
estrdup(const char *);

void *
ememdup(const void *, size_t);

const char *
bprintf(const char *, ...);

const char *
bstrftime(const char *, const struct tm *);

const char *
path(const char *);

void
replace(char **, const char *);

const char *
ttl(time_t, long long int);

#endif /* !IMGPASTER_UTIL_H */

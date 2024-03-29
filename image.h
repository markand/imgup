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

#ifndef IMGUP_IMAGE_H
#define IMGUP_IMAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#define IMAGE_DURATION_HOUR     3600            /*!< Seconds in one hour. */
#define IMAGE_DURATION_DAY      86400           /*!< Seconds in one day. */
#define IMAGE_DURATION_WEEK     604800          /*!< Seconds in one week. */
#define IMAGE_DURATION_MONTH    2592000         /*!< Rounded to 30 days. */

/**
 * \brief Paste structure.
 *
 * Every string in the paste is assumed to be allocated on the heap.
 */
struct image {
	char *id;
	char *title;
	char *author;
	void *data;
	size_t datasz;
	char *filename;
	time_t timestamp;
	bool visible;
	long long int duration;
};

void
image_finish(struct image *);

bool
image_isvalid(const void *, size_t);

#endif /* !IMGUP_IMAGE_H */

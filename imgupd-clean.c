/*
 * imgupd-clean.c -- main imgupd-clean(8) file
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "database.h"
#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: imgupd-clean [-d database-path]\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	const char *value;
	char path[PATH_MAX] = VARDIR "/imgup/imgup.db";
	int ch;

	/* Seek environment first. */
	if ((value = getenv("IMGUPD_DATABASE_PATH")))
		snprintf(path, sizeof (path), "%s", value);

	while ((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
		case 'd':
			snprintf(path, sizeof (path), "%s", optarg);
			break;
		default:
			usage();
			break;
		}
	}

	if (!path[0])
		die("abort: no database specified");
	if (!database_open(path))
		die("abort: could not open database");

	database_clear();
	database_finish();
}

/*
 * database.c -- sqlite storage
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sqlite3.h>

#include "database.h"
#include "image.h"
#include "log.h"
#include "util.h"

static sqlite3 *db;

static const char *sql_init =
	"BEGIN EXCLUSIVE TRANSACTION;\n"
	"\n"
	"CREATE TABLE IF NOT EXISTS image(\n"
	"  id TEXT PRIMARY KEY,\n"
	"  title TEXT,\n"
	"  author TEXT,\n"
	"  data BLOB,\n"
	"  filename TEXT,\n"
	"  date INT DEFAULT CURRENT_TIMESTAMP,\n"
	"  visible INTEGER DEFAULT 0,\n"
	"  duration INT\n"
	");\n"
	"\n"
	"END TRANSACTION";

static const char *sql_get =
	"SELECT id\n"
	"     , title\n"
	"     , author\n"
	"     , data\n"
	"     , LENGTH(data) AS length\n"
	"     , filename\n"
	"     , strftime('%s', date) AS date\n"
	"     , visible\n"
	"     , duration\n"
	"  FROM image\n"
	" WHERE id = ?";

static const char *sql_insert =
	"INSERT INTO image(\n"
	"  id,\n"
	"  title,\n"
	"  author,\n"
	"  data,\n"
	"  filename,\n"
	"  visible,\n"
	"  duration\n"
	") VALUES (?, ?, ?, ?, ?, ?, ?)";

static const char *sql_recents =
	"SELECT id\n"
	"     , title\n"
	"     , author\n"
	"     , data\n"
	"     , LENGTH(data) AS length\n"
	"     , filename\n"
	"     , strftime('%s', date) AS date\n"
	"     , visible\n"
	"     , duration\n"
	"  FROM image\n"
	" WHERE visible = 1\n"
	" ORDER BY date DESC\n"
	" LIMIT ?\n";

static const char *sql_clear =
	"BEGIN EXCLUSIVE TRANSACTION;\n"
	"\n"
	"DELETE\n"
	"  FROM image\n"
	" WHERE strftime('%s', 'now') - strftime('%s', date) >= duration;"
	"\n"
	"END TRANSACTION";

static const char *sql_search =
	"SELECT id\n"
	"     , title\n"
	"     , author\n"
	"     , data\n"
	"     , LENGTH(data)\n"
	"     , filename\n"
	"     , strftime('%s', date) AS date\n"
	"     , visible\n"
	"     , duration\n"
	"  FROM image\n"
	" WHERE title LIKE ?\n"
	"   AND author LIKE ?\n"
	"   AND visible = 1\n"
	" ORDER BY date DESC\n"
	" LIMIT ?\n";

/* sqlite3 use const unsigned char *. */
static char *
dup(const unsigned char *s)
{
	return estrdup(s ? (const char *)(s) : "");
}

static void
convert(sqlite3_stmt *stmt, struct image *image)
{
	image->id = dup(sqlite3_column_text(stmt, 0));
	image->title = dup(sqlite3_column_text(stmt, 1));
	image->author = dup(sqlite3_column_text(stmt, 2));
	image->datasz = sqlite3_column_int64(stmt, 4);
	image->data = ememdup(sqlite3_column_blob(stmt, 3), image->datasz);
	image->filename = dup(sqlite3_column_text(stmt, 5));
	image->timestamp = sqlite3_column_int64(stmt, 6);
	image->visible = sqlite3_column_int(stmt, 7);
	image->duration = sqlite3_column_int64(stmt, 8);
}

static bool
exists(const char *id)
{
	assert(id);

	sqlite3_stmt *stmt = NULL;
	bool ret = true;

	if (sqlite3_prepare(db, sql_get, -1, &stmt, NULL) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, id, -1, NULL);
		ret = sqlite3_step(stmt) == SQLITE_ROW;
		sqlite3_finalize(stmt);
	}

	return ret;
}

static const char *
create_id(void)
{
	static const char table[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	static char id[12];

	for (size_t i = 0; i < sizeof (id); ++i)
		id[i] = table[rand() % (sizeof (table) - 1)];

	return id;
}

static bool
set_id(struct image *image)
{
	assert(image);

	image->id = NULL;

	/*
	 * Avoid infinite loop, we only try to create a new id in 30 steps.
	 *
	 * On error, the function `exist` returns true to indicate we should
	 * not try to save with that id.
	 */
	int tries = 0;

	do {
		free(image->id);
		image->id = estrdup(create_id());
	} while (++tries < 30 && exists(image->id));

	return tries < 30;
}

bool
database_open(const char *path)
{
	assert(path);

	log_info("database: opening %s", path);

	if (sqlite3_open(path, &db) != SQLITE_OK) {
		log_warn("database: unable to open %s: %s", path, sqlite3_errmsg(db));
		return false;
	}

	/* Wait for 30 seconds to lock the database. */
	sqlite3_busy_timeout(db, 30000);

	if (sqlite3_exec(db, sql_init, NULL, NULL, NULL) != SQLITE_OK) {
		log_warn("database: unable to initialize %s: %s", path, sqlite3_errmsg(db));
		return false;
	}

	return true;
}

bool
database_recents(struct image *images, size_t *max)
{
	assert(images);
	assert(max);

	sqlite3_stmt *stmt = NULL;

	memset(images, 0, *max * sizeof (*images));
	log_debug("database: accessing most recents");

	if (sqlite3_prepare(db, sql_recents, -1, &stmt, NULL) != SQLITE_OK ||
	    sqlite3_bind_int64(stmt, 1, *max) != SQLITE_OK)
		goto sqlite_err;

	size_t i = 0;

	for (; i < *max && sqlite3_step(stmt) == SQLITE_ROW; ++i)
		convert(stmt, &images[i]);

	log_debug("database: found %zu images", i);
	sqlite3_finalize(stmt);
	*max = i;

	return true;

sqlite_err:
	log_warn("database: error (recents): %s\n", sqlite3_errmsg(db));

	if (stmt)
		sqlite3_finalize(stmt);

	return (*max = 0);
}

bool
database_get(struct image *image, const char *id)
{
	assert(image);
	assert(id);

	sqlite3_stmt* stmt = NULL;
	bool found = false;

	memset(image, 0, sizeof (*image));
	log_debug("database: accessing image with id: %s", id);

	if (sqlite3_prepare(db, sql_get, -1, &stmt, NULL) != SQLITE_OK ||
	    sqlite3_bind_text(stmt, 1, id, -1, NULL) != SQLITE_OK)
		goto sqlite_err;

	switch (sqlite3_step(stmt)) {
	case SQLITE_ROW:
		convert(stmt, image);
		found = true;
		break;
	case SQLITE_MISUSE:
	case SQLITE_ERROR:
		goto sqlite_err;
	default:
		break;
	}

	sqlite3_finalize(stmt);

	return found;

sqlite_err:
	if (stmt)
		sqlite3_finalize(stmt);

	log_warn("database: error (get): %s", sqlite3_errmsg(db));

	return false;
}

bool
database_insert(struct image *image)
{
	assert(image);

	sqlite3_stmt *stmt = NULL;

	log_debug("database: creating new image");

	if (sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, NULL) != SQLITE_OK) {
		log_warn("database: could not lock database: %s", sqlite3_errmsg(db));
		return false;
	}

	if (!set_id(image)) {
		log_warn("database: unable to randomize unique identifier");
		sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
		return false;
	}

	if (sqlite3_prepare(db, sql_insert, -1, &stmt, NULL) != SQLITE_OK)
		goto sqlite_err;

	sqlite3_bind_text(stmt, 1, image->id, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, image->title, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, image->author, -1, SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 4, image->data, image->datasz, NULL);
	sqlite3_bind_text(stmt, 5, image->filename, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, image->visible);
	sqlite3_bind_int64(stmt, 7, image->duration);

	if (sqlite3_step(stmt) != SQLITE_DONE)
		goto sqlite_err;

	sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
	sqlite3_finalize(stmt);

	log_info("database: new image (%s) from %s expires in one %lld seconds",
	    image->id, image->author, image->duration);

	return true;

sqlite_err:
	log_warn("database: error (insert): %s", sqlite3_errmsg(db));
	sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);

	if (stmt)
		sqlite3_finalize(stmt);

	free(image->id);
	image->id = NULL;

	return false;
}

bool
database_search(struct image *images,
                size_t *max,
                const char *title,
                const char *author)
{
	assert(images);
	assert(max);

	sqlite3_stmt *stmt = NULL;
	size_t i;

	memset(images, 0, *max * sizeof (*images));

	/* Select everything if not specified. */
	title    = title    ? title    : "%";
	author   = author   ? author   : "%";

	if (sqlite3_prepare(db, sql_search, -1, &stmt, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_text(stmt, 1, title, -1, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_text(stmt, 2, author, -1, NULL) != SQLITE_OK)
		goto sqlite_err;
	if (sqlite3_bind_int64(stmt, 3, *max) != SQLITE_OK)
		goto sqlite_err;

	for (i = 0; i < *max && sqlite3_step(stmt) == SQLITE_ROW; ++i)
		convert(stmt, &images[i]);

	log_debug("database: found %zu images", i);
	sqlite3_finalize(stmt);
	*max = i;

	return true;

sqlite_err:
	log_warn("database: error (search): %s\n", sqlite3_errmsg(db));

	if (stmt)
		sqlite3_finalize(stmt);

	return (*max = 0);
}

void
database_clear(void)
{
	log_debug("database: clearing deprecated images");

	if (sqlite3_exec(db, sql_clear, NULL, NULL, NULL) != SQLITE_OK)
		log_warn("database: error (clear): %s\n", sqlite3_errmsg(db));
}

void
database_finish(void)
{
	log_debug("database: closing");

	if (db) {
		sqlite3_close(db);
		db = NULL;
	}
}

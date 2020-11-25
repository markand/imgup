/*
 * test-database.c -- test database functions
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

#include <stdio.h>
#include <unistd.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include "database.h"
#include "image.h"
#include "util.h"

#define TEST_DATABASE "test.db"

static void
setup(void *data)
{
	remove(TEST_DATABASE);

	if (!database_open(TEST_DATABASE))
		die("abort: could not open database");

	(void)data;
}

static void
finish(void *data)
{
	database_finish();

	(void)data;
}

GREATEST_TEST
recents_empty(void)
{
	struct image images[10];
	size_t max = 10;

	if (!database_recents(images, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 0);
	GREATEST_PASS();
}

GREATEST_TEST
recents_one(void)
{
	struct image images[10];
	size_t max = 10;
	struct image one = {
		.title = estrdup("test 1"),
		.author = estrdup("unit test"),
		.data = estrdup("PNG..."),
		.datasz = 6,
		.filename = estrdup("image.png"),
		.duration = IMAGE_DURATION_HOUR,
		.visible = true
	};

	if (!database_insert(&one))
		GREATEST_FAIL();
	if (!database_recents(images, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 1);
	GREATEST_ASSERT(images[0].id);
	GREATEST_ASSERT_STR_EQ(images[0].title, "test 1");
	GREATEST_ASSERT_STR_EQ(images[0].author, "unit test");
	GREATEST_ASSERT_MEM_EQ(images[0].data, "PNG...", 6);
	GREATEST_ASSERT_EQ(images[0].datasz, 6);
	GREATEST_ASSERT_STR_EQ(images[0].filename, "image.png");
	GREATEST_ASSERT_EQ(images[0].duration, IMAGE_DURATION_HOUR);
	GREATEST_ASSERT(images[0].visible);
	GREATEST_PASS();
}

GREATEST_TEST
recents_hidden(void)
{
	struct image images[10];
	size_t max = 10;
	struct image one = {
		.title = estrdup("test 1"),
		.author = estrdup("unit test"),
		.data = estrdup("PNG..."),
		.datasz = 6,
		.filename = estrdup("image.png"),
		.duration = IMAGE_DURATION_HOUR,
		.visible = false
	};

	if (!database_insert(&one))
		GREATEST_FAIL();
	if (!database_recents(images, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 0);
	GREATEST_PASS();
}

GREATEST_TEST
recents_many(void)
{
	static const int expected[] = { 2, 1, 0 };
	struct image images[3];
	size_t max = 3;
	struct image image = {
		.duration = IMAGE_DURATION_HOUR,
		.visible = true
	};

	for (int i = 0; i < 3; ++i) {
		image.title = estrdup(bprintf("test %d", i));
		image.author = estrdup(bprintf("unit test %d", i));
		image.data = estrdup(bprintf("PNG... %d", i));
		image.datasz = 8;
		image.filename = estrdup(bprintf("%d.png", i));

		if (!database_insert(&image))
			GREATEST_FAIL();

		/* Sleep a little bit to avoid same timestamp. */
		sleep(2);
	};

	if (!database_recents(images, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 3U);

	for (int i = 0; i < 3; ++i) {
		/* Selected in most recents first. */
		GREATEST_ASSERT(images[i].id);
		GREATEST_ASSERT_STR_EQ(images[i].title,
		    bprintf("test %d", expected[i]));
		GREATEST_ASSERT_STR_EQ(images[i].author,
		    bprintf("unit test %d", expected[i]));
		GREATEST_ASSERT_MEM_EQ(images[i].data,
		    bprintf("PNG... %d", expected[i]), 6);
		GREATEST_ASSERT_EQ(images[i].datasz, 8);
		GREATEST_ASSERT_STR_EQ(images[i].filename,
		    bprintf("%i.png", expected[i]));
		GREATEST_ASSERT_EQ(images[i].duration, IMAGE_DURATION_HOUR);
		GREATEST_ASSERT(images[i].visible);
	};

	GREATEST_PASS();
}

GREATEST_TEST
recents_limits(void)
{
	static const int expected[] = { 19, 18, 17 };
	struct image images[3];
	size_t max = 3;
	struct image image = {
		.duration = IMAGE_DURATION_HOUR,
		.visible = true
	};

	for (int i = 0; i < 20; ++i) {
		image.title = estrdup(bprintf("test %d", i));
		image.author = estrdup(bprintf("unit test %d", i));
		image.data = estrdup(bprintf("PNG... %d", i));
		image.datasz = 8;
		image.filename = estrdup(bprintf("%d.png", i));

		if (!database_insert(&image))
			GREATEST_FAIL();

		/* Sleep a little bit to avoid same timestamp. */
		sleep(2);
	};

	if (!database_recents(images, &max))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 3U);

	for (int i = 0; i < 3; ++i) {
		/* Selected in most recents first. */
		GREATEST_ASSERT(images[i].id);
		GREATEST_ASSERT_STR_EQ(images[i].title,
		    bprintf("test %d", expected[i]));
		GREATEST_ASSERT_STR_EQ(images[i].author,
		    bprintf("unit test %d", expected[i]));
		GREATEST_ASSERT_MEM_EQ(images[i].data,
		    bprintf("PNG... %d", expected[i]), 6);
		GREATEST_ASSERT_EQ(images[i].datasz, 8);
		GREATEST_ASSERT_STR_EQ(images[i].filename,
		    bprintf("%i.png", expected[i]));
		GREATEST_ASSERT_EQ(images[i].duration, IMAGE_DURATION_HOUR);
		GREATEST_ASSERT(images[i].visible);
	};

	GREATEST_PASS();
}

GREATEST_SUITE(recents)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(finish, NULL);
	GREATEST_RUN_TEST(recents_empty);
	GREATEST_RUN_TEST(recents_one);
	GREATEST_RUN_TEST(recents_hidden);
	GREATEST_RUN_TEST(recents_many);
	GREATEST_RUN_TEST(recents_limits);
}

GREATEST_TEST
get_basic(void)
{
	struct image original = {
		.title = estrdup("test 1"),
		.author = estrdup("unit test"),
		.data = estrdup("PNG..."),
		.datasz = 6,
		.filename = estrdup("image.png"),
		.duration = IMAGE_DURATION_HOUR,
		.visible = false
	};
	struct image new = {0};

	if (!database_insert(&original))
		GREATEST_FAIL();
	if (!database_get(&new, original.id))
		GREATEST_FAIL();

	GREATEST_ASSERT_STR_EQ(new.id, original.id);
	GREATEST_ASSERT_STR_EQ(new.title, original.title);
	GREATEST_ASSERT_STR_EQ(new.author, original.author);
	GREATEST_ASSERT_MEM_EQ(new.data, original.data, 6);
	GREATEST_ASSERT_EQ(new.datasz, original.datasz);
	GREATEST_ASSERT_STR_EQ(new.filename, original.filename);
	GREATEST_ASSERT_EQ(new.visible, original.visible);
	GREATEST_PASS();
}

GREATEST_TEST
get_nonexistent(void)
{
	struct image new = {0};

	GREATEST_ASSERT(!database_get(&new, "unknown"));
	GREATEST_ASSERT(!new.id);
	GREATEST_ASSERT(!new.title);
	GREATEST_ASSERT(!new.author);
	GREATEST_ASSERT(!new.data);
	GREATEST_ASSERT(!new.datasz);
	GREATEST_ASSERT(!new.filename);
	GREATEST_ASSERT(!new.timestamp);
	GREATEST_ASSERT(!new.visible);
	GREATEST_PASS();
}

GREATEST_SUITE(get)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(finish, NULL);
	GREATEST_RUN_TEST(get_basic);
	GREATEST_RUN_TEST(get_nonexistent);
}

GREATEST_TEST
search_basic(void)
{
	struct image searched[3] = { 0 };
	struct image originals[] = {
		{
			.title = estrdup("Super Mario"),
			.author = estrdup("Mario"),
			.data = estrdup("PNG mario"),
			.datasz = 9,
			.filename = "mario.png",
			.duration = IMAGE_DURATION_HOUR,
			.visible = true
		},
		{
			.title = estrdup("Super Luigi"),
			.author = estrdup("Luigi"),
			.data = estrdup("PNG luigi"),
			.datasz = 9,
			.filename = "luigi.png",
			.duration = IMAGE_DURATION_HOUR,
			.visible = true
		},
	};
	size_t max = 3;

	for (int i = 0; i < 3; ++i)
		if (!database_insert(&originals[i]))
			GREATEST_FAIL();

	/*
	 * Search:
	 *
	 * title = <any>
	 * author = Mario,
	 */
	if (!database_search(searched, &max, NULL, "Mario"))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 1);
	GREATEST_ASSERT(searched[0].id);
	GREATEST_ASSERT_STR_EQ(searched[0].title, "Super Mario");
	GREATEST_ASSERT_STR_EQ(searched[0].author, "Mario");
	GREATEST_ASSERT_MEM_EQ(searched[0].data, "PNG mario", 9);
	GREATEST_ASSERT_EQ(searched[0].datasz, 9);
	GREATEST_ASSERT_STR_EQ(searched[0].filename, "mario.png");
	GREATEST_ASSERT_EQ(searched[0].duration, IMAGE_DURATION_HOUR);
	GREATEST_ASSERT(searched[0].visible);
	GREATEST_PASS();
}

GREATEST_TEST
search_notfound(void)
{
	struct image searched = {0};
	struct image original = {
		.title = estrdup("Super Mario"),
		.author = estrdup("Mario"),
		.data = estrdup("PNG mario"),
		.datasz = 9,
		.filename = "mario.png",
		.duration = IMAGE_DURATION_HOUR,
		.visible = true
	};
	size_t max = 1;

	if (!database_insert(&original))
		GREATEST_FAIL();

	/*
	 * Search:
	 *
	 * title = <any>
	 * author = jean,
	 */
	if (!database_search(&searched, &max, NULL, "jean"))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 0);
	GREATEST_ASSERT(!searched.id);
	GREATEST_ASSERT(!searched.title);
	GREATEST_ASSERT(!searched.author);
	GREATEST_ASSERT(!searched.data);
	GREATEST_ASSERT(!searched.datasz);
	GREATEST_ASSERT(!searched.filename);
	GREATEST_ASSERT(!searched.timestamp);
	GREATEST_ASSERT(!searched.visible);
	GREATEST_PASS();
}

GREATEST_TEST
search_private(void)
{
	struct image searched = {0};
	struct image original = {
		.title = estrdup("Super Mario"),
		.author = estrdup("Mario"),
		.data = estrdup("PNG mario"),
		.datasz = 9,
		.filename = "mario.png",
		.duration = IMAGE_DURATION_HOUR,
		.visible = false
	};
	size_t max = 1;

	if (!database_insert(&original))
		GREATEST_FAIL();

	/*
	 * Search:
	 *
	 * title = <any>
	 * author = <any>
	 */
	if (!database_search(&searched, &max, NULL, NULL))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 0);
	GREATEST_ASSERT(!searched.id);
	GREATEST_ASSERT(!searched.title);
	GREATEST_ASSERT(!searched.author);
	GREATEST_ASSERT(!searched.data);
	GREATEST_ASSERT(!searched.datasz);
	GREATEST_ASSERT(!searched.filename);
	GREATEST_ASSERT(!searched.timestamp);
	GREATEST_ASSERT(!searched.visible);
	GREATEST_PASS();
}

GREATEST_SUITE(search)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(finish, NULL);
	GREATEST_RUN_TEST(search_basic);
	GREATEST_RUN_TEST(search_notfound);
	GREATEST_RUN_TEST(search_private);
}

GREATEST_TEST
clear_run(void)
{
	struct image searched = { 0 };
	struct image originals[] = {
		/* Will be deleted */
		{
			.title = estrdup("Super Mario"),
			.author = estrdup("Mario"),
			.data = estrdup("PNG mario"),
			.datasz = 9,
			.filename = "mario.png",
			.duration = IMAGE_DURATION_HOUR,
			.visible = true,
			.duration = 1
		},
		/* Will be deleted */
		{
			.title = estrdup("Super Luigi"),
			.author = estrdup("Luigi"),
			.data = estrdup("PNG luigi"),
			.datasz = 9,
			.filename = "mario.png",
			.duration = IMAGE_DURATION_HOUR,
			.visible = true,
			.duration = 1
		},
		/* Will be kept */
		{
			.title = estrdup("Bowser"),
			.author = estrdup("Bowser"),
			.data = estrdup("PNG bowser"),
			.datasz = 10,
			.filename = "bowser.png",
			.duration = IMAGE_DURATION_HOUR,
			.visible = true
		},
	};
	size_t max = 1;

	for (int i = 0; i < 3; ++i)
		if (!database_insert(&originals[i]))
			GREATEST_FAIL();

	/* Sleep 2 seconds to exceed the lifetime of Mario and Luigi images. */
	sleep(2);
	database_clear();

	/*
	 * Search:
	 *
	 * title = <any>
	 * author = <any>
	 */
	if (!database_search(&searched, &max, NULL, NULL))
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(max, 1);
	GREATEST_ASSERT(searched.id);
	GREATEST_ASSERT_STR_EQ(searched.title, "Bowser");
	GREATEST_ASSERT_STR_EQ(searched.author, "Bowser");
	GREATEST_ASSERT_MEM_EQ(searched.data, "PNG bowser", 10);
	GREATEST_ASSERT_EQ(searched.datasz, 10);
	GREATEST_ASSERT_STR_EQ(searched.filename, "bowser.png");
	GREATEST_ASSERT_EQ(searched.duration, IMAGE_DURATION_HOUR);
	GREATEST_ASSERT(searched.visible);
	GREATEST_PASS();
}

GREATEST_SUITE(clear)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(finish, NULL);
	GREATEST_RUN_TEST(clear_run);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(recents);
	GREATEST_RUN_SUITE(get);
	GREATEST_RUN_SUITE(search);
	GREATEST_RUN_SUITE(clear);
	GREATEST_MAIN_END();
}

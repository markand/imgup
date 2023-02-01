#
# Makefile -- basic makefile for imgup
#
# Copyright (c) 2020-2023 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

.POSIX:

# User options.
CC=             cc
CFLAGS=         -DNDEBUG -O3

# Installation paths.
PREFIX=         /usr/local
BINDIR=         ${PREFIX}/bin
SHAREDIR=       ${PREFIX}/share
MANDIR=         ${PREFIX}/share/man
VARDIR=         ${PREFIX}/var

VERSION=        0.2.0

CORE_SRCS=      config.c                        \
                database.c                      \
                fragment-duration.c             \
                fragment-image.c                \
                fragment.c                      \
                http.c                          \
                image.c                         \
                log.c                           \
                page-download.c                 \
                page-image.c                    \
                page-index.c                    \
                page-new.c                      \
                page-search.c                   \
                page-static.c                   \
                page.c                          \
                util.c
CORE_HDRS=      config.h                        \
                database.h                      \
                fragment-duration.h             \
                fragment-image.h                \
                fragment.h                      \
                http.h                          \
                image.h                         \
                log.h                           \
                page-download.h                 \
                page-image.h                    \
                page-index.h                    \
                page-new.h                      \
                page-search.h                   \
                page-static.h                   \
                page.h                          \
                util.h
CORE_OBJS=      ${CORE_SRCS:.c=.o}
CORE_DEPS=      ${CORE_SRCS:.c=.d}
CORE_LIB=       libimgup.a

TESTS_SRCS=     tests/test-database.c
TESTS_OBJS=     ${TESTS_SRCS:.c=}

SQLITE_FLAGS=   -DSQLITE_THREADSAFE=0           \
                -DSQLITE_OMIT_LOAD_EXTENSION    \
                -DSQLITE_OMIT_DEPRECATED        \
                -DSQLITE_DEFAULT_FOREIGN_KEYS=1
SQLITE_LIB=     libsqlite3.a

MY_CFLAGS=      -std=c11                        \
                -MMD                            \
                -I .                            \
                -I extern                       \
                -D_XOPEN_SOURCE=700             \
                -DSHAREDIR=\"${SHAREDIR}\"      \
                -DVARDIR=\"${VARDIR}\"          \
                `pkg-config --cflags libmagic kcgi-html`

MY_LDFLAGS=     `pkg-config --libs libmagic kcgi-html`

.SUFFIXES:
.SUFFIXES: .o .c .in

all: imgupd imgupd-clean imgup

-include ${CORE_DEPS} imgup.d imgupd-clean.d

.c.o:
	${CC} ${MY_CFLAGS} ${CFLAGS} -c $<

.c:
	${CC} ${MY_CFLAGS} $< -o $@ ${CORE_LIB} ${SQLITE_LIB} ${MY_LDFLAGS} ${LDFLAGS}

.o:
	${CC} -o $@ $< ${CORE_LIB} ${SQLITE_LIB} ${MY_LDFLAGS} ${LDFLAGS}

.in:
	sed -e "s|@SHAREDIR@|${SHAREDIR}|" \
	    -e "s|@VARDIR@|${VARDIR}|" \
	    < $< > $@

${SQLITE_LIB}: extern/sqlite3.c extern/sqlite3.h
	${CC} ${CFLAGS} ${SQLITE_FLAGS} -c extern/sqlite3.c -o extern/sqlite3.o
	${AR} -rc $@ extern/sqlite3.o

${CORE_LIB}: ${CORE_OBJS}
	${AR} -rc $@ ${CORE_OBJS}

imgupd-clean.o: imgupd-clean.8 ${CORE_LIB} ${SQLITE_LIB}

imgupd.o: imgupd-themes.5 imgupd.8 ${CORE_LIB} ${SQLITE_LIB}

imgup: imgup.sh imgup.1
	cp imgup.sh imgup
	chmod +x imgup

clean:
	rm -f ${SQLITE_LIB} extern/sqlite3.o
	rm -f ${CORE_LIB} ${CORE_OBJS} ${CORE_DEPS}
	rm -f imgupd imgupd.d imgupd.o imgupd-themes.5 imgupd.8
	rm -f imgupd-clean imgupd-clean.d imgupd-clean.o imgupd-clean.8
	rm -f imgup imgup.1
	rm -f test.db ${TESTS_OBJS}

install-imgup:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	cp imgup ${DESTDIR}${BINDIR}
	cp imgup.1 ${DESTDIR}${MANDIR}/man1/imgup.1

install-imgupd:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man5
	mkdir -p ${DESTDIR}${MANDIR}/man8
	cp imgupd ${DESTDIR}${BINDIR}
	cp imgupd-clean ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${SHAREDIR}/imgup
	cp -R themes ${DESTDIR}${SHAREDIR}/imgup
	cp imgupd-themes.5 ${DESTDIR}${MANDIR}/man5
	cp imgupd.8 ${DESTDIR}${MANDIR}/man8
	cp imgupd-clean.8 ${DESTDIR}${MANDIR}/man8

install: install-imgupd install-imgup

dist:
	mkdir -p imgup-${VERSION}
	cp -R extern imgup-${VERSION}
	cp -R themes imgup-${VERSION}
	cp -R tests imgup-${VERSION}
	cp ${CORE_SRCS} ${CORE_HDRS} imgup-${VERSION}
	cp imgupd.8.in imgupd.c imgup-${VERSION}
	cp imgupd-clean.8.in imgupd-clean.c imgup-${VERSION}
	cp imgup.1.in imgup.sh imgup-${VERSION}
	cp Makefile CHANGES.md CONTRIBUTE.md CREDITS.md INSTALL.md LICENSE.md \
	    README.md STYLE.md TODO.md imgup-${VERSION}
	tar -cJf imgup-${VERSION}.tar.xz imgup-${VERSION}
	rm -rf imgup-${VERSION}

${TESTS_OBJS}: ${CORE_LIB} ${SQLITE_LIB}

tests: ${TESTS_OBJS}
	for t in ${TESTS_OBJS}; do $$t; done

.PHONY: all clean dist run tests

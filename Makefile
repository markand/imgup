#
# Makefile -- basic makefile for imgpaster
#
# Copyright (c) 2020 David Demelier <markand@malikania.fr>
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

VERSION=        0.1.0

CORE_SRCS=      config.c                \
                database.c              \
                fragment-duration.c     \
                fragment-image.c        \
                fragment.c              \
                http.c                  \
                image.c                 \
                log.c                   \
                page-download.c         \
                page-image.c            \
                page-index.c            \
                page-new.c              \
                page-search.c           \
                page-static.c           \
                page.c                  \
                util.c
CORE_HDRS=      config.h                \
                database.h              \
                fragment-duration.h     \
                fragment-image.h        \
                fragment.h              \
                http.h                  \
                image.h                 \
                log.h                   \
                page-download.h         \
                page-image.h            \
                page-index.h            \
                page-new.h              \
                page-search.h           \
                page-static.h           \
                page.h                  \
                util.h
CORE_OBJS=      ${CORE_SRCS:.c=.o}
CORE_DEPS=      ${CORE_SRCS:.c=.d}
CORE_LIB=       libimgpaster.a

TESTS_SRCS=     tests/test-database.c
TESTS_OBJS=     ${TESTS_SRCS:.c=}

SQLITE_FLAGS=   -DSQLITE_THREADSAFE=0 \
                -DSQLITE_OMIT_LOAD_EXTENSION \
                -DSQLITE_OMIT_DEPRECATED \
                -DSQLITE_DEFAULT_FOREIGN_KEYS=1
SQLITE_LIB=     libsqlite3.a

MY_CFLAGS=      -std=c11 -MMD -I. -Iextern \
                -D_XOPEN_SOURCE=700 \
                -DSHAREDIR=\"${SHAREDIR}\" \
                -DVARDIR=\"${VARDIR}\"
MY_LDFLAGS=     -lmagic -lkcgi -lkcgihtml -lz

.SUFFIXES:
.SUFFIXES: .o .c .in

all: imgpasterd imgpasterd-clean imgpaster

-include ${CORE_DEPS} imgpaster.d imgpasterd-clean.d

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

imgpasterd-clean.o: imgpasterd-clean.8 ${CORE_LIB} ${SQLITE_LIB}

imgpasterd.o: imgpasterd.8 ${CORE_LIB} ${SQLITE_LIB}

imgpaster: imgpaster.sh imgpaster.1
	cp imgpaster.sh imgpaster
	chmod +x imgpaster

clean:
	rm -f ${SQLITE_LIB} extern/sqlite3.o
	rm -f ${CORE_LIB} ${CORE_OBJS} ${CORE_DEPS}
	rm -f imgpasterd pasterd.d imgpasterd.o pasterd.8
	rm -f imgpasterd-clean imgpasterd-clean.d imgpasterd-clean.o imgpasterd-clean.8
	rm -f imgpaster imgpaster.1
	rm -f test.db ${TESTS_OBJS}

install-imgpaster:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	cp imgpaster ${DESTDIR}${BINDIR}
	cp imgpaster.1 ${DESTDIR}${MANDIR}/man1/imgpaster.1

install-imgpasterd:
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man8
	cp imgpasterd ${DESTDIR}${BINDIR}
	cp imgpasterd-clean ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${SHAREDIR}/imgpaster
	cp -R themes ${DESTDIR}${SHAREDIR}/imgpaster
	cp imgpasterd.8 ${DESTDIR}${MANDIR}/man8
	cp imgpasterd-clean.8 ${DESTDIR}${MANDIR}/man8

install: install-imgpasterd install-imgpaster

dist: clean
	mkdir -p imgpaster-${VERSION}
	cp -R extern imgpaster-${VERSION}
	cp -R themes imgpaster-${VERSION}
	cp -R tests imgpaster-${VERSION}
	cp ${CORE_SRCS} ${CORE_HDRS} imgpaster-${VERSION}
	cp imgpasterd.8.in imgpasterd.c imgpaster-${VERSION}
	cp imgpasterd-clean.8.in imgpasterd-clean.c imgpaster-${VERSION}
	cp imgpaster.1.in imgpaster.sh imgpaster-${VERSION}
	cp Makefile CHANGES.md CONTRIBUTE.md CREDITS.md INSTALL.md LICENSE.md \
	    README.md STYLE.md TODO.md imgpaster-${VERSION}
	tar -cJf imgpaster-${VERSION}.tar.xz imgpaster-${VERSION}
	rm -rf imgpaster-${VERSION}

${TESTS_OBJS}: ${CORE_LIB} ${SQLITE_LIB}

tests: ${TESTS_OBJS}
	for t in ${TESTS_OBJS}; do $$t; done

.PHONY: all clean dist run tests

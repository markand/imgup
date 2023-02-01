#
# imgup -- convenient front-end to a imgupd instance
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

author="Anonymous"
title="Untitled"
duration="day"
public=0
verbose=0

die()
{
	echo "$@" 1>&2
	exit 1
}

durations()
{
	echo "hour"
	echo "day"
	echo "week"
	echo "month"
	exit 0
}

usage()
{
	cat 1>&2 <<-EOF
	usage: imgup [-Dpv] [-a author] [-d duration] [-t title] filename host
	EOF
	exit 1
}

send()
{
	if [ $public -eq 1 ]; then
		with_private="-F private=off"
	else
		with_private="-F private=on"
	fi

	if [ $verbose -eq 0 ]; then
		with_verbose="-s"
	else
		with_verbose="-i"
	fi

	curl -L -X POST \
		-F author="$author" \
		-F duration="$duration" \
		-F title="$title" \
		-F filename=@"$1" \
		-F raw="on" \
		$with_private \
		$with_verbose \
		"$2"/new
}

if ! command -v curl >/dev/null 2>&1; then
	die "abort: curl is required"
fi

while getopts "Da:d:pt:v" opt; do
	case "$opt" in
	D)
		durations
		;;
	a)
		author="$OPTARG"
		;;
	d)
		duration="$OPTARG"
		;;
	p)
		public=1
		;;
	t)
		title="$OPTARG"
		;;
	v)
		verbose=1
		;;
	*)
		usage
		;;
	esac
done

shift $((OPTIND - 1))

if [ "$#" -ne 2 ]; then
	usage
fi

send "$1" "$2"

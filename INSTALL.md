imgpaster INSTALL
=================

Installation instructions.

Requirements
------------

- [kcgi][], minimal CGI/FastCGI library for C,
- [sqlite][], most used database in the world,
- [curl][], (Optional) only for `imgpaster(8)` client.

Basic installation
------------------

Quick install.

	$ tar xvzf imgpaster-x.y.z-tar.xz
	$ cd imgpaster-x.y.z
	$ make
	# sudo make install

To only install the web application:

	$ make imgpasterd
	# make install-imgpasterd

To only install the client:

	$ make imgpaster
	# make install-imgpaster

[curl]: https://curl.haxx.se
[kcgi]: https://kristaps.bsd.lv/kcgi
[sqlite]: https://www.sqlite.org

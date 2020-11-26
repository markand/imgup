imgup INSTALL
=============

Installation instructions.

Requirements
------------

- [kcgi][], minimal CGI/FastCGI library for C,
- [sqlite][], most used database in the world,
- [curl][], (Optional) only for `imgup(8)` client.

Basic installation
------------------

Quick install.

	$ tar xvzf imgup-x.y.z-tar.xz
	$ cd imgup-x.y.z
	$ make
	# sudo make install

To only install the web application:

	$ make imgupd
	# make install-imgupd

To only install the client:

	$ make imgup
	# make install-imgup

[curl]: https://curl.haxx.se
[kcgi]: https://kristaps.bsd.lv/kcgi
[sqlite]: https://www.sqlite.org

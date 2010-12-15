#!/bin/sh

gtkdocize || exit 1
aclocal -I m4 || exit 1
autoconf || exit 1
autoheader || exit 1
libtoolize --force || glibtoolize --force || exit 1
automake -a || exit 1
./configure --enable-maintainer-mode $* || exit 1

exit 0

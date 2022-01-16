#!/bin/sh
#-
# Copyright (C) 2011 Mikolaj Izdebski
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

set -e
IFS=' ''	''
'

if ! test -r build-aux/autogen.sh || ! test -r src/main.c; then
  echo autogen.sh: need to be called from top source directory >&2
  exit 1
fi

# Option -r removes autogenerated files.
if test x"$1" = x-r; then
  rm -Rf lib m4 build-aux/snippet autom4te.cache

  for f in config.guess config.sub depcomp install-sh missing \
      gitlog-to-changelog compile test-driver
    do rm -f build-aux/$f; done

  rm -f configure aclocal.m4 INSTALL
  for dir in . src man tests; do rm -f $dir/Makefile.in; done
  rm -f src/crctab.c src/scantab.h

  exit
fi

set -x

perl ./build-aux/make-crctab.pl
perl ./build-aux/make-scantab.pl

gnulib-tool --avoid=xalloc-die --add-import pthread utimens warnings \
    timespec-add timespec-sub dtotimespec stat-time lstat malloc-gnu \
    fprintf-posix inttypes xalloc largefile gitlog-to-changelog
# flockfile?

aclocal -Im4
autoconf
autoheader
automake --add-missing --copy

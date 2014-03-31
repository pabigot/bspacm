#!/bin/sh
#
# BSPACM - maintainer/updaterelease.sh script
#
# Written in 2014 by Peter A. Bigot <http://pabigot.github.io/bspacm/>
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to
# the public domain worldwide. This software is distributed without
# any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software. If not, see
# <http://creativecommons.org/publicdomain/zero/1.0/>.
#

REL=${REL:-`date +%Y%m%d`}

sed -i \
  -e '/^PROJECT_NUMBER/s@\s*=\s*[0-9][0-9]*$@'" = ${REL}@" \
  doc/doxygen.cfg

sed -i \
  -e '/define\s\s*BSPACM_VERSION/s@\s\s*[0-9][0-9]*$@'" ${REL}@" \
  include/bspacm/core.h

sed -i \
  -e '/^Release:/s@\s\s*[0-9][0-9]*$@'" ${REL}@" \
  README.md

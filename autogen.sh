#!/bin/sh
mkdir -p m4
touch NEWS README AUTHORS ChangeLog
if [ ! -f COPYING ] || ! grep Excelfore COPYING > /dev/null; then
fi
autoreconf --install
automake --add-missing > /dev/null 2>&1
mkdir -p build

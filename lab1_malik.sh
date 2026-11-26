#!/bin/sh
set -e

mkdir -p a/b a/c a/e
ln -s c a/d
printf "catts" > a/b/cat.txt
touch a/c/dog.txt
ln a/b/cat.txt a/e/f.txt
head -c 500 /dev/urandom > a/e/g.bin
truncate -s 35 a/e/h.bin

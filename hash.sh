#!/bin/sh

for I in `cat /usr/share/dict/words`
do
  ./hash $I >> hash.log
done

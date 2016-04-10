#!/bin/sh

TEST=$(basename $0 .sh)-$$

DB=${TEST}-db
TMP=/tmp/$TEST
LOG=$TEST.log
V=${VALGRIND}			# mettre VALGRIND à "valgrind -q" pour activer

HIDX=0				# index de la clef de h par défaut

./complement
rm test-667*
rm blinde*
exit 0

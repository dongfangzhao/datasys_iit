#!/bin/sh

if [ $# -lt 1 ]
then
	echo Usage: $0 proc_idx
	exit 1
fi

# echo -n 'Proc '$1' starts at' >> iozone.log
# date >> iozone.log

iozone -apew -f testfile$1 -s 1g -r 4k -i 2

echo -n 'Proc '$1' : ' >> iozone.log
date >> iozone.log

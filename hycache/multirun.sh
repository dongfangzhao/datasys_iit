#!/bin/sh

for i in {1..6}
do
	./singlerun.sh $i &
done

fusionfs : fusionfs.o util.o log.o
	gcc -g `pkg-config fuse --libs` -o fusionfs fusionfs.o util.o log.o -L./ffsnet -lffsnet_bridger -L./zht/lib -lzht -lstdc++ -lprotobuf -lprotobuf-c -lpthread 

fusionfs.o : fusionfs.c log.h params.h util.h
	gcc -g -Wall `pkg-config fuse --cflags` -c fusionfs.c -L./ffsnet -lffsnet_bridger 
	
util.o : util.c log.h params.h
	gcc -g -Wall `pkg-config fuse --cflags` -c util.c -L./zht/lib -lzht -lstdc++ -lprotobuf -lprotobuf-c -lpthread	

log.o : log.c log.h params.h
	gcc -g -Wall `pkg-config fuse --cflags` -c log.c

clean:
	rm -f fusionfs *.o 

dist:
	rm -rf fuse-tutorial/
	mkdir fuse-tutorial/
	cp ../*.html fuse-tutorial/
	mkdir fuse-tutorial/example/
	mkdir fuse-tutorial/example/mountdir/
	mkdir fuse-tutorial/example/rootdir/
	echo "a bogus file" > fuse-tutorial/example/rootdir/bogus.txt
	mkdir fuse-tutorial/src
	cp Makefile *.c *.h fuse-tutorial/src/
	tar cvzf ../../fuse-tutorial.tgz fuse-tutorial/
	rm -rf fuse-tutorial/

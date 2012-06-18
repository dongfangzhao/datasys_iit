DEBUG = 

schfs : schfs.o util.o log.o
	gcc -g `pkg-config fuse --libs` -o schfs schfs.o util.o log.o

schfs.o : schfs.c log.h params.h
	gcc $(DEBUG) -g -Wall `pkg-config fuse --cflags` -c schfs.c

util.o : util.c log.h params.h
	gcc $(DEBUG) -g -Wall `pkg-config fuse --cflags` -c util.c
	
log.o : log.c log.h params.h
	gcc $(DEBUG) -g -Wall `pkg-config fuse --cflags` -c log.c

clean:
	rm -f schfs *.o
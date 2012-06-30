/**
 * file: 	pbc_test.c
 * desc: 	to test if the C and C++ versions of Google Proto Buffers are compatible
 * author: 	dzhao8@hawk.iit.edu
 * date:	06/30/2012
 *
 * To Compile:
gcc -L/usr/local/lib -lprotobuf-c -I/usr/local/include/google/protobuf-c metac.pb-c.c pbc_test.c -o pbc_test
 */

#include "metac.pb-c.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
	size_t size = 0, size2 = 0;

	Package pkg = PACKAGE__INIT;
	Package *pkg_rtn;
	
	/* test data */
	pkg.has_replicano = 1; /* This is in need if this int32 is optional */
	pkg.replicano = 5;
	pkg.has_operation = 1;
	pkg.operation = 3;
	pkg.virtualpath = "mykey";
	pkg.realfullpath = "mypathname";

	/* pack the data */
	size = package__get_packed_size(&pkg);
	unsigned char *packed = malloc(size);
	size2 = package__pack(&pkg, packed);

	/* write packed data to a file, which will be read by a C++ program for compatibility test */
	FILE *pfile = fopen("serialize.file","wb");
	if (!pfile) {
		perror("Failed to open serialize.file.");
		return 1;
	}
	fwrite(packed, size, 1, pfile);
	fclose(pfile);

	/* read packed from a file written by the C++ program */
	FILE *readfile = fopen("cpp.file","rb");
	if (!readfile) {
		perror("Unable to open cpp.file.");
		return 1;
	}
	unsigned char *readbuf = malloc(size);
	fread(readbuf, size, 1, readfile);

	/* unpack the data */
	pkg_rtn = package__unpack(NULL, size, readbuf);
	
	/* verify the matchings */
	printf("dfz debug: pkg_rtn->replicano = %d \n", pkg_rtn->replicano); 
	printf("dfz debug: pkg_rtn->operation = %d \n", pkg_rtn->operation);
	printf("dfz debug: pkg_rtn->virtualpath = %s \n", pkg_rtn->virtualpath);
	printf("dfz debug: pkg_rtn->realfullpath = %s \n", pkg_rtn->realfullpath);
	
	package__free_unpacked(pkg_rtn, NULL);
	free(packed);
	
	return 0;
}

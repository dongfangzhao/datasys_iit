#include   <stdbool.h>
//#include   <stdlib.h>
#include   <stdio.h>
#include   <string.h>

#include "c_zhtclient.h"

int main(int argc, char **argv) {

	if (argc < 4) {

		printf("Usage: %s", "###.exe neighbor zht.cfg TCP\n");

		return -1;
	}

	bool useTCP = false;
	char *tcpFlag = argv[3];

	if (!strcmp("TCP", tcpFlag)) {
		useTCP = true;
	} else {
		useTCP = false;
	}

	c_zht_init(argv[1], argv[2], useTCP); //neighbor zht.cfg TCP

	const char *key = "/";
	const char *value = " ";
	const char *value2 = " dir/ ";

	int iret = c_zht_insert2(key, value);
	fprintf(stderr, "c_zht_insert, return code: %d\n", iret);

//	char *result = NULL;
	char result[1024];
	int lret = c_zht_lookup2(key, &result);
	fprintf(stderr, "c_zht_lookup, return code: %d\n", lret);
	fprintf(stderr, "c_zht_lookup, return value: %s.\n", result);

	int rret = c_zht_remove2(key);
	fprintf(stderr, "c_zht_remove, return code: %d\n", rret);

	iret = c_zht_insert2(key, value2);
	fprintf(stderr, "c_zht_insert, return code: %d\n", iret);

	lret = c_zht_lookup2(key, &result);
	fprintf(stderr, "c_zht_lookup, return code: %d\n", lret);
	fprintf(stderr, "c_zht_lookup, return value: %s.\n", result);

	c_zht_teardown();

}

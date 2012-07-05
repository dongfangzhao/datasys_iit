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
//	(!strcmp("TCP", tcpFlag)) ? useTCP = true : useTCP = false;

	if (!strcmp("TCP", tcpFlag)) {
		useTCP = true;
	} else {
		useTCP = false;
	}

	c_zht_init(argv[1], argv[2], useTCP); //neighbor zht.cfg TCP

	char *key = "Hello";
	char *value = "ZHT";

	int iret = 100;
	iret = c_zht_insert2(key, value);
	fprintf(stderr, "c_zht_insert, return code: %d\n", iret);

	char *result;
//	int lret = 100;
	result = c_zht_lookup2(key);
//	fprintf(stderr, "c_zht_lookup, return code: %d\n", lret);
	fprintf(stderr, "c_zht_lookup, return value: %s\n", result);

	int rret = 100;
	rret = c_zht_remove2(key);
	fprintf(stderr, "c_zht_remove, return code: %d\n", rret);

	c_zht_teardown();

}

#include   <stdbool.h>
#include   <stdlib.h>
#include   <stdio.h>

#include   <string.h>
#include "c_zhtclient.h"

const int LOOKUP_SIZE = 2048;

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

	const char *key = "/testFile.00000001";
	const char *value = "192.168.1.23";

	const char *largeKey = "keyofLargeValue";
	char largeVal[1024] = {0};
	memset(largeVal, '1', 1023);

	int iret = c_zht_insert2(key, value);
	fprintf(stderr, "c_zht_insert, return code: %d\n", iret);


	size_t n;
	char *result = (char*) calloc(LOOKUP_SIZE, sizeof(char));
	if (result != NULL) {
		int lret = c_zht_lookup2(key, result, &n);
		fprintf(stderr, "c_zht_lookup, return code: %d\n", lret);
		fprintf(stderr, "c_zht_lookup, return value: %lu, %s\n", n, result);
	}

	value = "new value";
	iret = c_zht_insert2(key, value);
	fprintf(stderr, "c_zht_insert, return code: %d\n", iret);


	result = (char*) calloc(LOOKUP_SIZE, sizeof(char));
	if (result != NULL) {
		int lret = c_zht_lookup2(key, result, &n);
		fprintf(stderr, "c_zht_lookup, return code: %d\n", lret);
		fprintf(stderr, "c_zht_lookup, return value: %lu, %s\n", n, result);
	}

	const char key2 = "nonexistent_key";
	int rret = c_zht_remove2(key);
	fprintf(stderr, "c_zht_remove, return code: %d\n", rret);

	c_zht_teardown();

	free(result);
	return 0;
}

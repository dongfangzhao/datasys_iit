/**
 * DFZ, 08/08/2012: updated with serialization interface
 *
 * DFZ, 07/14/2012: change zht_lookup() interface
 *
 * DFZ, 07/05/2012: added five ZHT based functions:
 * 		zht_init(), zht_insert(), zht_lookup(), zht_remove() and zht_free()
 *
 * DFZ, 06/27/2012: The hashtable is implemented with <search.h> for now.
 * This will be updated with more fancy ones
 */

/*for net facilities*/
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include "params.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>

/*for ZHT interface */
#include <stdbool.h>
#include <errno.h>
#include "./zht/inc/c_zhtclient.h"
#include "./zht/inc/meta.pb-c.h"

#include "log.h"
#include "util.h"

/*
 * get the ip address of the local machine
 */
int net_getmyip(char *addr) {
//	char hostname[PATH_MAX] = {0};
//	struct hostent *host = (struct hostent *) malloc(sizeof(struct hostent));
//
//	gethostname(hostname, PATH_MAX) ;
//	host = (struct hostent *) gethostbyname(hostname) ;
//
//	strcpy(addr, inet_ntoa(*((struct in_addr *)host->h_addr)));
////	printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)host->h_addr)));

	struct ifaddrs * ifAddrStruct = NULL;
	struct ifaddrs * ifa = NULL;
	void * tmpAddrPtr = NULL;

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		/*skip the loopback*/
		if (!strcmp("lo", ifa->ifa_name))
			continue;

		if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
//            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
			strcpy(addr, addressBuffer);
		}
//        else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
//            // is a valid IP6 Address
//            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
//            char addressBuffer[INET6_ADDRSTRLEN];
//            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
//            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
//        }
	}
	if (ifAddrStruct != NULL)
		freeifaddrs(ifAddrStruct);

	return 0;
}

/**
 *******************************************************************
 ** The following five operations are imported from ZHT utilities **
 *******************************************************************
 */

int zht_init()
{
	/* use TCP by default */
	c_zht_init("./src/zht/neighbor", "./src/zht/zht.cfg", true);

//	/* DFZ: debug info */
//	printf("\n =====DFZ debug: %s \n", "zht_init() succeeded. ");

	return 0;
}

int zht_free()
{
	c_zht_teardown();

//	/* DFZ: debug info */
//	printf("\n =====DFZ debug: %s \n", "zht_free() succeeded. ");

	return 0;
}

int zht_insert(const char *key, const char *value)
{
//	return c_zht_insert2(key, value);
	Package package = PACKAGE__INIT; // Package
	package.virtualpath = (char*)key;
	package.realfullpath = (char*)value;
	package.has_operation = true;
	package.operation = 3; //1 for look up, 2 for remove, 3 for insert

	char *buf; // Buffer to store serialized data
	unsigned len; // Length of serialized data

	len = package__get_packed_size(&package);
	buf = (char*) calloc(len, sizeof(char));
	package__pack(&package, (uint8_t *)buf);

	int ret = c_zht_insert(buf);
	if (ret)
		fprintf(stderr, "c_zht_insert, return code %d. \n", ret);

	free(buf); // Free the allocated serialized buffer

	return 0;
}

int zht_lookup(const char *key, char *val)
{
//	return c_zht_lookup2(key, val);
//	size_t len;
//	return c_zht_lookup2(key, val, &len);

	char *buf; // Buffer to store serialized data
	unsigned len; // Length of serialized data
	char result[ZHT_MAX_BUFF] = {0}; //return result
	size_t ln; //return length

	Package package = PACKAGE__INIT; // Package
	package.virtualpath = (char*)key;
	package.realfullpath = "";
	package.has_operation = true;
	package.operation = 1; //1 for look up, 2 for remove, 3 for insert

	len = package__get_packed_size(&package);
	buf = (char*) calloc(len, sizeof(char));
	package__pack(&package, (uint8_t *)buf);

	int lret = c_zht_lookup(buf, result, &ln);
	if (lret == 0 && ln > 0) {
		Package *lkPackage;
//		char *lkBuf = (char*) calloc(ln, sizeof(char));
//		char lkBuf[ln + 1] = {0};
//		strncpy(lkBuf, result, ln);
		lkPackage = package__unpack(NULL, ln, (const uint8_t *)result);

		if (lkPackage == NULL) {
			fprintf(stderr, "error unpacking lookup result\n");
		}
//		else {
//			fprintf(stderr,
//					"c_zht_lookup, return {key}:{value} ==>\n{%s}:{%s}\n",
//					lkPackage->virtualpath, lkPackage->realfullpath);
//		}

//		free(lkBuf);

		strcpy(val, lkPackage->realfullpath);
		package__free_unpacked(lkPackage, NULL);
	}

	free(buf); // Free the allocated serialized buffer

	return 0;
}

int zht_remove(const char *key)
{
//	return c_zht_remove2(key);

	Package package = PACKAGE__INIT; // Package
	package.virtualpath = (char*)key;
	package.realfullpath = "";
	package.has_operation = true;
	package.operation = 2; //1 for look up, 2 for remove, 3 for insert

	char *buf; // Buffer to store serialized data
	unsigned len; // Length of serialized data

	len = package__get_packed_size(&package);
	buf = (char*) calloc(len, sizeof(char));
	package__pack(&package, (uint8_t *)buf);

	int ret = c_zht_remove(buf);
	if (ret)
		fprintf(stderr, "c_zht_remove, return code %d\n", ret);

	free(buf); // Free the allocated serialized buffer

	return 0;
}

/**
 *****************************************************************************
 ** The following 3 functions are hashtable implementations from <search.h> **
 *****************************************************************************
 */

/**
 * Insert <key, val> into the global hash table
 *
 * return int:
 * 		0 - success
 * 		1 - failed to insert
 */
int ht_insert(const char *key, const char *val)
{
	e.key = (char *)key;
	e.data = (void *)val;

	ep = hsearch(e, ENTER);
	if (NULL == ep) {
		fprintf(stderr, "entry failed in hash table \n");
		exit(EXIT_FAILURE);
	}

	/* DFZ: debug info */
	log_msg("\n =====DFZ debug: ep->key = %s, ep->data = %s \n", ep->key, ep->data);

	/*
	 * DFZ: test ht_search
	 */
	ep = ht_search("/tmpfile");
	if ((ENTRY*)0 != ep)
		log_msg("\n =====DFZ debug: ep->key = %s, ep->data = %s (after 'ht_search')\n", ep->key, ep->data);
	else
		log_msg("\n =====DFZ debug: not found '/tmpfile' \n ");

	return 0;
}

/**
 * Remove <key, val> from the global hash table
 *
 * return int:
 * 		0 - success
 * 		1 - <key> not found in the hash table
 */
int ht_remove(const char *key)
{
	e.key = (char *)key;
	e.data = (void *)0;

	ep = hsearch(e, ENTER);
	if (NULL == ep) {
		fprintf(stderr, "entry failed in hash table \n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

/**
 * Search <key> from the global hash table
 *
 * return *ENTRY:
 * 		0 - not found
 * 		Otherwise - the pointer of the first found entry
 */
ENTRY* ht_search(const char *key)
{
	e.key = (char*) key;
	ep = hsearch(e, FIND);

	if (NULL != ep) {
		log_msg("\n ========DFZ debug: in ht_search(): ep->key = %s, ep->data = %s", ep->key, ep->data);
		return ep;
	}

	log_msg("\n ========DFZ debug: in ht_search(): %s not found ", key);

	return (ENTRY*)0;
}

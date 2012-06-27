/**
 * The hashtable is implemented with <search.h> for now.
 * This will be updated with more fancy ones
 */

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

#include "log.h"
#include "util.h"
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

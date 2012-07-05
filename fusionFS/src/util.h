#ifndef _UTIL_H_
#define _UTIL_H_

int ht_insert(const char *key, const char *val);
int ht_remove(const char *key);
ENTRY* ht_search(const char *key);

int zht_init();
int zht_free();
int zht_insert(const char *key, const char *value);
const char* zht_lookup(const char *key);
int zht_remove(const char *key);

#endif

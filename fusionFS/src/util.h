#ifndef _UTIL_H_
#define _UTIL_H_

int ht_insert(const char *key, const char *val);
int ht_remove(const char *key);
ENTRY* ht_search(const char *key);

#endif

#ifndef C_ZHTCLIENT_H_
#define C_ZHTCLIENT_H_

#ifdef __cplusplus
# define ZHT_CPP(x) x
#else
# define ZHT_CPP(x)
#endif

ZHT_CPP(extern "C" {)

	int c_zht_init(const char *memberConfig, const char *zhtConfig, bool tcp);

	int c_zht_insert(const char *pair);
	int c_zht_insert2(const char *key, const char *value);

	int c_zht_lookup(const char *pair, char *result);
	const char* c_zht_lookup2(const char *key);

	int c_zht_remove(const char *pair);
	int c_zht_remove2(const char *key);

	int c_zht_teardown();

ZHT_CPP	(})

#endif

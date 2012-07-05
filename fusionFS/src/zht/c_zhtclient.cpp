#include "c_zhtclient.h"
#include "cpp_zhtclient.h"
#include <string.h>

ZHTClient zhtClient;
bool TCP = false;

int c_zht_init(const char *memberConfig, const char *zhtConfig, bool tcp) {

	string zhtStr(zhtConfig);
	string memberStr(memberConfig);

	if (zhtClient.initialize(zhtStr, memberStr, tcp) != 0) {
		printf("ZHTClient initialization failed, program exits.");

		return -1;
	}

	return 0;
}

int c_zht_insert(const char *pair) {

	string str(pair);

	return zhtClient.insert(str);
}

int c_zht_insert2(const char *key, const char *value) {
	string keyStr(key);

	Package package;
	package.set_virtualpath(keyStr); //as key
	package.set_isdir(true);
	package.set_replicano(5);
	package.set_operation(3); //1 for look up, 2 for remove, 3 for insert
	package.set_realfullpath(value);

	return zhtClient.insert(package.SerializeAsString());
}

int c_zht_lookup(const char *pair, char *result) {

	string pkg(pair);
	string returnStr;

	int ret = zhtClient.lookup(pkg, returnStr);

	char *chars = new char[returnStr.size() + 1];
	strcpy(chars, returnStr.c_str());

	result = chars;

	return ret;
}

const char* c_zht_lookup2(const char *key){

	string keyStr(key);
	string resultStr;

	Package package;
	package.set_virtualpath(keyStr); //as key
	package.set_isdir(true);
	package.set_replicano(5);
	package.set_operation(1); //1 for look up, 2 for remove, 3 for insert

	int ret = zhtClient.lookup(package.SerializeAsString(), resultStr);

	package.ParseFromString(resultStr);

	return package.realfullpath().c_str();
/*
	string str = package.realfullpath();
	char* a = new char[str.size() + 1];
	a[str.size()] = 0;
	//strcpy(result, str.c_str());
	memcpy(a, str.c_str(), str.size());

*/
}

int c_zht_remove(const char *pair) {

	string str(pair);

	return zhtClient.remove(str);
}

int c_zht_remove2(const char *key) {

	string keyStr(key);

	Package package;
	package.set_virtualpath(keyStr);
	package.set_operation(2); //1 for look up, 2 for remove, 3 for insert

	return zhtClient.remove(package.SerializeAsString());
}

int c_zht_teardown() {

	return zhtClient.tearDownTCP();
}


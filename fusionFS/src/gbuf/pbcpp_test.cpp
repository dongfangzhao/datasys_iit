/**
 * file: 	pbcpp_test.cpp
 * desc: 	to test if the C and C++ versions of Google Proto Buffers are compatible
 * author: 	dzhao8@hawk.iit.edu
 * date:	06/30/2012
 *
 * To Compile:
g++ -L/usr/local/lib -lprotobuf -lprotoc -lpthread -I/usr/local/include/google/protobuf meta.pb.cc pbcpp_test.cpp -o pbcpp_test
 */

#include "meta.pb.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main()
{
	Package package, pkg_rtn;

	/* test data */
	package.set_virtualpath("mykey");
	package.set_realfullpath("mypathname");
	package.set_replicano(5);
	package.set_operation(3);

	/* serialize the data */
	string str = package.SerializeAsString();

	/* write str to file, which will be read by a C program */
	ofstream myfile;
	myfile.open ("cpp.file");
	myfile << str;
	myfile.close();

	/* read str from file written by a C program */
	char *buf;
	ifstream::pos_type size;
	ifstream infile("serialize.file", ios::in | ios::binary | ios::ate);
	if (infile.is_open()) {
		size = infile.tellg();
		buf = new char[size];
		infile.seekg(0, ios::beg);
		infile.read(buf, size);
		infile.close();

	} else {
		cerr << "Unable to open serialize.file ";
		return 1;
	}

	/* parse the data to the structure */
	pkg_rtn.ParseFromString(buf);
	delete[] buf;

	/* verify the matchings */
	cout << "virtualpath = " << pkg_rtn.virtualpath() << endl;
	cout << "realfullpath = " << pkg_rtn.realfullpath() << endl;
	cout << "replicano = " << pkg_rtn.replicano() << endl;
	cout << "operation = " << pkg_rtn.operation() << endl;

	cout << "pbcpp_test succeed. " << endl;
	return 0;
}

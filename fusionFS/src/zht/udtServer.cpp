/*
 * udtServer.cpp
 *
 *  Created on: Jun 11, 2012
 *      Author: tony
 */

#include <arpa/inet.h>
#include <udt/udt.h>
#include <string.h>
#include <iostream>

using namespace std;

int main() {
	UDTSOCKET serv = UDT::socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(9000);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8);

	if (UDT::ERROR == UDT::bind(serv, (sockaddr*) &my_addr, sizeof(my_addr))) {
		cout << "bind: " << UDT::getlasterror().getErrorMessage();
		return 0;
	}

	UDT::listen(serv, 10);

	int namelen;
	sockaddr_in their_addr;

	UDTSOCKET recver = UDT::accept(serv, (sockaddr*) &their_addr, &namelen);

	char ip[16];
	cout << "new connection: " << inet_ntoa(their_addr.sin_addr) << ":"
			<< ntohs(their_addr.sin_port) << endl;

	char data[100];

	if (UDT::ERROR == UDT::recv(recver, data, 100, 0)) {
		cout << "recv:" << UDT::getlasterror().getErrorMessage() << endl;
		return 0;
	}

	cout << data << endl;

	UDT::close(recver);
	UDT::close(serv);

	return 1;
}


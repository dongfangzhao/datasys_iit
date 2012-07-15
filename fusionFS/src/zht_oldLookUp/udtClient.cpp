/*
 * udtClient.cpp
 *
 *  Created on: Jun 11, 2012
 *      Author: tony
 */

#include <iostream>
#include <udt/udt.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

using namespace std;
using namespace UDT;

int main() {
	UDTSOCKET client = UDT::socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(9000);
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

	memset(&(serv_addr.sin_zero), '\0', 8);

	char* host = "localhost";
	int port =9000;
	struct sockaddr_in dest;
		memset(&dest, 0, sizeof(struct sockaddr_in)); /*zero the struct*/
		struct hostent * hinfo = gethostbyname(host);
		if (hinfo == NULL)
			printf("getbyname failed!\n");
		dest.sin_family = PF_INET; /*storing the server info in sockaddr_in structure*/
		dest.sin_addr = *(struct in_addr *) (hinfo->h_addr); /*set destination IP number*/
		dest.sin_port = htons(port);
/*		to_sock = socket(PF_INET, SOCK_STREAM, 0); //try change here.................................................
		if (to_sock < 0) {
			cerr << "net_util: error on socket(): " << strerror(errno) << endl;
			return -1;
		}
*/


// connect to the server, implict bind
	if (UDT::ERROR
			== UDT::connect(client, (sockaddr*) &dest,
					sizeof(dest))) {
		cout << "connect: " << UDT::getlasterror().getErrorMessage();
		return 0;
	}

	char* hello = "hello world!\n";
	if (UDT::ERROR == UDT::send(client, hello, strlen(hello) + 1, 0)) {
		cout << "send: " << UDT::getlasterror().getErrorMessage();
		return 0;
	}

	UDT::close(client);

	return 1;
}

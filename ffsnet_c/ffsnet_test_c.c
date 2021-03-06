/**
 * File name: ffsnet_test_c.c
 *
 * Function: this is a C file to test calling the C++ library methods
 *
 * Author: dzhao8@hawk.iit.edu
 *
 * Update history:
 *		- 06/25/2012: initial development
 *
 * To compile this single file:
 * 		gcc ffsnet_test_c.c -L. -lffsnet_bridger -o ffsnet_test_c
 */


#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	if ((argc != 6) || (0 == atoi(argv[2]))) {
		printf("usage: ffsnet_test_c server_ip server_port remote_filename local_filename [download?0:1] \n");
		return -1;
	}

	if (!atoi(argv[5])) /* 0-download, 1-upload */
		ffs_recvfile_c("udt", argv[1], argv[2], argv[3], argv[4]);
	else
		ffs_sendfile_c("udt", argv[1], argv[2], argv[4], argv[3]);
}

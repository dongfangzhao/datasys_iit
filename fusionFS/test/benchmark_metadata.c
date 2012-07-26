/**
 * This is a benchmark for metadata of FusionFS
 * Author: dzhao8@iit.edu
 * History:
 *		- 07/25/2012: initial development
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/time.h>

#define FILENAME "f_"
#define TOTAL_FILE 10

int main()
{
	int retstat;

	retstat = creat_file(TOTAL_FILE);
	if (retstat) {
		return retstat;
	}

	retstat = open_file(TOTAL_FILE);
	if (retstat) {
		return retstat;
	}

	retstat = remove_file(TOTAL_FILE);
	if (retstat) {
		return retstat;
	}

	return 0;
}

/*
 *get current timestamp
 */
double getFloatTime()
{
	struct timeval t;
	gettimeofday(&t, 0);
	return (double) t.tv_sec + (double) t.tv_usec / 1000000.0;
}

int creat_file(int num_of_files)
{
	double start, end, ops;
	start = getFloatTime();
	int fileno = 0;
	char fname[PATH_MAX] = {0}, fileid[10] = {0};
	for (; fileno < num_of_files; fileno++){
		sprintf(fileid, "%d", fileno);
		strcpy(fname, FILENAME);
		strcat(fname, fileid);
		int fd = creat(fname, 0755);
		if (fd < 0) {
			perror("creat_file() failed to create file. ");
			return 1;
		}
		memset(fname, 0, PATH_MAX);
		memset(fileid, 0, 10);
	}
	end = getFloatTime();
	ops = (double) num_of_files / (end - start);
	printf("Create file: %10.2f ops. \n", ops);
	return 0;
}

int open_file(int num_of_files)
{
	double start, end, ops;
	start = getFloatTime();
	int fileno = 0;
	char fname[PATH_MAX] = {0}, fileid[10] = {0};
	for (; fileno < num_of_files; fileno++){
		sprintf(fileid, "%d", fileno);
		strcpy(fname, FILENAME);
		strcat(fname, fileid);
		int fd = open(fname, O_RDONLY);
		if (fd < 0) {
			perror("open_file() failed to open file. ");
			return 1;
		}
		int stat = close(fd);
		if (stat < 0) {
			perror("open_file() failed to close file. ");
			return 1;
		}
		memset(fname, 0, PATH_MAX);
		memset(fileid, 0, 10);
	}
	end = getFloatTime();
	ops = (double) num_of_files / (end - start);
	printf("Open/close file: %10.2f ops. \n", ops);
	return 0;
}

int remove_file(int num_of_files)
{
	double start, end, ops;
	start = getFloatTime();
	int fileno = 0;
	char fname[PATH_MAX] = {0}, fileid[10] = {0};
	for (; fileno < num_of_files; fileno++){
		sprintf(fileid, "%d", fileno);
		strcpy(fname, FILENAME);
		strcat(fname, fileid);

//		char cmd_rm[PATH_MAX] = {0};
//		strcpy(cmd_rm, "rm ");
//		strcat(cmd_rm, fname);
//		system(cmd_rm);

		/*remove is actually rename in Linux...*/
		int stat = remove(fname);
		if (stat < 0) {
			perror("open_file() failed to close file. ");
			return 1;
		}
		memset(fname, 0, PATH_MAX);
		memset(fileid, 0, 10);
	}
	end = getFloatTime();
	ops = (double) num_of_files / (end - start);
	printf("Remove file: %10.2f ops. \n", ops);
	return 0;
}

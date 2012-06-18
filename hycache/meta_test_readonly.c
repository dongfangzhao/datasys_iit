#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>

double getFloatTime()
{
	struct timeval t;
	
	gettimeofday(&t, 0);
	
	return (double) t.tv_sec + (double) t.tv_usec / 1000000.0;
}

main()
{
	char *tmpfname_prefix = "meta_tmpfile";
	int fd, i = 0, iter = 1000;
	double start, end, tot;
	char tmpfname[32], tmpfname_new[32];
	
	tot = 0;
	for (i = 0; i < iter; i++)
	{	
		sprintf(tmpfname, "%s_%04d%s", tmpfname_prefix, i, "_new");
		
		start = getFloatTime();
		if((fd = open(tmpfname, O_RDONLY)) == -1) 
		{
			perror("Error: open");
			exit(1);
		}
		end = getFloatTime();
		
		tot += end-start;
		
		close(fd);
	}	
	printf("Open file: %.0f/s \n", 1000/tot);
}
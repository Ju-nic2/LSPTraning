#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <utime.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

char getOption(char *argv);
void setTime(int argc, char **argv);
void setTimeSame(char *origin, char *new);

int main(int argc, char **argv)
{
	if(argc == 2 && !strcmp(argv[1],"--help"))
	{
		printf("touch [option] <file1> .. <filen>\n");
		printf("option -r : you must! wirtie mytouch -r file1 file2\n");
		printf("if you don't follow that rule , core dump\n");
		exit(1);
	}

	if(getOption(argv[1]) == 'r' && argc == 4)
		setTimeSame(argv[2],argv[3]);
	else
		setTime(argc,argv);
	
}

char getOption(char*argv)
{
	if(argv[0] == '-' && strlen(argv) > 1)
		return argv[1];
	else
		return ' ';
}

void setTime(int argc, char **argv)
{
	for(int i = 1; i<argc; i++)
	{
		int fd;
		if((fd = open(argv[i],O_CREAT|O_EXCL,0644)) == -1)
		{
			utime(argv[i],NULL);
		}
	}
}
void setTimeSame(char *origin, char *new)
{
	struct stat st;
	struct utimbuf tb;
	if(lstat(origin,&st) == -1)
	{
		fprintf(stderr,"cant open %s",origin);
		exit(1);
	}
	
	tb.actime = st.st_atime;
	tb.modtime = st.st_mtime;
	
	if(lstat(new,&st) == -1)
	{
		creat(new,0644);
		utime(new,&tb);
	}else
		utime(new,&tb);
}

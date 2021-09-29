#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <utmp.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>
//vertival order in file
#define STAT_CPU_ROW 0
#define STAT_BTIME_ROW 4
#define STAT_TTY_NR_IDX 6

//index for temp array
#define PIDSTAT_STATE_TOK 2
#define PIDSTAT_CMD_TOK 1
#define PIDSTAT_SID_TOK 5
#define PIDSTAT_TPGID_TOK 7
#define PIDSTAT_UTIME_TOK 13
#define PIDSTAT_STIME_TOK 14
#define PIDSTAT_PR_TOK 17
#define PIDSTAT_NI_TOK 18
#define PIDSTAT_THREAD_TOK 19
#define PIDSTAT_STARTTIME_TOK 21

#define PIDSTATUS_VMRT_ROW 17
#define PIDSTATUS_RES_ROW 20
#define PIDSTATUS_SHR_ROW 23

#define LINEBUFSIZE 1024
#define MAX_ROW_LEN 1024
#define NAMEBUFSIZE 128

#define PATH_LEN 1024
#define UNAME_LEN 32
#define TTY_LEN 32
#define STAT_LEN 8
#define TIME_LEN 16
#define CMD_LEN 1024

#define EXTR_NUM -1
#define EXTR_VMS 0
#define EXTR_VMH 1
#define EXTR_RSS 2

#define OPTION_A 0X01
#define OPTION_U 0X02
#define OPTION_X 0X04

typedef struct processInfo{
    int printFlag;
	int pid;
	char userName[UNAME_LEN];		//user명
	long double cpuUsage;			//cpu 사용률
	long double memUsage;			//메모리 사용률
	unsigned long vsz;			//가상 메모리 사용량
	unsigned long rss;			//실제 메모리 사용량
	unsigned long shr;			//공유 메모리 사용량
	int priority;				//우선순위
	int ni;					    //nice 값
	char tty[TTY_LEN];			//터미널
	char status[STAT_LEN];		//상태
	char start[TIME_LEN];		//프로세스 시작 시각
	char time[TIME_LEN];		//총 cpu 사용 시간
	char cmd[CMD_LEN];			//option 없을 경우에만 출력되는 command (short)
	char command[CMD_LEN];		//option 있을 경우에 출력되는 command (long)
}PROCESSINFO;

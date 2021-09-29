#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define FLAG_I 0x01
#define FLAG_N 0x02
#define FLAG_P 0x04

#define CPUSORT 0
#define MEMSORT 1
#define TIMSORT 2

//number of data in file
#define CPU_USAGE 8
#define MEM_DATAS 8

//horizontal order in line
#define STAT_BTIME_TOK 1
#define STAT_CPU_US 1
#define STAT_CPU_SY 3
#define STAT_CPU_NI 2
#define STAT_CPU_ID 4
#define STAT_CPU_WA 5
#define STAT_CPU_HI 6
#define STAT_CPU_SI 7
#define STAT_CPU_ST 8

#define UTIME_UTIME_TOK 1

#define LOADAVG_1MIN_TOK 0
#define LOADAVG_5MIN_TOK 1
#define LOADAVG_15MIN_TOK 2

//vertival order in file
#define STAT_CPU_ROW 0
#define STAT_BTIME_ROW 4

#define MEMINFO_TOTAL_ROW 0
#define MEMINFO_FREE_ROW 1
#define MEMINFO_AVAIL_ROW 2 
#define MEMINFO_BUFFER_ROW 3 
#define MEMINFO_CACHED_ROW 4
#define MEMINFO_SWAPTOTAL_ROW 14
#define MEMINFO_SWAPFREE_ROW 15
#define MEMINFO_SRECLAIM_ROW 23

// index for temp array
#define MEMINFO_TOTAL_IDX 0
#define MEMINFO_FREE_IDX 1
#define MEMINFO_AVAIL_IDX 2
#define MEMINFO_BUFFER_IDX 3
#define MEMINFO_CACHED_IDX 4
#define MEMINFO_SWAPTOTAL_IDX 5
#define MEMINFO_SWAPFREE_IDX 6
#define MEMINFO_SRECLAIM_IDX 7

#define PIDSTAT_STATE_TOK 2
#define PIDSTAT_CMD_TOK 1
#define PIDSTAT_UTIME_TOK 13
#define PIDSTAT_STIME_TOK 14
#define PIDSTAT_PR_TOK 17
#define PIDSTAT_NI_TOK 18
#define PIDSTAT_STARTTIME_TOK 21

#define PIDSTATUS_VMRT_ROW 17
#define PIDSTATUS_RES_ROW 20
#define PIDSTATUS_SHR_ROW 23

#define MAX_PROCESS_ID 200000
#define MAX_PROCESS_NUM 4096
#define LINEBUFSIZE 2048
#define MAX_ROW_LEN 1024
#define NAMEBUFSIZE 128


// data structures
typedef struct loadavg{
	float min_1;
	float min_5;
	float min_15;
}LOADAVG;

typedef struct proc_stat{
	unsigned long btime;
	double us;
	double sy;
	double ni;
	double id;
	double wa;
	double hi;
	double si;
	double st;
}PROC_STAT;

typedef struct meminfo{
	double total;
	double free;
	double used;
	double buf_cache;
	double swap_total;
	double swap_free;
	double swap_used;
	double avail;
}MEMINFO;

typedef struct processInfo
{
	int pid;
	char username[NAMEBUFSIZE];
	int priority;
	int ni;
	unsigned long virt;
	unsigned long res;
	unsigned long shr;
	char status;
	double cpuUsage;
	double memUsage;
	double time;	
	char command[NAMEBUFSIZE];
}PROCESSINFO;

typedef struct threadArg
{
	int pid;
	int index;
	int sortF;
}THREADARG;

typedef struct sortNode
{
	int pid;
	double cpuUsage;
	double memUsage;
	unsigned long time;
	struct sortNode* next;
}SORTNODE;


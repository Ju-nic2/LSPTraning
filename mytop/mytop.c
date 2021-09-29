#include "myheader.h"
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <ncurses.h>
#include <utmp.h>
#include <termios.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/shm.h>

static PROC_STAT ps;
static MEMINFO mem;
static LOADAVG loadavg;

static int indexTable[MAX_PROCESS_ID];
static PROCESSINFO *processTable[MAX_PROCESS_NUM];
static pthread_t threadTable[MAX_PROCESS_NUM];

unsigned int userCnt;

unsigned int tasks = 0;
unsigned int running = 0;
unsigned int sleeping = 0;
unsigned int stopped = 0;
unsigned int zombie = 0;

int startPid = 1;
int endPid = 0;
int line =  7;

int sortFlag = 0;
int endFlag = 0;

int iteration = 1;
int specificPid = 0; 
int FLAG;

SORTNODE *head = NULL;

void* setProcStatInfo(void* arg);
void* setMemInfo(void* arg);
void* setloadavgInfo(void* arg);
void* setProcessInfo(void* arg);

unsigned long extractNum(char *line,int flag);
int countAllProcessInProc();

void extractCmd(char *ptr, char *source);
void initializeArr();
int getUserNum();
unsigned int getUpTime();
void initBuffer(char (*buf)[LINEBUFSIZE]);
// for sort resources
void pushList(SORTNODE *node);
void sortByCpu(); 
void sortByMem();
void sortByTim();
void resetList();

//print function
void print_mytop();

void parseArgv(int argc, char **argv);

void sighandler(int signo)
{ }
void* getchThread();

void *t();
pthread_barrier_t basicFileRead;
pthread_barrier_t basicTimeSynk;

pthread_barrier_t fileRead;
pthread_barrier_t timeSynk;
pthread_mutex_t lock;

int main(int argc, char **argv)
{
	if(argc > 1) parseArgv(argc, argv);
	signal(SIGALRM,sighandler);
	pthread_mutex_init(&lock,NULL);
	initializeArr();
	print_mytop();
	exit(0);
}

void *t()
{
	while(1)
	{
		int ch = getch();
		switch(ch)
		{
			case 'M' : sortFlag = MEMSORT; raise(SIGALRM); break;
			case 'T' : sortFlag = TIMSORT; raise(SIGALRM);break;
			case 'C' : sortFlag = CPUSORT; raise(SIGALRM); break;
			case 'q' : endFlag = 1; raise(SIGALRM); break;
			case 'a' : line++; raise(SIGALRM); break;
			case 'b' : line--; raise(SIGALRM);break;
			
		}
	}
}

void parseArgv(int argc, char **argv)
{
	for(int i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(argv[i][1] == 'n' && i+1 < argc) {FLAG |= FLAG_N; iteration = atoi(argv[i+1]); continue;}
			else{fprintf(stderr,"Optinon Error Please see -h\n"); exit(0);}
		}
	}
}


void initializeArr()
{
	//initialize Tables
	for(int i = 0; i <MAX_PROCESS_ID; i++)
	{
		indexTable[i] = -1;
	}
	for(int i =0; i< MAX_PROCESS_NUM; i++)
		processTable[i] = NULL;

}
unsigned int getUpTime()
{
	unsigned int uptime = 0;
	char buf[LINEBUFSIZE];
	FILE *fp;
	if((fp = fopen("/proc/uptime","r")) == NULL) fprintf(stderr,"/proc/uptime open err\n");
	memset(buf,'\0',LINEBUFSIZE);
	fgets(buf,LINEBUFSIZE,fp);
	char *timeptr = strtok(buf," ");
	uptime = atol(timeptr);
	if(fp != NULL)
		fclose(fp);
	return uptime;
}

int getUserNum()
{
	int user = 0;
	
	struct utmp *ut;
	setutent();
	while((ut = getutent())!=NULL){
		if(ut->ut_type == USER_PROCESS)
			user++;
	}
	endutent();
	return user;
}


void initBuffer(char (*buf)[LINEBUFSIZE])
{
	for(int i = 0; i < MAX_ROW_LEN; i++)
		for(int j = 0; j< LINEBUFSIZE; j++)
			buf[i][j] = '\0';
}
void print_mytop()
{
	char printBuffer[MAX_ROW_LEN][LINEBUFSIZE];

	pthread_barrier_init(&basicFileRead,NULL,4);
	pthread_barrier_init(&basicTimeSynk,NULL,4);
	pthread_t basicTids[4];
	pthread_t tid;
	pthread_create(&basicTids[0],NULL,setProcStatInfo,NULL);
	pthread_create(&basicTids[1],NULL,setMemInfo,NULL);
	pthread_create(&basicTids[2],NULL,setloadavgInfo,NULL);
	pthread_create(&basicTids[3],NULL,t,NULL);
	initscr();
	int count = 7;
	int icount = 0;
	while(icount < iteration)
	{
		pthread_mutex_lock(&lock);
		
		int test = sortFlag;
		int threadCount = countAllProcessInProc(test);	
		pthread_barrier_init(&timeSynk,NULL,threadCount+1);
		pthread_barrier_init(&fileRead,NULL,threadCount+1);
		pthread_mutex_unlock(&lock);

		initBuffer(printBuffer);
		
		struct winsize wsize;

		if (ioctl(0, TIOCGWINSZ, (char *)&wsize) < 0){
			fprintf(stderr, "ioctl error\n");
			exit(1);
		}

		int w_row = wsize.ws_row;
		int w_col = wsize.ws_col;
	
		time_t t = time(NULL);
		struct tm *lt = localtime(&t);
		unsigned int uptime = getUpTime();
		int up_hour = uptime/3600;
		int up_min = (uptime - up_hour*3600)/60;
		
		int user = getUserNum();
		
		pthread_barrier_wait(&basicFileRead);
		pthread_barrier_wait(&fileRead);
		//all dates are set
		
		switch(test)
		{
			case CPUSORT : sortByCpu();break;
			case MEMSORT : sortByMem();break;
			case TIMSORT : sortByTim();break;
		}

		sprintf(printBuffer[0],"top - %02d:%02d:%02d up %02d:%02d, %d user, load average %3.2f, %3.2f, %3.2f",
				lt->tm_hour,lt->tm_min,lt->tm_sec,up_hour,up_min,user,loadavg.min_1,loadavg.min_5,loadavg.min_15);
		sprintf(printBuffer[1],"Tasks: %3d total, %3d running, %3d sleeping %3d stopped, %3d zombie", 
				running+sleeping+stopped+zombie, running, sleeping, stopped, zombie);
		sprintf(printBuffer[2],"Cpu(s): %3.1f us, %3.1f sy, %3.1f ni, %3.1f id, %3.1f wa, %3.1f hi, %3.1f si, %3.1f st", 
				ps.us,ps.sy,ps.ni,ps.id,ps.wa,ps.hi,ps.si,ps.st);
		sprintf(printBuffer[3],"MiB Mem : %8.1f total, %8.1f free, %8.1f used, %8.1f buff/cache", 
				mem.total, mem.free, mem.used, mem.buf_cache);
		sprintf(printBuffer[4],"MiB Swap: %8.1f total, %8.1f free, %8.1f used, %8.1f avail Mem", 
				mem.swap_total, mem.swap_free, mem.swap_used, mem.avail);
		
		sprintf(printBuffer[6], "%5s %-8s %3s %3s %7s %6s %6s %c %4s %4s   %7s %s",
			"PID", "USER", "PR", "NI", "VIRT", "RES", "SHR", 'S', "CPU", "%MEM", "TIME+", "COMMAND");
		
			
		SORTNODE *cur = head;
		count = 7;
		while(cur != NULL){
			snprintf(printBuffer[count],w_col ,"%5d %-8s %3d %3d %7ld %6ld %6ld %c %4.1f %4.1f   %d:%2.2f %s",
					processTable[indexTable[cur->pid]]->pid, processTable[indexTable[cur->pid]]->username,
					processTable[indexTable[cur->pid]]->priority,processTable[indexTable[cur->pid]]->ni,
					processTable[indexTable[cur->pid]]->virt,processTable[indexTable[cur->pid]]->res,
					processTable[indexTable[cur->pid]]->shr,
					processTable[indexTable[cur->pid]]->status, processTable[indexTable[cur->pid]]->cpuUsage,
					processTable[indexTable[cur->pid]]->memUsage,
					(int)processTable[indexTable[cur->pid]]->time/60,
					fmod(processTable[indexTable[cur->pid]]->time,60.00),
					processTable[indexTable[cur->pid]]->command);
			count++;
			indexTable[cur->pid] = -1;
			cur = cur->next;
		}
		int pid = startPid;
		while(count	<= threadCount)
		{
			if(indexTable[pid] >= 0)
			{
				snprintf(printBuffer[count],w_col, "%5d %-8s %3d %3d %7ld %6ld %6ld %c %4.1f %4.2f  %d:%2.2f %s",
					processTable[indexTable[pid]]->pid, processTable[indexTable[pid]]->username,
					processTable[indexTable[pid]]->priority,processTable[indexTable[pid]]->ni,
					processTable[indexTable[pid]]->virt,processTable[indexTable[pid]]->res,
					processTable[indexTable[pid]]->shr,
					processTable[indexTable[pid]]->status, processTable[indexTable[pid]]->cpuUsage,
					processTable[indexTable[pid]]->memUsage,
					(int)processTable[indexTable[pid]]->time/60,fmod(processTable[indexTable[pid]]->time,60.00),
					processTable[indexTable[pid]]->command);
				indexTable[pid] = -1;
				count++;
			}
			pid++;
		}
		for(int i = 0; i < 7; i++)
		{
			mvprintw(i,0,printBuffer[i]);
		}
		if(line < 7) line = 7; 
		int start = line; int end = count; 
		for(int i = 7; i < w_row; i++)
		{
			mvprintw(i,0,printBuffer[start++]);
			if(i >= w_row) break;
		}
		for(int i = count; i < w_row; i++)
		{
			//mvprintw(i,0,printBuffer[i]);
		}
		refresh();
		running = 0;
		sleeping = 0;
		zombie = 0;
		resetList();
		alarm(3);
		pause();
		if(FLAG & FLAG_N) icount++;
		pthread_barrier_wait(&timeSynk);
		pthread_barrier_wait(&basicTimeSynk);
		if(endFlag == 1) {
			for(int i = 0; i<4; i++ )
			{
				pthread_kill(basicTids[i],SIGINT);
			}
			
			for(int i = 0; i < w_row; i++)
			{
				printf("%s\n",printBuffer[i]);
			}

			break;
		}
	}
	endwin();


	pthread_barrier_destroy(&fileRead);
	pthread_barrier_destroy(&basicFileRead);
	pthread_barrier_destroy(&timeSynk);
	pthread_barrier_destroy(&basicTimeSynk);
	
}

void pushList(SORTNODE *node)
{
	if(head == NULL)
		head = node;
	else{
		SORTNODE *tmp = head;
		while(tmp -> next != NULL)
			tmp = tmp->next;
	   tmp->next = node;	
	}
}
void sortByCpu()
{
	SORTNODE *cur = head;
	while(cur->next != NULL)
	{
		SORTNODE *nextCur = cur->next;
		while(nextCur != NULL){
			if(cur->cpuUsage < nextCur->cpuUsage)
			{
				int pid = cur->pid;
				double tmp = cur->cpuUsage;
				cur->pid = nextCur->pid;
				cur->cpuUsage = nextCur->cpuUsage;
				nextCur->pid = pid;
				nextCur->cpuUsage = tmp;
			}
			nextCur = nextCur->next;
		}
		cur = cur->next;
	}
}
void sortByMem()
{
	SORTNODE *cur = head;
	while(cur->next != NULL)
	{
		SORTNODE *nextCur = cur->next;
		while(nextCur != NULL){
			if(cur->memUsage < nextCur->memUsage)
			{
				int pid = cur->pid;
				double tmp = cur->memUsage;
				cur->pid = nextCur->pid;
				cur->memUsage = nextCur->memUsage;
				nextCur->pid = pid;
				nextCur->memUsage = tmp;
			}
			nextCur = nextCur->next;
		}
		cur = cur->next;
	}
}
void sortByTim()
{
	SORTNODE *cur = head;
	while(cur->next != NULL)
	{
		SORTNODE *nextCur = cur->next;
		while(nextCur != NULL){
			if(cur->time < nextCur->time)
			{
				int pid = cur->pid;
				unsigned long tmp = cur->time;
				cur->pid = nextCur->pid;
				cur->time = nextCur->time;
				nextCur->pid = pid;
				nextCur->time = tmp;
			}
			nextCur = nextCur->next;
		}
		cur = cur->next;
	}
}

void resetList()
{
	if(head == NULL) return;
	else
	{
		SORTNODE *cur = head;
		while(cur != NULL)
		{
			SORTNODE *tmp = cur;
			cur = cur->next;
			free(tmp);
		}
		head = NULL;
	}
}

void extractCmd(char *ptr, char *source)
{
	memset(ptr,'\0',NAMEBUFSIZE);
	int count = 0;
	for(int i = 0; i<strlen(source); i++)
	{
		if(source[i] == '(') continue;
		if(source[i] == ')' ) break;
		ptr[count++] = source[i];
	}
	for(int i = count; i < NAMEBUFSIZE; i++)
		ptr[i] = '\0';
}
void* setProcessInfo(void* arg)
{

	pthread_mutex_lock(&lock);
 pthread_mutex_unlock(&lock);
	THREADARG *my = (THREADARG*)arg;
	char statfile[NAMEBUFSIZE];
	char statusfile[NAMEBUFSIZE];
	sprintf(statfile,"/proc/%d/stat",my->pid);
	sprintf(statusfile,"/proc/%d/status",my->pid);
	char buf[LINEBUFSIZE];
	FILE *fp,*fp1,*fp2;
	
	// for start synk

		//get user name using uid.
		char tmpbuf[NAMEBUFSIZE];

		struct stat statbuf;
		stat(statfile,&statbuf);
		struct passwd* up = getpwuid(statbuf.st_uid);
		strcpy(tmpbuf,up->pw_name);
		if(strlen(tmpbuf) > 8){
			tmpbuf[7] = '+';
			for(int i = 8; i < strlen(tmpbuf); i++)
			{
				tmpbuf[i] = '\0';
			}
		}
		strcpy(processTable[my->index]->username,tmpbuf);


		//for %cpu, time+ calculation
		unsigned long utime,stime,startTime;
		int hertz = (int)sysconf(_SC_CLK_TCK);
		
		if((fp1 = fopen(statfile,"r")) == NULL)
		{
		}
		int tok = 0;
		memset(buf,'\0',LINEBUFSIZE);
		if(fp1 != NULL ){
			fgets(buf,LINEBUFSIZE,fp1);
			fclose(fp1);
		}
		char *ptr = strtok(buf," ");
		while(ptr != NULL)
		{
			switch(tok){
				case PIDSTAT_STATE_TOK :processTable[my->index]->status = *ptr; break;
				case PIDSTAT_CMD_TOK : extractCmd(processTable[my->index]->command,ptr); break;
				case PIDSTAT_UTIME_TOK : utime = atoll(ptr); break;
				case PIDSTAT_STIME_TOK : stime = atoll(ptr); break;
				case PIDSTAT_PR_TOK : processTable[my->index]->priority = atoi(ptr); break;
				case PIDSTAT_NI_TOK : processTable[my->index]->ni = atoi(ptr); break;
				case PIDSTAT_STARTTIME_TOK : startTime = atol(ptr); break;
			}
			tok++;
			ptr = strtok(NULL," ");
		}
		
		if((fp2 = fopen(statusfile,"r")) == NULL){}
		int row = 0;

		while(row <= PIDSTATUS_SHR_ROW)
		{
			if(fp2 != NULL){
				memset(buf,'\0',LINEBUFSIZE);
				fgets(buf,LINEBUFSIZE,fp2);
				switch(row){
					case PIDSTATUS_VMRT_ROW : processTable[my->index]->virt = extractNum(buf,0); break;
					case PIDSTATUS_RES_ROW : processTable[my->index]->res = extractNum(buf,1); break;
					case PIDSTATUS_SHR_ROW : processTable[my->index]->shr = extractNum(buf,2); break;
				}
			}
			row++;
		}
		if(fp2 != NULL) 
			fclose(fp2);
		
		FILE *tmp;
		if((tmp = fopen("/proc/meminfo","r")) == NULL) fprintf(stderr,"/proc/meminfo open err\n");
		memset(buf,'\0',LINEBUFSIZE);
		fgets(buf,LINEBUFSIZE,tmp);
		double memTotal = (double)extractNum(buf,-1);
		if(tmp != NULL)
			fclose(tmp);
		

		unsigned long uptime = getUpTime();
		processTable[my->index]->pid = my->pid;
		processTable[my->index]->cpuUsage =((double)(utime + stime)/hertz) / (uptime-((double)startTime/hertz)) * 100;
		processTable[my->index]->memUsage = ((double)processTable[my->index]->res / memTotal) *100;
		processTable[my->index]->time = ((double)(utime+stime)/(double)hertz);
	

		pthread_mutex_lock(&lock);
		switch(processTable[my->index]->status)
		{
			case 'R' : running++; break;
			case 'S' : sleeping++; break;
			case 'I' : sleeping++; break;
			case 'Z' : zombie++; break;
			case 'T' : stopped++; break;
		}

		if(my->sortF == CPUSORT && processTable[my->index]->cpuUsage >= 0.1)
		{
			SORTNODE *node = (SORTNODE*)malloc(sizeof(SORTNODE));
			node->cpuUsage = processTable[my->index]->cpuUsage;
			node->pid = my->pid;
			node->next = NULL;
			pushList(node);
		}
		else if(my->sortF == MEMSORT && processTable[my->index]->memUsage >= 0.1)
		{
			SORTNODE *node = (SORTNODE*)malloc(sizeof(SORTNODE));
			node->memUsage = processTable[my->index]->memUsage;
			node->pid = my->pid;
			node->next = NULL;
			pushList(node);
		}
		else if(my->sortF == TIMSORT &&	processTable[my->index]->time >= 1)
		{
			SORTNODE *node = (SORTNODE*)malloc(sizeof(SORTNODE));
			node->time = processTable[my->index]->time;
			node->pid = my->pid;
			node->next = NULL;
			pushList(node);
		}

		pthread_mutex_unlock(&lock);

		pthread_barrier_wait(&fileRead);
		pthread_barrier_wait(&timeSynk);

		free(processTable[my->index]);
		processTable[my->index] = NULL;
		indexTable[my->pid] = -1;
		pthread_exit(0);
}


int countAllProcessInProc(int flag)
{
	int processCount = 0;
	DIR *dp;
	struct dirent *d;
	if((dp = opendir("/proc")) == NULL)
	{
		fprintf(stderr,"open '/proc' directory error\n");
		exit(1);
	}

	while((d = readdir(dp)) != NULL)
	{
		if(d->d_name[0] >= '0' && d->d_name[0] <= '9')
		{
			processCount++;
			long pid = atol(d->d_name);	
			//if(indexTable[pid] != -1) continue;
			//new process
			for(int i = 0; i < MAX_PROCESS_NUM;i++)
			{
				if(processTable[i] == NULL)
				{
					pthread_t tid;
					THREADARG *tmp = (THREADARG*)malloc(sizeof(THREADARG));
					tmp->pid = pid;
					tmp->index = i;
					tmp->sortF = flag;
					pthread_create(&tid,NULL,setProcessInfo,tmp);
					indexTable[pid] = i;
					processTable[i] = (PROCESSINFO*)malloc(sizeof(PROCESSINFO));
					break;
				}
			}	
			
		}
	}
	return processCount;	

}

void* setProcStatInfo(void* arg)
{
	pthread_mutex_lock(&lock);
	pthread_mutex_unlock(&lock);
	FILE *fp;
	char buf[LINEBUFSIZE];
	
	unsigned long before[CPU_USAGE] = {0 , };
	unsigned long now[CPU_USAGE];
	unsigned long result[CPU_USAGE];
	unsigned long total;

	while(1)
	{
		total = 0;
		if((fp = fopen("/proc/stat","r")) == NULL)
		{
			fprintf(stderr,"/proc/stat open error\n");
			pthread_exit(0);
		}
		int row = 0;
		while(row <= STAT_BTIME_ROW)
		{
			memset(buf,'\0',LINEBUFSIZE);
			fgets(buf,LINEBUFSIZE,fp);
			if(row ==  STAT_CPU_ROW){

				int tok = 0;
				char *ptr = strtok(buf," ");

				while(ptr != NULL)
				{
					switch(tok){
						case STAT_CPU_US : now[STAT_CPU_US-1] = atol(ptr); break;
						case STAT_CPU_SY : now[STAT_CPU_SY-1] = atol(ptr); break;
						case STAT_CPU_NI : now[STAT_CPU_NI-1] = atol(ptr); break;
						case STAT_CPU_ID : now[STAT_CPU_ID-1] = atol(ptr); break;
						case STAT_CPU_WA : now[STAT_CPU_WA-1] = atol(ptr); break;
						case STAT_CPU_HI : now[STAT_CPU_HI-1] = atol(ptr); break;
						case STAT_CPU_SI : now[STAT_CPU_SI-1] = atol(ptr); break;
						case STAT_CPU_ST : now[STAT_CPU_ST-1] = atol(ptr); break;
					}
					ptr = strtok(NULL," ");
					tok++;
				}
			}else if(row == STAT_BTIME_ROW){
				
				int tok = 0;
				char *ptr = strtok(buf," ");

				while(ptr != NULL)
				{
					switch(tok){
						case STAT_BTIME_TOK : ps.btime = atol(ptr); break;
					}
					ptr = strtok(NULL," ");
					tok++;
				}
			}
			row++;	
		}
		for(int i = 0; i < CPU_USAGE; i++)
		{
			total = total + (now[i] - before[i]);
			result[i] = now[i] - before[i];
			before[i] = now[i];
		}
		if(total == 0) 
		{
			total = 1;
			for(int i = 0; i < CPU_USAGE; i++) result[i] = 0;
		}
		for(int i = 0; i < CPU_USAGE; i++)
		{
			switch(i+1){
				//us - time spent in user space
				case STAT_CPU_US : ps.us = (result[i]/(long double)total)*100; break;
				//sy - Time spent in kernel space
				case STAT_CPU_SY : ps.sy = (result[i]/(long double)total)*100; break;
				//ni - Time spent running niced user process(User defined priority)	
				case STAT_CPU_NI : ps.ni = (result[i]/(long double)total)*100; break;
				//id - time spent in idle operations
				case STAT_CPU_ID : ps.id = (result[i]/(long double)total)*100; break;
				//wa - Time spent on waiting on IO peripherals (ex DISK)
				case STAT_CPU_WA : ps.wa = (result[i]/(long double)total)*100; break;
				//hi - Time spent handling hardware interrupt routines
				// Whenever peripheral unit want attention from the CPU , to siganl the CPU 
				case STAT_CPU_HI : ps.hi = (result[i]/(long double)total)*100; break;
				//si - Time spent handling software interrupt routine
				case STAT_CPU_SI : ps.si = (result[i]/(long double)total)*100; break;
				//st - Time spent on involuntary waits by virtual cpu. 
				//while hypervisor is serving another processor(taken by VM)
				case STAT_CPU_ST : ps.st = (result[i]/(long double)total)*100; break;
			}

		}	
		fclose(fp);
		pthread_barrier_wait(&basicFileRead);
		pthread_barrier_wait(&basicTimeSynk);
	}	
}
unsigned long extractNum(char *line,int flag)
{
	if(flag == 0)
	{
		char *tmp = (char*)malloc(sizeof(char)*strlen(line));
		strcpy(tmp,line);	
		char *ptr = strtok(tmp,":");
		if(strcmp(ptr,"VmSize")) return 0;
	}else if(flag == 1){
		char *tmp = (char*)malloc(sizeof(char)*strlen(line));
		strcpy(tmp,line);
		char *ptr = strtok(tmp,":");
		if(strcmp(ptr,"VmHWM")) return 0;
	}else if(flag == 2){
		char *tmp = (char*)malloc(sizeof(char)*strlen(line));
		strcpy(tmp,line);
		char *ptr = strtok(tmp,":");
		if(strcmp(ptr,"RssFile")) return 0;
	}
	unsigned long tmp = 0;
	for(int i = 0; i <strlen(line); i++)
	{
		if(line[i] >= '0' && line[i] <= '9')
			tmp = tmp*10 + line[i] - '0';
	}
	return tmp;
}
void* setMemInfo(void* arg)
{
	char buf[LINEBUFSIZE];
	FILE *fp;

	unsigned long memData[MEM_DATAS];
	while(1)
	{
		if((fp = fopen("/proc/meminfo","r")) == NULL)
		{
			fprintf(stderr,"/proc/meminfo open error\n");
			pthread_exit(0);
		}
		int row = 0;
		while(row <= MEMINFO_SRECLAIM_ROW)
		{
			memset(buf,'\0',LINEBUFSIZE);
			fgets(buf,LINEBUFSIZE,fp);
			switch(row){
				case MEMINFO_TOTAL_ROW: memData[MEMINFO_TOTAL_IDX] = extractNum(buf,-1); break;
				case MEMINFO_FREE_ROW: memData[MEMINFO_FREE_IDX] = extractNum(buf,-1); break;
				case MEMINFO_AVAIL_ROW: memData[MEMINFO_AVAIL_IDX] = extractNum(buf,-1); break;
				case MEMINFO_BUFFER_ROW: memData[MEMINFO_BUFFER_IDX] = extractNum(buf,-1); break;
				case MEMINFO_CACHED_ROW: memData[MEMINFO_CACHED_IDX] = extractNum(buf,-1); break;
				case MEMINFO_SWAPTOTAL_ROW: memData[MEMINFO_SWAPTOTAL_IDX] = extractNum(buf,-1); break;
				case MEMINFO_SWAPFREE_ROW: memData[MEMINFO_SWAPFREE_IDX] = extractNum(buf,-1); break;
				case MEMINFO_SRECLAIM_ROW: memData[MEMINFO_SRECLAIM_IDX] = extractNum(buf,-1); break;
			}	
			row++;
		}

		// sreclaimable memory is used by kernel, but any need arise, it should be used for that need
		// proc/meminfo has kb data, so needed to convert to mib
		mem.total = (double)memData[MEMINFO_TOTAL_IDX]/1024;
		mem.free = (double)memData[MEMINFO_FREE_IDX]/1024;
		// mem currently in use (total-free-buffer-cache-Sreclaimable)
		mem.used = (double)(memData[MEMINFO_TOTAL_IDX] - memData[MEMINFO_FREE_IDX] -
					memData[MEMINFO_BUFFER_IDX]- memData[MEMINFO_CACHED_IDX] -
					memData[MEMINFO_SRECLAIM_IDX])/1024;
		// mem buffer + cached + sreclaimable mem
		mem.buf_cache = (double)(memData[MEMINFO_BUFFER_IDX]+memData[MEMINFO_CACHED_IDX]
						+memData[MEMINFO_SRECLAIM_IDX])/1024;
		// swap mem is DISK mem, when RAM is full, system can use it. It can raise stability of system.
		mem.swap_total =(double)memData[MEMINFO_SWAPTOTAL_IDX]/1024;
		mem.swap_free = (double)memData[MEMINFO_SWAPFREE_IDX]/1024;
		mem.swap_used = (double)(memData[MEMINFO_SWAPTOTAL_IDX] - memData[MEMINFO_SWAPFREE_IDX])/1024;

		mem.avail = (double)memData[MEMINFO_AVAIL_IDX]/1024;
		fclose(fp);
		pthread_barrier_wait(&basicFileRead);
		pthread_barrier_wait(&basicTimeSynk);
	}	
}


void* setloadavgInfo(void* arg)
{
	char buf[LINEBUFSIZE];
	FILE *fp;

	while(1)
	{
		if((fp = fopen("/proc/loadavg","r")) == NULL)
		{
			fprintf(stderr,"/proc/loadagv open error\n");
			pthread_exit(0);
		}
		//loadavg is agvarage execute / avarage wait processes,
		//At last 1min,5min,15min
		memset(buf,'\0',LINEBUFSIZE);
		fgets(buf,LINEBUFSIZE,fp);
		
		int tok = 0;		
		char *ptr = strtok(buf," ");
		while(tok <= LOADAVG_15MIN_TOK)
		{
			switch(tok)
			{
				case LOADAVG_1MIN_TOK : loadavg.min_1 = atof(ptr); break;
				case LOADAVG_5MIN_TOK : loadavg.min_5 = atof(ptr); break;
				case LOADAVG_15MIN_TOK : loadavg.min_15 = atof(ptr); break;
			}
			ptr = strtok(buf," ");
			tok++;
		}
		fclose(fp);
		pthread_barrier_wait(&basicFileRead);
		pthread_barrier_wait(&basicTimeSynk);
	}		
}




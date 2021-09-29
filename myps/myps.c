#include "mypsheader.h"

PROCESSINFO * procs;

unsigned int opt = 0;
static int numFilter(const struct dirent *dirent){
	if(isdigit(dirent->d_name[0])!=0) return 1;
	else return 0;
}

unsigned int getUpTime(void);
unsigned long extractNum(char *line,int flag);
unsigned int getUpTime(void);
void getTTY(char path[PATH_LEN], char tty[TTY_LEN]);
void initProc(PROCESSINFO *proc);

void getProcessInfo(PROCESSINFO *proc);
void scanProcDir(void);
void printMyPs(int processCount);



int main(int argc, char **argv)
{
	if(argc > 1){
	 for(int i = 0; i < strlen(argv[1]); i++)
    {
        switch(argv[1][i])
        {
            case 'a' : opt |= OPTION_A; break;
            case 'u' : opt |= OPTION_U; break;
            case 'x' : opt |= OPTION_X; break;
        }
    }
	}	
     scanProcDir();
}
void initProc(PROCESSINFO *proc)
{
	proc->pid = 0;
	memset(proc->userName, '\0', UNAME_LEN);
	proc->cpuUsage = 0.0;
	proc->memUsage = 0.0;
	proc->vsz = 0;
	proc->rss = 0;
	proc->shr = 0;
	proc->priority = 0;
	proc->ni = 0;
	memset(proc->tty, '\0', TTY_LEN);
	memset(proc->status, '\0', STAT_LEN);
	memset(proc->start, '\0', TIME_LEN);
	memset(proc->time, '\0', TIME_LEN);
	memset(proc->cmd, '\0', CMD_LEN);
	memset(proc->command, '\0', CMD_LEN);
	return;
}


unsigned long getMemTotal(void)
{
    FILE *fp;
    char buf[LINEBUFSIZE];
    unsigned long tmp = 0;
	if((fp = fopen("/proc/meminfo","r")) == NULL) fprintf(stderr,"/proc/meminfo open err\n");
	memset(buf,'\0',LINEBUFSIZE);
	fgets(buf,LINEBUFSIZE,fp);
	fclose(fp);
    tmp = extractNum(buf,-1);
    return tmp;
    
}
unsigned int getUpTime(void)
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

unsigned long extractNum(char *line,int flag)
{
	if(flag == EXTR_VMS)
	{
		char *tmp = (char*)malloc(sizeof(char)*strlen(line));
		strcpy(tmp,line);	
		char *ptr = strtok(tmp,":");
		if(strcmp(ptr,"VmSize")) return 0;
	}else if(flag == EXTR_VMH){
		char *tmp = (char*)malloc(sizeof(char)*strlen(line));
		strcpy(tmp,line);
		char *ptr = strtok(tmp,":");
		if(strcmp(ptr,"VmHWM")) return 0;
	}else if(flag == EXTR_RSS){
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

void getTTY(char path[PATH_LEN], char tty[TTY_LEN])
{
	char fdZeroPath[PATH_LEN];			//0번 fd에 대한 절대 경로
	memset(tty, '\0', TTY_LEN);
	memset(fdZeroPath, '\0', TTY_LEN);
	strcpy(fdZeroPath, path);
	strcat(fdZeroPath, "/fd/0");

	if(access(fdZeroPath, F_OK) < 0){//fd 0이 없을 경우

		char statPath[PATH_LEN];		// /proc/pid/stat에 대한 절대 경로
		memset(statPath, '\0', PATH_LEN);
		strcpy(statPath, path);
		strcat(statPath, "/stat");

		FILE *statFp;
		if((statFp = fopen(statPath, "r")) == NULL){	// /proc/pid/stat open
			fprintf(stderr, "fopen error %s\n",  statPath);
			sleep(1);
			return;
		}

		char buf[LINEBUFSIZE];
		for(int i = 0; i <= STAT_TTY_NR_IDX; i++){		// 7행까지 read해 TTY_NR 획득
			memset(buf, '\0', LINEBUFSIZE);
			fscanf(statFp, "%s", buf);
		}
		fclose(statFp);

		int ttyNr = atoi(buf);		//ttyNr 정수 값으로 저장

		DIR *dp;
		struct dirent *dentry;
		if((dp = opendir("/dev")) == NULL){		// 터미널 찾기 위해 /dev 디렉터리 open
			fprintf(stderr, "opendir error for /dev\n");
			exit(1);
		}
		char nowPath[PATH_LEN];

		while((dentry = readdir(dp)) != NULL){	// /dev 디렉터리 탐색
			memset(nowPath, '\0', PATH_LEN);	// 현재 탐색 중인 파일 절대 경로
			strcpy(nowPath, "/dev");
			strcat(nowPath, "/");
			strcat(nowPath, dentry->d_name);

			struct stat statbuf;
			if(stat(nowPath, &statbuf) < 0){	// stat 획득
				fprintf(stderr, "stat error for %s\n", nowPath);
				exit(1);
			}
			if(!S_ISCHR(statbuf.st_mode))		//문자 디바이스 파일이 아닌 경우 skip
				continue;
			else if(statbuf.st_rdev == ttyNr){	//문자 디바이스 파일의 디바이스 ID가 ttyNr과 같은 경우
				strcpy(tty, dentry->d_name);	//tty에 현재 파일명 복사
				break;
			}
		}
		closedir(dp);
		if(!strlen(tty))                    // /dev에서도 찾지 못한 경우
			strcpy(tty, "?");				//nonTerminal
	}
	else{
		char symLinkName[NAMEBUFSIZE];
		memset(symLinkName, '\0', NAMEBUFSIZE);
		if(readlink(fdZeroPath, symLinkName, NAMEBUFSIZE) < 0){
			fprintf(stderr, "readlink error for %s\n", fdZeroPath);
			exit(1);
		}
        //printf("%s\n",symLinkName);
		if(!strcmp(symLinkName, "/dev/null"))		//symbolic link로 가리키는 파일이 /dev/null일 경우
			strcpy(tty, "?");					//nonTerminal
		else
			sscanf(symLinkName, "/dev/%s", tty);	//그 외의 경우 tty 획득

	}
	return;
}

void scanProcDir(void)
{
	struct dirent **namelist;
	int processCount=0; //process개수
	char pathBuf[PATH_LEN];
    memset(pathBuf, '\0',PATH_LEN);//메모리 초기화
	sprintf(pathBuf, "%s", "/proc");//proc디렉토리 경로

	//get Directory whose name is numeric
	if((processCount = scandir(pathBuf, &namelist, *numFilter, alphasort))==-1){
		fprintf(stderr, "%s Directory Scan Error\n", pathBuf);
		exit(1);
	}

    procs = (PROCESSINFO*)malloc(sizeof(PROCESSINFO)*processCount);
    
    //init array
    for(int i = 0; i < processCount; i++){
         initProc(&procs[i]);
        procs[i].pid = atoi(namelist[i]->d_name);
       
        getProcessInfo(&procs[i]);
    }

    //sort by name
    for(int i=0; i<processCount-1; i++){
        for(int j=i+1; j<processCount;j++){
            if(procs[i].pid > procs[j].pid){
                PROCESSINFO tmp;
                tmp=procs[i];
                procs[i]=procs[j];
                procs[j]=tmp;
            } 
        }
    }
   printMyPs(processCount);
}

void getProcessInfo(PROCESSINFO *proc)
{
    char pathBuf[PATH_LEN];
	char buf[LINEBUFSIZE];
    
    //get tty
    memset(pathBuf,'\0',PATH_LEN);
    sprintf(pathBuf,"/proc/%d",proc->pid);
    getTTY(pathBuf,proc->tty);

    

    FILE *statFp;
    memset(pathBuf,'\0',PATH_LEN);
    sprintf(pathBuf,"/proc/%d/stat",proc->pid);

    //get user name wiith uid
    char userNameBuf[NAMEBUFSIZE];
	struct stat statbuf;
	stat(pathBuf,&statbuf);
	struct passwd* up = getpwuid(statbuf.st_uid);
	strcpy(userNameBuf,up->pw_name);
	if(strlen(userNameBuf) > 8){
		userNameBuf[7] = '+';
		for(int i = 8; i < strlen(userNameBuf); i++)
		{
			userNameBuf[i] = '\0';
		}
	}
	strcpy(proc->userName,userNameBuf);

    if((statFp = fopen(pathBuf,"r")) == NULL)
	{
        //If statfile is not open,Finish without having to read more.
        //It will be filtered when i print information.
        proc->pid = -1;
        return;
    }

    memset(buf,'\0',LINEBUFSIZE);
	fgets(buf,LINEBUFSIZE,statFp);
	fclose(statFp);

    //for %cpu, time+ calculation
    unsigned long utime,stime,startTime;
	int hertz = (int)sysconf(_SC_CLK_TCK);

    //for status+
    int sid,tpgid,threadNum;
    //get information from /proc/pid/stat
    int tok = 0;
	char *ptr = strtok(buf," ");
	while(ptr != NULL)
	{
		switch(tok){
			case PIDSTAT_STATE_TOK : memset(proc->status,'\0',STAT_LEN); 
                                    proc->status[0] = *ptr; break;
			case PIDSTAT_CMD_TOK : extractCmd(proc->cmd,ptr); break;
            case PIDSTAT_SID_TOK : sid = atoi(ptr); break;
            case PIDSTAT_TPGID_TOK : tpgid = atoi(ptr); break;
			case PIDSTAT_UTIME_TOK : utime = atoll(ptr); break;
			case PIDSTAT_STIME_TOK : stime = atoll(ptr); break;
			case PIDSTAT_PR_TOK : proc->priority = atoi(ptr); break;
			case PIDSTAT_NI_TOK : proc->ni = atoi(ptr); break;
            case PIDSTAT_THREAD_TOK : threadNum = atoi(ptr); break;
			case PIDSTAT_STARTTIME_TOK : startTime = atol(ptr); break;
	    }
		tok++;
		ptr = strtok(NULL," ");
	}

    FILE *statusFp;
    memset(pathBuf,'\0',PATH_LEN);
    sprintf(pathBuf,"/proc/%d/status",proc->pid);
    if((statusFp = fopen(pathBuf,"r")) == NULL)
    {
        //If status file is not open,Finish without having to read more.
        //It will be filtered when i print information.
        proc->pid = -1;
        return;
    }
	int row = 0;
	while(row <= PIDSTATUS_SHR_ROW)
	{
        //prevent segfault
	    if(statusFp != NULL){
			memset(buf,'\0',LINEBUFSIZE);
			fgets(buf,LINEBUFSIZE,statusFp);
			switch(row){
				case PIDSTATUS_VMRT_ROW : proc->vsz = extractNum(buf,EXTR_VMS); break;
				case PIDSTATUS_RES_ROW :proc->rss = extractNum(buf,EXTR_VMH); break;
				case PIDSTATUS_SHR_ROW : proc->shr = extractNum(buf,EXTR_RSS); break;
			}
		}
		row++;
	}
	if(statusFp != NULL) 
		fclose(statusFp);

    FILE *cmdLineFp;
    memset(pathBuf,'\0',PATH_LEN);
    sprintf(pathBuf,"/proc/%d/cmdline",proc->pid);
	if((cmdLineFp = fopen(pathBuf, "r")) == NULL){
		//If statfile is not open,Finish without having to read more.
        //It will be filtered when i print information.
        proc->pid = -1;
		return;
	}

	while(1){
		char c[2] = {'\0', '\0'};
		fread(&c[0], 1, 1, cmdLineFp);
		if(c[0] == '\0'){					
			fread(&c[0], 1, 1, cmdLineFp);
			if(c[0] == '\0')
				break;
			else {
				strcat(proc->command, " ");
			}
		}
		strcat(proc->command, c);
	}
	if(!strlen(proc->command))				//cmdline에서 읽은 문자열 길이 0일 경우
		sprintf(proc->command, "[%s]", proc->cmd);	// [cmd]로 채워기
	fclose(cmdLineFp);

    //STATUS
    if(proc->ni < 0)
        proc->status[strlen(proc->status)] = '<';
    if(proc->ni > 0)
        proc->status[strlen(proc->status)] = 'N';
    if(proc->vsz == 0)
        proc->status[strlen(proc->status)] = 'L';
    if(sid == proc->pid) //session leader
        proc->status[strlen(proc->status)] = 's';
    if(threadNum > 1)
        proc->status[strlen(proc->status)] = 'l';
    if(tpgid == proc->pid) // is in the foregrount process group
        proc->status[strlen(proc->status)] = '+';
    
    
   
    unsigned long memTotal = getMemTotal();
    unsigned long uptime = getUpTime();
	proc->cpuUsage =((double)(utime + stime)/hertz) / (uptime-((double)startTime/hertz)) * 100;
	proc->memUsage = ((double) proc->rss / memTotal) *100;

    //START 
	unsigned long start = time(NULL) - uptime + (startTime/hertz);
	struct tm *tmStart= localtime(&start);
	if(time(NULL) - start < 24 * 60 * 60){
		strftime(proc->start, TIME_LEN, "%H:%M", tmStart);
	}
	else if(time(NULL) - start < 7 * 24 * 60 * 60){
		strftime(proc->start, TIME_LEN, "%b %d", tmStart);
	}
	else{
		strftime(proc->start, TIME_LEN, "%y", tmStart);
	}

	//TIME
    unsigned long long totalTime = utime + stime;
	unsigned long cpuTime = totalTime / hertz;
	struct tm *tmCpuTime= localtime(&cpuTime);

    //if there is no option
	if(!(opt & OPTION_A) && !(opt & OPTION_X) && !(opt & OPTION_U) )
		sprintf(proc->time, "%02d:%02d:%02d", tmCpuTime->tm_min/60, tmCpuTime->tm_min, tmCpuTime->tm_sec);
	else
		sprintf(proc->time, "%1d:%02d", tmCpuTime->tm_min, tmCpuTime->tm_sec);
}

void printMyPs(int processCount)
{ 
   // a option Print process connected with terminal
    if((opt & OPTION_A))
    {
        for(int i = 0; i < processCount; i++)
        {
            if(procs[i].pid != -1 && strcmp(procs[i].tty,"?"))
            {
                procs[i].printFlag = 1;
            }
        }
	}
	// x option, Print process username is same with user
   if(opt & OPTION_X)
    {
		//if ax option print all porcess
		if(opt & OPTION_A){
			for(int i = 0; i < processCount; i++)
			{
				procs[i].printFlag = 1;
			}
		}else{
			char pathBuf[PATH_LEN];
			 memset(pathBuf,'\0',PATH_LEN);
   			 sprintf(pathBuf,"/proc/%d/stat",getpid());		
			char userNameBuf[NAMEBUFSIZE];
			struct stat statbuf;
			stat(pathBuf,&statbuf);
			struct passwd* up = getpwuid(statbuf.st_uid);
			strcpy(userNameBuf,up->pw_name);
			
			for(int i = 0; i < processCount; i++)
			{
				if(procs[i].pid != -1 && !strcmp(procs[i].userName,userNameBuf))
				{
					procs[i].printFlag = 1;
				}
			}
		}
	}
	if(opt & OPTION_U)
	{
		//if option has only u
		if(!(opt & OPTION_A) && !(opt & OPTION_X))
		{
			char pathBuf[PATH_LEN];
			 memset(pathBuf,'\0',PATH_LEN);
   			 sprintf(pathBuf,"/proc/%d/stat",getpid());		
			char userNameBuf[NAMEBUFSIZE];
			struct stat statbuf;
			stat(pathBuf,&statbuf);
			struct passwd* up = getpwuid(statbuf.st_uid);
			strcpy(userNameBuf,up->pw_name);
			printf("%s\n",userNameBuf);
			for(int i = 0; i < processCount; i++)
			{
				if(procs[i].pid != -1 && !strcmp(procs[i].userName,userNameBuf)&& strcmp(procs[i].tty,"?"))
				{
					procs[i].printFlag = 1;
				}
			}
		}
	}

	if(!(opt & OPTION_U) && !(opt & OPTION_U) && !(opt & OPTION_U))
	{
		char *ret, tty[TTY_LEN];
		memset(tty, '\0', TTY_LEN);
		if((ret = ttyname(STDERR_FILENO))==NULL){
			fprintf(stderr, "ttyname() error\n");
			exit(1);
		}
		strcpy(tty, ret+5);
		for(int i = 0; i < processCount; i++)
        {
            if(procs[i].pid != -1 && !strcmp(procs[i].tty,tty))
            {
                procs[i].printFlag = 1;
            }
        }
	}

	struct winsize size;

	if (ioctl(0, TIOCGWINSZ, (char *)&size) < 0){
		fprintf(stderr, "ioctl error\n");
		exit(1);
	}
	int w_col = size.ws_col;
	char printBuf[LINEBUFSIZE];
	if(opt & OPTION_U)
	{
		memset(printBuf,'\0',LINEBUFSIZE);
		snprintf(printBuf,w_col,"%-10s%5s%5s%5s%8s%7s %-9s%-5s%7s%7s %-8s", "USER", "PID", "%CPU", "%MEM" ,
								"VSZ", "RSS","TTY","STAT", "START", "TIME", "COMMAND");
		printf("%s\n",printBuf);

		for(int i = 0; i < processCount; i++)
		{
			if(procs[i].printFlag == 1){
				memset(printBuf,'\0',LINEBUFSIZE);
				snprintf(printBuf,w_col,"%-10s%5d%5.1Lf%5.1Lf%8ld%7ld %-9s%-5s%7s%7s %-8s", 
									procs[i].userName,procs[i].pid,procs[i].cpuUsage,procs[i].memUsage,
									procs[i].vsz,procs[i].rss,procs[i].tty,procs[i].status,procs[i].start,procs[i].time,procs[i].cmd);

				printf("%s\n",printBuf);
			}
		}
	}
	else if( (opt & OPTION_A) || (opt &OPTION_X))
	{
		memset(printBuf,'\0',LINEBUFSIZE);
		snprintf(printBuf,w_col,"%5s %-9s%-5s%7s %-8s","PID", "TTY","STAT", "TIME", "COMMAND");
		printf("%s\n",printBuf);
		for(int i = 0; i < processCount; i++)
		{
			if(procs[i].printFlag == 1){
				memset(printBuf,'\0',LINEBUFSIZE);
				snprintf(printBuf,w_col,"%5d %-9s%-5s%7s %-8s", 
									procs[i].pid,procs[i].tty,procs[i].status,procs[i].time,procs[i].command);

				printf("%s\n",printBuf);
			}
		}

	}
	else
	{
		memset(printBuf,'\0',LINEBUFSIZE);
		snprintf(printBuf,w_col,"%5s %-9s%7s %-8s","PID", "TTY", "TIME", "COMMAND");
		printf("%s\n",printBuf);
		for(int i = 0; i < processCount; i++)
		{
			if(procs[i].printFlag == 1){
				memset(printBuf,'\0',LINEBUFSIZE);
				snprintf(printBuf,w_col,"%5d %-9s%7s %-8s", 
									procs[i].pid,procs[i].tty,procs[i].time,procs[i].command);

				printf("%s\n",printBuf);
			}
		}

	}
}

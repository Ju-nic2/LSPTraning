#include "lscpuheader.h"

struct lscpu_type type;
struct counter count;
struct lscpu_cpu cpuList;
struct cpu_cache cpuCache;
int * onlinelist;

char* getStr(char *ptr,int flag)
{
    char *tmp;
    if(flag == CUT) {ptr = strtok(ptr,":"); ptr = strtok(NULL,"");}
    tmp = (char*)malloc(sizeof(char)*(strlen(ptr)+1));
    strcpy(tmp,ptr);
    return tmp;
}

int extractNum(char *ptr)
{
    char *str1 = (char*)malloc(sizeof(char)*strlen(ptr)+2);
    strcpy(str1,ptr);
	int tmp = 0;
	for(int i = 0; i <strlen(str1); i++)
	{
		if(str1[i] >= '0' && str1[i] <= '9')
			tmp = tmp*10 + str1[i] - '0';
	}

	return tmp;
}

int* extractNumFromList(char *ptr)
{
    int count = 1;
    int tmp = 0;
    for(int i = 0; i < strlen(ptr); i++)
    {
        if(ptr[i] == '-')count++;
    }
    int *arr = malloc(sizeof(int)*count);

    if(count == 1)
    {
        arr[0] = 0;
        arr[1] = 0;
        return arr;
    }

    count = 0;
    for(int i = 0; i < strlen(ptr); i++)
    {
        if(ptr[i] == '-'){arr[count] = tmp; tmp = 0; count++;}
        else if(ptr[i] >= '0' && ptr[i] <= '9')
        {
            tmp = tmp*10 + ptr[i] - '0';
        }
    }
    arr[count] = tmp;
    return arr;
}

char * makePath(char *prefix,int num,char * suffix);

void setEndian();
void initCounter();
void getArchitecture();
void getProcessorInfo();
void getNUMAnode();
void getCpuNode();
void getcache();
void getvulnerabilities();
void parseFlag();
void printLscpu();

void parseArgv();

int main(int argc, char ** argv)
{
    setEndian();
    initCounter();
    getArchitecture();
    getProcessorInfo();
    getNUMAnode();
    getCpuNode();
    getcache();
    getvulnerabilities();
    parseFlag();
    if(argc>= 2)
        parseArgv(argc,argv);

    printLscpu();   
}

char *makePath(char *prefix,int num,char * suffix)
{
    //number limit is length of max integer in (int)
	char numbuffer[11] = {'\0',};
	//only positive number can write in file name
	if(num >= 0)
		sprintf(numbuffer,"%d",num);

    char *path = (char*)malloc(sizeof(char) * (strlen(prefix) + strlen(suffix) + 1));
    strncpy(path,prefix,strlen(prefix));
	strncpy(&path[strlen(prefix)],numbuffer,strlen(numbuffer));
	strncpy(&path[strlen(prefix)+strlen(numbuffer)],suffix,strlen(suffix));
	path[strlen(prefix)+strlen(numbuffer)+strlen(suffix)] = '\0';
    return path;
}

void parseArgv(int argc, char **argv)
{
    if(argc < 2){
        fprintf(stderr,"please write -e + (-a | NODE name)\n");
        exit(1);
    }

    char *colum[6] = {"CPU","SOCKET","CORE","NODE","L1d:L1i:L2:L3","ONLINE"};
    int printList[6] = {1,1,1,1,1,1};
    if(!strcmp(argv[1],"-e"))
    {
        if(!strcmp(argv[2],"-a")){
            if(type.nptr == NULL) printList[NODE] = 0;
            for(int i = 0; i < 6; i++)
            {
                if(printList[i] == 1)
                    printf("%-s ",colum[i]);
            }
            printf("\n");
            for(int i = 0; i < cpuList.count; i++)
            {
                for(int j = 0; j < 6; j++)
                {
                    if(printList[j] == 1)
                    {
                        switch(j)
                        {
                            case CPU : printf("%-3d ",cpuList.node[i].logical_id);break;
                            case SOCKET : printf("%-5d ",cpuList.node[i].socket);break;
                            case CORE : printf("%-4d ",cpuList.node[i].core);break;
                            case NODE : printf("%-4d ",cpuList.node[i].node);break;
                            case CACHE : printf("%-1d:%-1d:%-1d:%-1d   ",cpuList.node[i].cache[L1d],cpuList.node[i].cache[L1i],
                            cpuList.node[i].cache[L2],
                            cpuList.node[i].cache[L3]);break;
                            case ONLINE : printf("%-7s ",(cpuList.node[i].online == 1) ? "YES": "NO");break;
                        }
                    }
                   
                }
                 printf("\n");
    
            }   
        }
    }else
    {
        for(int i = 0; i < 6; i++)
            printList[i] = 0;
        char *ptr = strtok(argv[1],"=");
        if(!strcmp(ptr,"-e"))
        {
            ptr = strtok(NULL,",");
            while (ptr != NULL){
                if(!strcmp(ptr,"CPU")) printList[CPU] = 1;
                else if(!strcmp(ptr,"NODE"))  printList[NODE] = 1;
                else if(!strcmp(ptr,"SOCKET"))  printList[SOCKET] = 1;
                else if(!strcmp(ptr,"CORE"))  printList[CORE] = 1;
                else if(!strcmp(ptr,"CACHE"))  printList[CACHE] = 1;
                else if(!strcmp(ptr,"ONLINE"))  printList[ONLINE] = 1;
				else
				{
					fprintf(stderr,"worng colum name\n");
					exit(1);
				}
                ptr = strtok(NULL,",");
            }

             for(int i = 0; i < 6; i++)
            {
                if(printList[i] == 1)
                    printf("%-s ",colum[i]);
            }
             printf("\n");
            for(int i = 0; i < cpuList.count; i++)
            {
                for(int j = 0; j < 6; j++)
                {
                    if(printList[j] == 1)
                    {
                        switch(j)
                        {
                            case CPU : printf("%-3d ",cpuList.node[i].logical_id);break;
                            case SOCKET : printf("%-5d ",cpuList.node[i].socket);break;
                            case CORE : printf("%-4d ",cpuList.node[i].core);break;
                            case NODE : printf("%-4d ",(type.nptr == NULL) ? : cpuList.node[i].node);break;
                            case CACHE : printf("%-1d:%-1d:%-1d:%-1d   ",cpuList.node[i].cache[L1d],cpuList.node[i].cache[L1i],
                            cpuList.node[i].cache[L2],
                            cpuList.node[i].cache[L3]);break;
                            case ONLINE : printf("%-7s ",(cpuList.node[i].online == 1) ? "YES": "NO");break;
                        }
                    }
                   
                }
                 printf("\n");
    
            }   
        }
		else{
			fprintf(stderr,"worng colum name\n");
			exit(1);
		}

    }
    exit(1);
}
void initCounter()
{
    count.cpuCount = 0;
    count.threadPerCore = 0;
    count.corePerSocket = 0;
    count.socketCount = 0;
    count.onLineList = NULL;
    cpuCache.L1d = NULL;
    cpuCache.L1i = NULL;
    cpuCache.L2 = NULL;
    cpuCache.L3 = NULL;
}

void setEndian()
{
    union {
        uint32_t i;
        char c[4];
    } e = { 0x01000000 };
    switch(e.c[0])
    {
        case LITTLE_EDIAN : type.byteOrder = getStr(byte_order[LITTLE_EDIAN],UNCUT);break;
        case BIG_EDIAN : type.byteOrder = getStr(byte_order[BIG_EDIAN],UNCUT);break;
    }
}
void getArchitecture()
{
    struct utsname un;
    uname(&un);
    type.Architecture = (char*)malloc(sizeof(char)*strlen(un.machine)+1);
    strcpy(type.Architecture,un.machine);
}
void getProcessorInfo()
{
    FILE *fp;
    char buf[LINEBUFSIZE];
    int recent_physical_id = -1;

    if((fp = fopen(path_proc_cpuinfo,"r")) == NULL)
    {
        fprintf(stderr,"/proc/cpuinfo error\n");
        exit(1);
    }

    int row = 0;
    int isfirst = 0;

    memset(buf,'\0',LINEBUFSIZE);
    fgets(buf,LINEBUFSIZE,fp);

    while(buf[0] != '\0')
    {
        //read info only first 
        if(isfirst == 0){
            switch(row)
            {
                case CPUINFO_PROCESSOR :count.cpuCount++; break;
                case CPUDINFO_VENDORID : type.vendorID = getStr(buf,CUT);break;
                case CPUINFO_CPUFAMILY : type.cpuFamily = getStr(buf,CUT);break;
                case CPUINFO_CPUMODEL : type.model = getStr(buf,CUT);break;
                case CPUINFO_MODELNAME : type.modelName = getStr(buf,CUT);break;
                case CPUINFO_STEPPING : type.stepping = getStr(buf,CUT);break;
                case CPUINFO_CPUMHZ : type.cpuMHz = getStr(buf,CUT);break;
                case CPUINFO_PYSICALID : count.socketCount++; recent_physical_id = extractNum(buf); break;

                case CPUINFO_CPUCORES : count.corePerSocket = extractNum(buf);  break;
                case CPUINFO_FLAGS : type.flag = getStr(buf,CUT);break;
                case CPUINFO_BOBOMIPS :type.BogoMIPS = getStr(buf,CUT);break;
                case CPUINFO_ADDRESS :type.addressSize = getStr(buf,CUT);break;
                case CPUINFO_END : isfirst = 1; row = -1; break;          
            }
        }
        else
        {
            switch(row)
            {
                case CPUINFO_PROCESSOR : count.cpuCount++; break;
                case CPUINFO_PYSICALID : (recent_physical_id == extractNum(buf)) ?  : count.socketCount++; break;
                case CPUINFO_END : row = -1; break;          
            }
        }
        memset(buf,'\0',LINEBUFSIZE);
        fgets(buf,LINEBUFSIZE,fp);
        row++;
    }

    count.threadPerCore = (count.cpuCount * count.socketCount) / count.corePerSocket;
    fclose(fp);
}

void getNUMAnode()
{
    char buf[LINEBUFSIZE];
    FILE *fp;
    char *onlinePath = makePath(path_sys_node,-1,"/online");

    if((fp = fopen(onlinePath,"r")) == NULL)
    {
		printf("%s",onlinePath);
        type.nptr = NULL;
        return;
    }
    memset(buf,'\0',LINEBUFSIZE);
    fgets(buf,LINEBUFSIZE,fp);
    int *onlineList = extractNumFromList(buf);

    type.nptr = (struct numa*)malloc(sizeof(struct numa));
    type.nptr->onlineCount = onlineList[1] - onlineList[0] + 1;
    type.nptr->node = (struct numa_node*)malloc(sizeof(struct numa_node)*type.nptr->onlineCount);

    for(int i = 0; i < type.nptr->onlineCount; i++)
    {
        FILE *tmp;
        char *nodePath = makePath(path_sys_node2,i,"/cpulist");
        tmp = fopen(nodePath,"r");

        type.nptr->node[i].name = makePath("node",i,"");
        memset(buf,'\0',LINEBUFSIZE);
		fgets(buf,LINEBUFSIZE,tmp);
        type.nptr->node[i].cpulist = extractNumFromList(buf);
        free(nodePath);
    }
}

void getCpuNode()
{
    char buf[LINEBUFSIZE];
    
    /*possible cpu is cpus that have been allocated resources and can be
	brought online if they are present.*/
    FILE *fp;
    char *possiblePath = makePath(path_sys_cpu,-1,"/possible");
    if((fp = fopen(possiblePath,"r")) == NULL)
    {
        return;
    }
    memset(buf,'\0',LINEBUFSIZE);
    fgets(buf,LINEBUFSIZE,fp);

    int *possibleList = extractNumFromList(buf);
    cpuList.count = possibleList[1] - possibleList[0] + 1;

    cpuList.node = (struct cpu_node*)malloc(sizeof(struct cpu_node)*cpuList.count);
    free(possiblePath);

    
    char *onlinePath = makePath(path_sys_cpu,-1,"/online");
    if((fp = fopen(onlinePath,"r")) == NULL)
    {
        return;
    }
    memset(buf,'\0',LINEBUFSIZE);
    fgets(buf,LINEBUFSIZE,fp);

    onlinelist = extractNumFromList(buf);

    free(onlinePath);
    
    for(int i = 0; i < cpuList.count; i++)
    {
        cpuList.node[i].logical_id = i;

        if(i >=  onlinelist[0] && i <=  onlinelist[1])
            cpuList.node[i].online = 1;
        if(type.nptr == NULL)
            cpuList.node[i].node = -1;
        
        char *cpu_topology_pid = makePath(path_sys_cpu2,i,"/topology/physical_package_id");
        if((fp = fopen(cpu_topology_pid,"r")) == NULL)
        {
            return;
        }
        memset(buf,'\0',LINEBUFSIZE);
        fgets(buf,LINEBUFSIZE,fp);

        cpuList.node[i].socket = extractNum(buf); 
        free(cpu_topology_pid);

        char *cpu_topology_cid = makePath(path_sys_cpu2,i,"/topology/core_id");
        if((fp = fopen(cpu_topology_cid,"r")) == NULL)
        {
            return;
        }
        memset(buf,'\0',LINEBUFSIZE);
        fgets(buf,LINEBUFSIZE,fp);
        if(fp != NULL)
        cpuList.node[i].core = extractNum(buf);
        free(cpu_topology_cid);
    }
}
int getNum(int fd)
{
    char ch;
	int tmp = 0;
	while(read(fd,&ch,1) > 0)
	{
		if(ch >= '0' && ch <= '9')
			tmp = tmp*10 + ch - '0';
	}
    return tmp;
}
void getcache()
{
    char pathbuf[SMALLBUFSIZE];
    int firstScan[4] = {1,1,1,1};

    for(int i = 0; i < cpuList.count; i++)
    {
       for(int j = 0; j < 4; j++)
       {
           int fd;
           memset(pathbuf,'\0',SMALLBUFSIZE);
           sprintf(pathbuf,"%s%d/cache/index%d/level",path_sys_cpu2,i,j);
            if((fd = open(pathbuf,O_RDONLY)) == -1)
            {
                //there is no index#
                continue;
            }
           int level = getNum(fd);
           close(fd);
           

            if(level == 1)
            {
                char typebuf[BUFSIZE];
                memset(pathbuf,'\0',SMALLBUFSIZE);
                sprintf(pathbuf,"%s%d/cache/index%d/type",path_sys_cpu2,i,j);
                if((fd = open(pathbuf,O_RDONLY)) == -1)
                {}
                memset(typebuf,'\0',BUFSIZE);
                {
                    char ch; int count = 0;
                    while(read(fd,&ch,1) > 0)
                    {
                        if(ch == '\n') break;
                        typebuf[count++] = ch;
                    }
                }
                close(fd);

                memset(pathbuf,'\0',SMALLBUFSIZE);
                sprintf(pathbuf,"%s%d/cache/index%d/id",path_sys_cpu2,i,j);
                if((fd = open(pathbuf,O_RDONLY)) == -1)
                {}
                int id = getNum(fd);
                close(fd);

                char sizeBuf[BUFSIZE];
                memset(sizeBuf,'\0',BUFSIZE);
                memset(pathbuf,'\0',SMALLBUFSIZE);
                sprintf(pathbuf,"%s%d/cache/index%d/size",path_sys_cpu2,i,j);
                
                if((fd = open(pathbuf,O_RDONLY)) == -1)
                {}
                {
                    char ch; int count = 0;
                    while(read(fd,&ch,1) > 0)
                    {
                        if(ch == 'K') break;
                        sizeBuf[count++] = ch;
                    }
                }
                close(fd);

                if(!strcmp(typebuf,"Instruction"))
                {
                    
                    cpuList.node[i].cache[L1i] = id;
                    if(firstScan[L1i]) {cpuCache.L1i = getStr(sizeBuf,UNCUT); firstScan[L1i] = 0;}
                }else if(!strcmp(typebuf,"Data"))
                {
                    cpuList.node[i].cache[L1d] = id;
                     if(firstScan[L1d]) {cpuCache.L1d = getStr(sizeBuf,UNCUT); firstScan[L1d] = 0;}
                }            
            }else if(level == 2)
            {
                memset(pathbuf,'\0',SMALLBUFSIZE);
                sprintf(pathbuf,"%s%d/cache/index%d/id",path_sys_cpu2,i,j);
                if((fd = open(pathbuf,O_RDONLY)) == -1)
                {}
                int id = getNum(fd);
                close(fd);

                char sizeBuf[BUFSIZE];
                memset(sizeBuf,'\0',BUFSIZE);
                memset(pathbuf,'\0',SMALLBUFSIZE);
                sprintf(pathbuf,"%s%d/cache/index%d/size",path_sys_cpu2,i,j);             
                if((fd = open(pathbuf,O_RDONLY)) == -1)
                {}
                {
                    char ch; int count = 0;
                    while(read(fd,&ch,1) > 0)
                    {
                        if(ch == 'K') break;
                        sizeBuf[count++] = ch;
                    }
                }
                close(fd);


                cpuList.node[i].cache[L2] = id;
                if(firstScan[L2]) {cpuCache.L2 = getStr(sizeBuf,UNCUT); firstScan[L2] = 0;}
                    
            }else if(level == 3)
            {  
                 memset(pathbuf,'\0',SMALLBUFSIZE);
                sprintf(pathbuf,"%s%d/cache/index%d/id",path_sys_cpu2,i,j);
                if((fd = open(pathbuf,O_RDONLY)) == -1)
                {}
                int id = getNum(fd);
                close(fd);

                char sizeBuf[BUFSIZE];
                memset(sizeBuf,'\0',BUFSIZE);
                memset(pathbuf,'\0',SMALLBUFSIZE);
                sprintf(pathbuf,"%s%d/cache/index%d/size",path_sys_cpu2,i,j);             
                if((fd = open(pathbuf,O_RDONLY)) == -1)
                {}
                {
                    char ch; int count = 0;
                    while(read(fd,&ch,1) > 0)
                    {
                        if(ch == 'K') break;
                        sizeBuf[count++] = ch;
                    }
                }
                close(fd);

                
                cpuList.node[i].cache[L3] = id;
                if(firstScan[L3]) {cpuCache.L3 = getStr(sizeBuf,UNCUT); firstScan[L3] = 0;}
            }
       }
    }
}

void parseFlag()
{
    if(strstr(type.flag,"vmx") != NULL)
        type.virtualTayp = getStr(virtual_type[VMX],UNCUT);
    else if(strstr(type.flag,"svm ") != NULL)
        type.virtualTayp = getStr(virtual_type[SVM],UNCUT);
    else if(strstr(type.flag,"vme") != NULL)
        type.virtualTayp = getStr(virtual_type[VME],UNCUT);
    
    if(strstr(type.flag,"lm") != NULL)
        type.opmode = getStr(op_modes[LONG_MODE],UNCUT);
    else if(strstr(type.flag,"thumb") != NULL)
        type.opmode = getStr(op_modes[THUMB],UNCUT);

}

void getvulnerabilities()
{
    char pathbuf[SMALLBUFSIZE];
    char linebuf[LINEBUFSIZE];

    memset(pathbuf,'\0',SMALLBUFSIZE);
    sprintf(pathbuf,"%s/vulnerabilities/",path_sys_cpu);

    for(int i = 0; i < 9; i++)
    {
        type.vulnerabilities[i] = NULL;
    }

    DIR *dir = NULL;
    struct dirent *d = NULL;

    if((dir = opendir(pathbuf)) == NULL)
    {return;}

    
    while((d = readdir(dir)) != NULL)
    {
        if(!strcmp(d->d_name,"itlb_multihit"))
        {
            
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                continue;
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[ITLB_MYLTIHIT] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
					type.vulnerabilities[ITLB_MYLTIHIT][count++] = ch;
				 }
            }
            close(fd);
        }

       else  if(!strcmp(d->d_name,"l1tf"))
        {
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                continue;
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[L1TF] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
					type.vulnerabilities[L1TF][count++] = ch;
				 }
            }
            close(fd);
        }

       else  if(!strcmp(d->d_name,"mds"))
        {
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                continue;
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[MDS] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
                     type.vulnerabilities[MDS][count++] = ch;
				 }
            }
            close(fd);
        }

       else  if(!strcmp(d->d_name,"meltdown"))
        {
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                continue;
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[MELTDOUN] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
					type.vulnerabilities[MELTDOUN][count++] = ch;
				 }
            }
            close(fd);
        }

        else if(!strcmp(d->d_name,"spec_store_bypass"))
        {
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                continue;
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[SPEC_STORE_BYPASS] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
					type.vulnerabilities[SPEC_STORE_BYPASS][count++] = ch;
				 }
            }
            close(fd);
        }

        else if(!strcmp(d->d_name,"spectre_v1"))
        {
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                continue;
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[SPECTRE_V1] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
					type.vulnerabilities[SPECTRE_V1][count++] = ch;
				 }
            }
            close(fd);
        }
        
        else if(!strcmp(d->d_name,"spectre_v2"))
        {
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                continue;
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[SPECTRE_V2] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
					type.vulnerabilities[SPECTRE_V2][count++] = ch;
				 }
            }
            close(fd);
        }
        else if(!strcmp(d->d_name,"srbds"))
        {
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                continue;
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[SRBDS] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
					type.vulnerabilities[SRBDS][count++] = ch;
				 }
            }
            close(fd);
        }
        else if(!strcmp(d->d_name,"tsx_async_abort"))
        {
            memset(pathbuf,'\0',SMALLBUFSIZE);
            sprintf(pathbuf,"%s/vulnerabilities/%s",path_sys_cpu,d->d_name);
            int fd;
            if((fd = open(pathbuf,O_RDONLY)) == -1)
                {continue;}
            {
                char ch; int count = 0;
                while(read(fd,&ch,1) > 0) count++;
                type.vulnerabilities[TSX_ASYNC_ABORT] = (char*)malloc(sizeof(char)*count+1);
				lseek(fd,0,SEEK_SET);
				count = 0;
				 while(read(fd,&ch,1) > 0)
				 {
					type.vulnerabilities[TSX_ASYNC_ABORT][count++] = ch;
				 }
            }
            close(fd);
        }
    }
    

    


}

void printLscpu()
{
    printf("Architecture:          %s\n",type.Architecture);
    printf("CPU op-mode(s):        %s\n",type.opmode);
    printf("ByteOrder:             %s\n",type.byteOrder);
    printf("Address Size:         %s",type.addressSize);
    printf("CPU(s)                 %d\n",count.cpuCount);
    printf("On-line CPU(s) list :  %d-%d\n",onlinelist[0],onlinelist[1]);
    printf("Thread(s) per core:    %d\n",(int)count.threadPerCore);
    printf("Core(s) per socket:    %d\n",count.corePerSocket);
    printf("Socket:                %d\n",count.socketCount);
	if(type.nptr != NULL)
		printf("NUMA nodes(s):         %d\n",type.nptr->onlineCount);
    printf("Vendor ID:            %s",type.vendorID);
    printf("CPU family:           %s",type.cpuFamily);
    printf("Model:                %s",type.model);
    printf("Model name:           %s",type.modelName);
    printf("Stepping:             %s",type.stepping);
    printf("CPU MHz:              %s",type.cpuMHz);
    printf("BogoMIPS:             %s",type.BogoMIPS);
    printf("Virtualization type:   %s\n",type.virtualTayp);
    printf("L1d cache:             %d Kib\n",atoi(cpuCache.L1d) * count.corePerSocket);
    printf("L1i cache:             %d\ Kib\n",atoi(cpuCache.L1i)* count.corePerSocket);
    if(atoi(cpuCache.L2) * count.corePerSocket >= 1024){
		if((atoi(cpuCache.L2) * count.corePerSocket) % 1024 == 0)
    		printf("L2 cache:              %d Mib\n ",(atoi(cpuCache.L2) * count.corePerSocket)/1024);
    	else		
			printf("L2 cache:              %.1f Mib\n ",(float)(atoi(cpuCache.L2) * count.corePerSocket)/1024.00);
	}
    else
    	printf("L2 cache:              %d Kib\n ",atoi(cpuCache.L2) * count.corePerSocket );
    if(cpuCache.L3 != NULL)
        printf("L3 cache:              %d Mib\n",atoi(cpuCache.L3) * count.corePerSocket / 1024);
    if(type.nptr != NULL)
	{
		for(int i = 0; i < type.nptr->onlineCount; i++)
		{
			printf("NUMA    %s    CPU(s): %d,%d\n",type.nptr->node->name, type.nptr->node->cpulist[0]
																	,type.nptr->node->cpulist[1]);
		}
	}
	if(type.vulnerabilities[ITLB_MYLTIHIT] != NULL)	
            printf("Vulnerability Itlb multihibit:    %s",type.vulnerabilities[ITLB_MYLTIHIT]);
	if(type.vulnerabilities[L1TF] != NULL)	
			printf("Vulnerability L1tf:               %s",type.vulnerabilities[L1TF]);
	if(type.vulnerabilities[MDS] != NULL)	
			printf("Vulnerability Mds:                %s",type.vulnerabilities[MDS]);
	if(type.vulnerabilities[MELTDOUN] != NULL)	
			printf("Vulnerability Meltdown:           %s",type.vulnerabilities[MELTDOUN]);
	if(type.vulnerabilities[SPEC_STORE_BYPASS] != NULL)	
			printf("Vulnerability Spec store bypass:  %s",type.vulnerabilities[SPEC_STORE_BYPASS]);
	if(type.vulnerabilities[SPECTRE_V1] != NULL)	
			printf("Vulnerability Spectre v1:         %s",type.vulnerabilities[SPECTRE_V1]);
	if(type.vulnerabilities[SPECTRE_V2] != NULL)	
			printf("Vulnerability Spectre v2:         %s",type.vulnerabilities[SPECTRE_V2]);
	if(type.vulnerabilities[SRBDS] != NULL)	
			printf("Vulnerability Srbds:              %s",type.vulnerabilities[SRBDS]);
	if(type.vulnerabilities[TSX_ASYNC_ABORT] != NULL)	
			printf("Vulnerability Tsx async abort:    %s",type.vulnerabilities[TSX_ASYNC_ABORT]);			
	printf("Flags:                %s",type.flag);
}

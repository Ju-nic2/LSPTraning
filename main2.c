#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

// barrier for multi thread
pthread_barrier_t operate_barrier;
pthread_barrier_t write_barrier;

//Tread Argv
struct ThreadArgvs{
	int startline;
	int endline;
	int rows;
	int cols;
	int generation;
	char **nowMatrix;
	char **nextMatrix;
};
//for cell operation location information
struct LocationInfo{
	int x;
	int y;
	int m;
	int n;
};

//shared memory key
#define NOWMATRIXKEY 1111
#define NEXTMATRIXKEY 2222
#define COUNTERKEY 3333

//file name size 
#define NAMEBUFSIZE 512

void workSignal(int sig)
{
}
//menu function
void sequentialOperation(int generation, char *inputfile);
void multiProcessOperation(int generation, int processnum, char *inputfile);
void multiThreadOperation(int generation, int threadnum, char *inputfile);

//operation function
void operation(char **current, char **next, int m, int n, int startline,int endline);
char nextGenerationCell(char **current, struct LocationInfo *now,int flag);

//diskIO function
char** readInputFile(int *n, int *m, char *filename);
void writeMatrixInFile(char **Matrix, int m, int n, int max);
void makeWriteFormat(char* linebuf, char *line,int linesize);
void makeFileName(char *filename,char *prefix,int num,char *suffix);

//memory function
unsigned int sizeof_Matrix(int rows, int cols, size_t sizeElement);
void create_index(void **Matrix,int rows, int cols, size_t sizeElement);
void initializeSharedMemory(char **originMatrix, char **emptyMatrix,int rows, int cols);
void deleteMatrix(char **matrix, int rows);
void rowdistribution(int *arr,int arrsize,int rows);
void mymemcpy(char **copyed, char **origin,int m, int n);

//tread function
void initializeThreadArgv(struct ThreadArgvs *ta,int startline,int endline,int m, int n,int generation,char **m1,char **m2);
void* threadMethod(void* argv);

int main(int argc, char **argv)
{
	
	int choice = 0;
	int generation = 0;
	char *inputfile;
	if(argc < 2)
	{
		inputfile = malloc(sizeof(char) * strlen("input.matrix"));
		strcpy(inputfile,"input.matrix");
	}else{
		inputfile = malloc(sizeof(char) * strlen(argv[1]));
		strcpy(inputfile,argv[1]);
	}
	printf("Your Input FIle is %s\n",inputfile);
	while(1)
	{
		printf("**Cell Matrix Game **\n");
		printf("1. exit\n");
		printf("2. sequential Processing \n");
		printf("3. multi Processing \n");
		printf("4. multi Thread\n");
		printf("Input Menu Number : ");
		scanf("%d",&choice);
		if(choice == 1) break;
		if(choice == 2)
		{
			printf("How many Generation ? : ");
			scanf("%d",&generation);
			sequentialOperation(generation,inputfile);
		}if(choice == 3)
		{
			int processNum=0;
			printf("How Many Generation ? : ");
			scanf("%d",&generation);
			printf("How Many Process ? : ");
			scanf("%d",&processNum);
			multiProcessOperation(generation,processNum,inputfile);
		}
		if(choice == 4)
		{
			int threadNum=0;
			printf("How Many Generation ? : ");
			scanf("%d",&generation);
			printf("How Many thread ? : ");
			scanf("%d",&threadNum);
			multiThreadOperation(generation,threadNum,inputfile);
		}
		
	}

}

void sequentialOperation(int generation, char *inputfile)
{
	struct timeval start,end;
	char **nowMatrix; char **newMatrix;
	int n=0; int m = 0;
	gettimeofday(&start,NULL);
	//initailize metrix
	if((nowMatrix = readInputFile(&n,&m,inputfile)) == NULL)
	{
		fprintf(stderr,"Input Matrix File has invailed value\n");
		exit(1);
	}
	newMatrix = (char**)malloc(sizeof(char*)*m);
	for(int i = 0; i<m; i++)
	{
		newMatrix[i] = (char*)malloc(sizeof(char)*n);
	}
	mymemcpy(newMatrix,nowMatrix,m,n);

	for(int i = 1; i<=generation; i++)
	{
		operation(nowMatrix, newMatrix,m,n,0,m);
		writeMatrixInFile(newMatrix,m,n,generation);
		mymemcpy(nowMatrix,newMatrix,m,n);
		
	}
	gettimeofday(&end,NULL);
	printf("time : %fms\n",(double)((end.tv_sec - start.tv_sec)*1000 + 
				(end.tv_usec - start.tv_usec)/1000));
	deleteMatrix(nowMatrix,m);
	deleteMatrix(newMatrix,n);
}

void multiProcessOperation(int generation,int processnum, char *inputfile)
{
	struct timeval start,end;
	char **readMatrix;
	int m = 0; int n = 0;

	gettimeofday(&start,NULL);
	if((readMatrix = readInputFile(&n,&m,inputfile)) == NULL)
	{
		fprintf(stderr,"Input Matrix file has invailed value\n");
		exit(1);
	}

	if(processnum > m)
	{
		fprintf(stderr,"Too Many Process\n");
		fprintf(stderr,"appropriate Process num is under ROWs number\n");
		exit(1);
	}

	int shm_id1; int shm_id2; int shm;
	char **nowMatrix; char **nextMatrix; int *counter;

	size_t Matrix_size = sizeof_Matrix(m,n,sizeof(char));
	//shared memory allocate for now Matrix 
	if((shm_id1 = shmget(NOWMATRIXKEY,Matrix_size,IPC_CREAT|0666)) == -1)
	{
		fprintf(stderr,"Get Shared Memory Id Error\n");
		exit(1);
	}
	//shared memory allocate for next Matrix
	if((shm_id2 = shmget(NEXTMATRIXKEY,Matrix_size,IPC_CREAT|0666)) == -1)
	{
		fprintf(stderr,"Get Shared Memory Id Error\n");
		exit(1);
	}
	if((shm = shmget(COUNTERKEY,sizeof(int),IPC_CREAT|0666)) == -1)
	{
		fprintf(stderr,"Get shared Memory Id Error\n");
		exit(1);
	}

	//get shared memory address
	if((nowMatrix = shmat(shm_id1,NULL,0)) == (void*)-1)
	{
		fprintf(stderr,"Get Shared Memory addr Error\n");
		exit(1);
	}
	if((nextMatrix = shmat(shm_id2,NULL,0)) == (void*)-1)
	{
		fprintf(stderr,"Get Shared Memory addr Error\n");
		exit(1);
	}
	if((counter = shmat(shm,NULL,0)) == (void*)-1)
	{
		fprintf(stderr,"Get Shared Memory addr Error\n");
		exit(1);
	}

	//create index of Matrix(2Demtion array)
	create_index((void*)nowMatrix,m,n,sizeof(char));
	create_index((void*)nextMatrix,m,n,sizeof(char));
	
	//initialize all shared Memory
	initializeSharedMemory(readMatrix,nowMatrix,m,n);
	initializeSharedMemory(readMatrix,nextMatrix,m,n);
	deleteMatrix(readMatrix,m);
	(*counter) = 0;
	
	//for Synchronize process
	sem_t mutex;
	if(sem_init(&mutex,1,1) < 0){
		fprintf(stderr,"semaphore error");
		exit(1);
	}

	pid_t *child = malloc(sizeof(int)*processnum);
	int *distribution = malloc(sizeof(int)*processnum);
	rowdistribution(distribution,processnum,m);

	int ischild = 0;
	int myorder;
	if(signal(SIGUSR1,workSignal) == SIG_ERR)
	{
		fprintf(stderr, "Signal error\n");
		exit(1);
	}
	//create child process 
	for(int p = 0; p < processnum; p++)
	{
		if((child[p] = fork())< 0)
		{
			fprintf(stderr,"fork error\n");
			exit(1);
		}
		else if(child[p] == 0)
		{
			ischild = 1;
			myorder = p;
			printf("%d 's child id : %d ppid : %d\n",p,getpid(),getppid());
			if(p > 0)
				printf("I'll operate %d ~ %d\n",distribution[p-1]+1,distribution[p]);
			else
				printf("I'll operate %d ~ %d\n",1,distribution[p]);
			break;
		}
	}
	//make generation
	for(int i = 1; i <= generation; i++)
	{
			//child process code
			if(ischild == 1 && myorder > 0){
				operation(nowMatrix,nextMatrix,m,n,distribution[myorder-1],distribution[myorder]);
				//for child process synchronize
				sem_wait(&mutex);
				(*counter)++;
				sem_post(&mutex);
				//wait parent's signal
				pause();
			}else if(ischild == 1 && myorder == 0){
				operation(nowMatrix,nextMatrix,m,n,0,distribution[myorder]);
				//for child process synchronize
				sem_wait(&mutex);
				(*counter)++;
				sem_post(&mutex);
				//wait parent's signal
				pause();
			}
			//parent process code
			else{
				//wait for all child finish
				while(*counter < processnum);
				*counter = 0;
				writeMatrixInFile(nextMatrix,m,n,generation);
				initializeSharedMemory(nextMatrix,nowMatrix,m,n);
				if(i < generation){
					for(int j = 0; j<processnum; j++)
					{
						//signal to child process "work!"
						kill(child[j],SIGUSR1);
					}
				}
				else{
					for(int j = 0; j<processnum; j++)
					{
						//siganl to child process "exit"
						kill(child[j],SIGKILL);
					}

				}
			}
	}

	gettimeofday(&end,NULL);
	printf("time : %fms\n",(double)((end.tv_sec - start.tv_sec)*1000 + 
				(end.tv_usec - start.tv_usec)/1000));
	free(child);
	free(distribution);
	shmdt(nowMatrix);
	shmdt(nextMatrix);
}

void multiThreadOperation(int generation, int threadnum, char* inputfile)
{
	struct timeval start,end;
	char **readMatrix;
	int m = 0; int n = 0;

	gettimeofday(&start,NULL);
	if((readMatrix = readInputFile(&n,&m,inputfile)) == NULL)
	{
		fprintf(stderr,"Input Matrix file has invailed value\n");
		exit(1);
	}

	if(threadnum > m)
	{
		fprintf(stderr,"Too Many thread\n");
		fprintf(stderr,"appropriate thread num is under ROWs number\n");
		exit(1);
	}

	int shm_id1; int shm_id2;
	char **nowMatrix; char **nextMatrix;

	size_t Matrix_size = sizeof_Matrix(m,n,sizeof(char));
	//shared memory allocate for now Matrix 
	if((shm_id1 = shmget(NOWMATRIXKEY,Matrix_size,IPC_CREAT|0666)) == -1)
	{
		fprintf(stderr,"Get Shared Memory Id Error\n");
		exit(1);
	}
	//shared memory allocate for next Matrix
	if((shm_id2 = shmget(NEXTMATRIXKEY,Matrix_size,IPC_CREAT|0666)) == -1)
	{
		fprintf(stderr,"Get Shared Memory Id Error\n");
		exit(1);
	}
	//get shared memory address
	if((nowMatrix = shmat(shm_id1,NULL,0)) == (void*)-1)
	{
		fprintf(stderr,"Get Shared Memory addr Error\n");
		exit(1);
	}
	if((nextMatrix = shmat(shm_id2,NULL,0)) == (void*)-1)
	{
		fprintf(stderr,"Get Shared Memory addr Error\n");
		exit(1);
	}
	//create index of Matrix(2Demtion array)
	create_index((void*)nowMatrix,m,n,sizeof(char));
	create_index((void*)nextMatrix,m,n,sizeof(char));
	
	initializeSharedMemory(readMatrix,nowMatrix,m,n);
	initializeSharedMemory(readMatrix,nextMatrix,m,n);
	deleteMatrix(readMatrix,m);
	//init barrier
	if(pthread_barrier_init(&operate_barrier,NULL,threadnum+1) != 0)
	{
		fprintf(stderr,"operate_barrier init error\n");
		exit(1);
	}

	if(pthread_barrier_init(&write_barrier,NULL,threadnum+1) != 0)
	{
		fprintf(stderr,"write_barrier init error\n");
		exit(1);
	}

	pthread_t *tid = malloc(sizeof(pthread_t)*threadnum);
	struct ThreadArgvs *argv = malloc(sizeof(struct ThreadArgvs)*threadnum);
	int *distribution = malloc(sizeof(int)*threadnum);
	rowdistribution(distribution,threadnum,m);

	for(int t = 0; t<threadnum; t++)
	{
		if(t > 0)
			initializeThreadArgv(&argv[t],distribution[t-1],distribution[t],m,n,generation,nowMatrix,nextMatrix);
		else
			initializeThreadArgv(&argv[t],0,distribution[t],m,n,generation,nowMatrix,nextMatrix);
		pthread_create(&tid[t],NULL,threadMethod,&argv[t]);
	}

	for(int i = 1; i<=generation; i++)
	{
		pthread_barrier_wait(&operate_barrier);
		writeMatrixInFile(nextMatrix,m,n,generation);
		initializeSharedMemory(nextMatrix,nowMatrix,m,n);
		pthread_barrier_wait(&write_barrier);

		if(i == generation){
			for(int t = 0; t<threadnum; t++)
			{
				pthread_join(tid[t],NULL);
			}
		}
	}
	gettimeofday(&end,NULL);
	printf("time : %fms\n",(double)((end.tv_sec - start.tv_sec)*1000 + 
				(end.tv_usec - start.tv_usec)/1000));

	free(tid);
	free(argv);
	free(distribution);
	shmdt(nowMatrix);
	shmdt(nextMatrix);
	pthread_barrier_destroy(&operate_barrier);
	pthread_barrier_destroy(&write_barrier);

}

void initializeThreadArgv(struct ThreadArgvs *ta,int startline,int endline,int m, int n,int generation, char **m1,char **m2)
{
	ta->startline = startline;
	ta->endline = endline;
	ta->rows = m;
	ta->cols = n;
	ta->generation = generation;
	ta->nowMatrix = m1;
	ta->nextMatrix = m2;
}


void* threadMethod(void* argv)
{
	struct ThreadArgvs *ta = (struct ThreadArgvs *)argv;
	printf("Thread : %u\n",(unsigned int)pthread_self());
	printf("I'll operate %d ~ %d\n",ta->startline+1,ta->endline);
	for(int i = 0; i< ta->generation; i++){
		operation(ta->nowMatrix,ta->nextMatrix,ta->rows,ta->cols,ta->startline,ta->endline);
		pthread_barrier_wait(&operate_barrier);
		pthread_barrier_wait(&write_barrier);
	}
	pthread_exit(NULL);
}
void initializeSharedMemory(char **originMatrix, char **newMatrix, int rows, int cols)
{
	for(int i = 0; i<rows; i++)
	{
		for(int j = 0; j<cols; j++)
		{
			newMatrix[i][j] = originMatrix[i][j];
		}
	}
}

void deleteMatrix(char **matrix, int rows)
{
	for(int i = 0; i<rows; i++)
		free(matrix[i]);
	free(matrix);
}

void rowdistribution(int *arr,int arrsize,int rows)
{
	int num = rows;
	int index = 0;
	for(int i = 0; i<arrsize; i++)
	{
		arr[i] = 0;
	}
	while(num > 0)
	{
		arr[index++]++;
		num--;
		if(index == arrsize)
			index = 0;
	}
	for(int i = 1; i<arrsize; i++)
	{
		arr[i] = arr[i] + arr[i-1];
	}

}
void mymemcpy(char **copyed, char **origin,int m, int n)
{
	for(int i=0; i<m; i++)
	{
		memcpy(copyed[i],origin[i],sizeof(char)*n);
	}
}



void operation(char **current, char **next, int m, int n, int startline,int endline)
{
	struct LocationInfo li;
	li.m=m; li.n=n;
	for(int i = startline; i < endline; i++)
	{
		for(int j = 0; j < n; j++)
		{
			li.x = j;
			li.y = i;
			switch(current[i][j]){
				case '1':
					next[i][j] = nextGenerationCell(current,&li,1);
					break;
				case '0':
					next[i][j] = nextGenerationCell(current,&li,0);
					break;
			}
		}
	}
}

char nextGenerationCell(char **current, struct LocationInfo *now,int flag)
{
	int survivors = 0;
	int x = now->x; int y = now->y; int m = now->m; int n = now->n;
	if((x >= 1 && x < n-1) && (y >= 1 && y < m-1))
	{
		for(int i = -1; i<=1; i++)
		{
			for(int j = -1; j<=1; j++)
			{
				if(current[y+i][x+j] == '1') survivors++;
			}
		}
	}else if(y == 0)
	{
		if(x >= 1 && x < n-1)
		{
			for(int i = 0; i<=1; i++)
			{
				for(int j = -1; j <=1; j++)
				{
					if(current[y+i][x+j] == '1') survivors++;
				}
			}
		}else if(x == 0)
		{
			for(int i =0; i<=1; i++)
			{
				for(int j = 0; j<=1; j++)
				{	
					if(current[y+i][x+j] == '1') survivors++;
				}
			}
		}else if(x == n-1){
			for(int i =0; i<=1; i++)
			{
				for(int j = -1; j<=0; j++)
				{	
					if(current[y+i][x+j] == '1') survivors++;
				}
			}
		}
	}else if(y == m-1)
	{
		if(x >= 1 && x < n-1)
		{
			for(int i = -1; i<=0; i++)
			{
				for(int j = -1; j <=1; j++)
				{
					if(current[y+i][x+j] == '1') survivors++;
				}
			}
		}else if(x == 0)
		{
			for(int i =-1; i<=0; i++)
			{
				for(int j = 0; j<=1; j++)
				{	
					if(current[y+i][x+j] == '1') survivors++;
				}
			}
		}else if(x == n-1){
			for(int i =-1; i<=0; i++)
			{
				for(int j = -1; j<=0; j++)
				{	
					if(current[y+i][x+j] == '1') survivors++;
				}
			}
		}
	}else{
		if(x==0)
		{
			for(int i =-1; i<=1; i++)
			{
				for(int j = 0; j<=1; j++)
				{	
					if(current[y+i][x+j] == '1') survivors++;
				}
			}
		}
		else if(x == n-1)
		{
			for(int i =-1; i<=1; i++)
			{
				for(int j = -1; j<=0; j++)
				{	
					if(current[y+i][x+j] == '1') survivors++;
				}
			}
		}
	}
	//arrive cell
	if(flag == 1){
		survivors--;
		if(survivors >= 3 && survivors <=6) return '1';
		else return '0';
	}else
	{
		//deadcell
		if(survivors == 4) return '1';
		else return '0';

	}

}


char** readInputFile(int *n, int *m, char *inputfile)
{
	FILE *inputfp;

	if((inputfp = fopen(inputfile,"r")) == NULL)
	{
		fprintf(stderr,"There are no input file");
		exit(1);
	}
	int tn = 0; int tm = 0;
	//get first line of matrix file
	while(fgetc(inputfp) != '\n'){
		tn++;
	}

	char *linebuf = (char*)malloc(sizeof(char)*(tn+1));
	fseek(inputfp,0,SEEK_SET);
	while(fgets(linebuf,tn+1,inputfp) != NULL)
	{
		if(ferror(inputfp))
		{
			fprintf(stderr,"file read error\n");
			exit(1);
		}
		tm=tm+1;
		fseek(inputfp,1,SEEK_CUR);
	}

	char **matrix = (char**)malloc(sizeof(char*)* tm);	
	for(int i = 0; i<tm; i++)
	{
		matrix[i] = (char*)malloc(sizeof(char)* tn/2);
	}

	int tmp; int index = 0; int line = 0;
	
	fseek(inputfp,0,SEEK_SET);
	while((tmp = fgetc(inputfp)) != EOF)
	{
		if(tmp == '0') matrix[line][index++] = (char)tmp;
		else if(tmp == '1') matrix[line][index++] = (char)tmp;
		else if(tmp == '\n')
		{
			line++;
			index = 0;
		}else if(tmp >= '2'&& tmp <= '9')
			return NULL;
	}

	*n = tn/2;
	*m = tm;

	free(linebuf);
	fclose(inputfp);

	return(matrix);
}

void writeMatrixInFile(char **Matrix, int m, int n, int max)
{
	static int nowGeneration = 1; 
	FILE *outputfp;
	char filename[NAMEBUFSIZE];
	if(nowGeneration < max)
	{
		makeFileName(filename,"gen_",nowGeneration,".matrix");
	}else
	{
		makeFileName(filename,"output",-1,".matrix");
		nowGeneration = 0;
	}

	if((outputfp = fopen(filename,"w+")) == NULL)
	{
		fprintf(stderr,"file opne error\n");
		exit(1);
	}
	char *linebuf = (char *)malloc(sizeof(char) * (n*2));
	for(int i = 0; i<m; i++)
	{
		makeWriteFormat(linebuf,Matrix[i],n);
		fwrite(linebuf,strlen(linebuf),1,outputfp);
	}

	nowGeneration++;
	fclose(outputfp);
	free(linebuf);

}

//this method make filename string + number + string
void makeFileName(char *filename,char *prefix, int num, char *suffix)
{
	//number limit is length of max integer in (int)
	char numbuffer[11] = {'\0',};
	//only positive number can write in file name
	if(num > 0)
		sprintf(numbuffer,"%d",num);

	strncpy(filename,prefix,strlen(prefix));
	strncpy(&filename[strlen(prefix)],numbuffer,strlen(numbuffer));
	strncpy(&filename[strlen(prefix)+strlen(numbuffer)],suffix,strlen(suffix));
	filename[strlen(prefix)+strlen(numbuffer)+strlen(suffix)] = '\0';

}
void makeWriteFormat(char* linebuf, char *line,int linesize)
{
	for(int i = 0; i<linesize*2; i+=2)
	{
		linebuf[i] = line[i/2];
		linebuf[i+1] = ' ';
	}
	linebuf[linesize*2-1] = '\n';
}

	
unsigned int sizeof_Matrix(int rows, int cols, size_t sizeElement)
{
	size_t size = rows * (sizeof(void *) + (cols *sizeElement));
	return size;
}

void create_index(void **Matrix,int rows, int cols, size_t sizeElement)
{
	size_t sizeRow = cols * sizeElement;
	Matrix[0] = Matrix+rows;
	for(int i = 1; i<rows; i++)
	{
		Matrix[i] = (Matrix[i-1]+sizeRow);
	}
}
		



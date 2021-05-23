#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include "choice.h"

void sequentialOperation(int generation);
void multiProcessOperation(int generation, int processnum);
void multiThreadOperation(int generation, int threadnum);
void initializeThreadArgv(struct ThreadArgvs *ta,int startline,int endline,int m, int n, char **m1,char **m2);
void* threadMethod(void* argv);

//seperate to operation.c
void operation(char **current, char **next, int m, int n, int startline,int endline);
char nextGenerationCell(char **current, struct LocationInfo *now,int flag);
void rowdistribution(int *arr,int arrsize,int rows);

//seperate to disk_i/o.c
char** readInputFile(int *n, int *m);
void writeMatrixInFile(char **Matrix, int m, int n, int max);
void makeWriteFormat(char* linebuf, char *line,int linesize);
void makeFileName(char *filename,char *prefix,int num,char *suffix);

//seperate to memory.c
unsigned int sizeof_Matrix(int rows, int cols, size_t sizeElement);
void create_index(void **Matrix,int rows, int cols, size_t sizeElement);
void initializeSharedMemory(char **originMatrix, char **emptyMatrix,int rows, int cols);
void deleteMatrix(char **matrix, int rows);
void mymemcpy(char **copyed, char **origin,int m, int n);

void sequentialOperation(int generation)
{
	struct timeval start,end;
	char **nowMatrix; char **newMatrix;
	int n=0; int m = 0;
	gettimeofday(&start,NULL);
	//initailize metrix
	if((nowMatrix = readInputFile(&n,&m)) == NULL)
	{
		fprintf(stderr,"Input Matrix File has invailed value\n");
		exit(1);
	}
	if((newMatrix = readInputFile(&n,&m)) == NULL)
	{
		fprintf(stderr,"Input Matrix File has invailed value\n");
		exit(1);
	}


	for(int i = 1; i<=generation; i++)
	{
		operation(nowMatrix, newMatrix,m,n,0,m);
		writeMatrixInFile(newMatrix,m,n,generation);
		mymemcpy(nowMatrix,newMatrix,m,n);
		
	}
	gettimeofday(&end,NULL);
	printf("time : %dus\n",(int)((end.tv_sec - start.tv_sec)*1000000 + 
				(end.tv_usec - start.tv_usec)));
}

void multiProcessOperation(int generation,int processnum)
{
	struct timeval start,end;
	char **readMatrix;
	int m = 0; int n = 0;

	gettimeofday(&start,NULL);
	if((readMatrix = readInputFile(&n,&m)) == NULL)
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

	pid_t *child = malloc(sizeof(int)*processnum);
	int *distribution = malloc(sizeof(int)*processnum);
	rowdistribution(distribution,processnum,m);


	for(int i = 1; i <= generation; i++)
	{
		for(int p = 0; p < processnum; p++)
		{
			if((child[p] = fork())< 0)
			{
				fprintf(stderr,"fork error\n");
				exit(1);
			}
			else if(child[p] == 0)
			{
				printf("%d 's child id : %d pid : %d\n",p,getpid(),getppid());
				if(p > 0)
					operation(nowMatrix,nextMatrix,m,n,distribution[p-1],distribution[p]);
				else
					operation(nowMatrix,nextMatrix,m,n,0,distribution[p]);
				exit(1);
			}
		}
		while(wait((int*)0) != -1);
		printf("parent \n");
		writeMatrixInFile(nextMatrix,m,n,generation);
		initializeSharedMemory(nextMatrix,nowMatrix,m,n);
	}
	gettimeofday(&end,NULL);
	printf("time : %dus\n",(int)((end.tv_sec - start.tv_sec)*1000000 + 
				(end.tv_usec - start.tv_usec)));
	free(child);
	free(distribution);
	shmdt(nowMatrix);
	shmdt(nextMatrix);
}

void multiThreadOperation(int generation, int threadnum)
{
	struct timeval start,end;
	char **readMatrix;
	int m = 0; int n = 0;

	gettimeofday(&start,NULL);
	if((readMatrix = readInputFile(&n,&m)) == NULL)
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

	pthread_t *tid = malloc(sizeof(pthread_t)*threadnum);
	struct ThreadArgvs *argv = malloc(sizeof(struct ThreadArgvs)*threadnum);
	int *distribution = malloc(sizeof(int)*threadnum);
	rowdistribution(distribution,threadnum,m);

	for(int i = 1; i<=generation; i++)
	{
		for(int t = 0; t<threadnum; t++)
		{
			if(t > 0)
				initializeThreadArgv(&argv[t],distribution[t-1],distribution[t],m,n,nowMatrix,nextMatrix);
			else
				initializeThreadArgv(&argv[t],0,distribution[t],m,n,nowMatrix,nextMatrix);
			pthread_create(&tid[t],NULL,threadMethod,&argv[t]);
		}

		for(int t = 0; t<threadnum; t++)
		{
			pthread_join(tid[t],NULL);
		}
		printf("main Thread\n");
		writeMatrixInFile(nextMatrix,m,n,generation);
		initializeSharedMemory(nextMatrix,nowMatrix,m,n);
	}
	gettimeofday(&end,NULL);
	printf("time : %dus\n",(int)((end.tv_sec - start.tv_sec)*1000000 + 
				(end.tv_usec - start.tv_usec)));

	free(tid);
	free(argv);
	free(distribution);
	shmdt(nowMatrix);
	shmdt(nextMatrix);



}

void initializeThreadArgv(struct ThreadArgvs *ta,int startline,int endline,int m, int n, char **m1,char **m2)
{
	ta->startline = startline;
	ta->endline = endline;
	ta->rows = m;
	ta->cols = n;
	ta->nowMatrix = m1;
	ta->nextMatrix = m2;
}


void* threadMethod(void* argv)
{
	struct ThreadArgvs *ta = (struct ThreadArgvs *)argv;
	printf("Thread : %u\n",(unsigned int)pthread_self());
	operation(ta->nowMatrix,ta->nextMatrix,ta->rows,ta->cols,ta->startline,ta->endline);
	pthread_exit(NULL);
}


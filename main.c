#include <stdio.h>
#include <stdlib.h>


//sepeate to choice.c
void sequentialOperation(int generation);
void multiProcessOperation(int generation, int processnum);
void multiThreadOperation(int generation, int threadnum);
void initializeThreadArgv(struct ThreadArgvs *ta,int startline,int endline,int m, int n, char **m1,char **m2);
void* threadMethod(void* argv);


int main(int argc, char **argv)
{
	
	int choice = 0;
	int generation = 0;
	printf("**Cell Matrix Game **\n");
	while(1)
	{
		printf("1. exit\n");
		printf("2. sequential Processing \n");
		printf("3. multi Processing \n");
		printf("4. multi Thread\n");
		scanf("%d",&choice);
		if(choice == 1) break;
		if(choice == 2)
		{
			printf("How many Generation ? : ");
			scanf("%d",&generation);
			sequentialOperation(generation);
		}if(choice == 3)
		{
			int processNum=0;
			printf("How Many Generation ? : ");
			scanf("%d",&generation);
			printf("How Many Process ? : ");
			scanf("%d",&processNum);
			multiProcessOperation(generation,processNum);
		}
		if(choice == 4)
		{
			int threadNum=0;
			printf("How Many Generation ? : ");
			scanf("%d",&generation);
			printf("How Many thread ? : ");
			scanf("%d",&threadNum);
			multiThreadOperation(generation,threadNum);
		}
		
	}

}


		



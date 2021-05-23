#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** readInputFile(int *n, int *m);
void writeMatrixInFile(char **Matrix, int m, int n, int max);
void makeWriteFormat(char* linebuf, char *line,int linesize);
void makeFileName(char *filename,char *prefix,int num,char *suffix);

char** readInputFile(int *n, int *m)
{
	FILE *inputfp;

	if((inputfp = fopen("input.matrix","r")) == NULL)
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

	printf("%d %d\n",tn,tm);
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
		nowGeneration = 1;
	}

	if((outputfp = fopen(filename,"w+")) == NULL)
	{
		fprintf(stderr,"file opne error\n");
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

void makeWriteFormat(char* linebuf, char *line,int linesize)
{
	for(int i = 0; i<linesize*2; i+=2)
	{
		linebuf[i] = line[i/2];
		linebuf[i+1] = ' ';
	}
	linebuf[linesize*2-1] = '\n';
}


//this method make filename string + number + string
void makeFileName(char *filename,char *prefix, int num, char *suffix)
{
	//number limit is length of max integer in (int)
	char numbuffer[10];
	//only positive number can write in file name
	if(num > 0)
		sprintf(numbuffer,"%d",num);

	strncpy(filename,prefix,strlen(prefix));
	strncpy(&filename[strlen(prefix)],numbuffer,strlen(numbuffer));
	strncpy(&filename[strlen(prefix)+strlen(numbuffer)],suffix,strlen(suffix));
	filename[strlen(prefix)+strlen(numbuffer)+strlen(suffix)] = '\0';

}

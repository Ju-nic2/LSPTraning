#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>
#include <string.h>
#include "flags.h" 


#define BUFSIZE 1024

//argument Parsing Functions
int getOption(int argc, char **argv);
void getAllDir(int argc, char**argv,char **files);

//get num of elements for dynamic allocate array
int getNumOfFiles(int argc, char **argv);
int getNumOfDir(int argc,char **argv);
int getNumOfFilesInDir(char *dir);

//get datas for array
void getAllFiles(struct files farr[],char *dir);
void getAllFiles_ar(struct files farr[],int limit,int argc, char**argv);

//print Fuctions
int getSumOfFileSize(struct files farr[],int flag,int n);
void printFiles(struct files farr[],int flag,int n);
void printDir(char **dirlist,int flag,int n);
void print_l(struct files farr[],int flag,int n);
void print_n(struct files farr[],int flag,int n);
char type(mode_t);
char* perm(mode_t);

//sorting Functions
void sortByName(struct files farr[], int p, int r);
int partitionByName(struct files farr[],int p, int r);
void sortByTime(struct files farr[], int p, int f);
int partitionByTime(struct files farr[],int p,int f);
void swap(struct files *a, struct files *b);


int main(int argc,char **argv)
{

	if(argc == 2 && !(strcmp(argv[1],"--help")))
	{
		printf("myls [option] <directory> or <file>\n");
		printf("you can mix order option, directory , filename\n");
		printf("you can write many file, directory\n");
		printf("[option] ---------------------------\n");
		printf("-l : show all information of files\n");
		printf("-a : do not ignore file start with '.'\n");
		printf("-t : sort by last modified tiem\n");
		printf("-i : show i-node number\n");
		printf("you can mix options\n");
		exit(0);
	}

	//get option in argument
	int opt = 0;
	opt = getOption(argc,argv);

	int num_files_in_argu = getNumOfFiles(argc,argv);
	if(num_files_in_argu > 0) printf("num : %d\n",num_files_in_argu);
	if(num_files_in_argu > 0){
		//dynamic allocate struct array for files in argument
		struct files *_farr = malloc(sizeof(struct files)*num_files_in_argu);
		getAllFiles_ar(_farr,num_files_in_argu, argc, argv);
		printFiles(_farr,opt,num_files_in_argu);
		printf("\n");
	}
	//get num of directory in argument
	int num_of_dir = getNumOfDir(argc,argv);
	if(num_of_dir > 0){
		//dynamic allocate for directory name
		char **dirlist = malloc(sizeof(char*)*num_of_dir);
		//get directory name in dirlist array
		getAllDir(argc,argv,dirlist);
		//print files in directory(directory in dirlist array)
		printDir(dirlist,opt,num_of_dir);
	}else if(num_of_dir == 0 && num_files_in_argu == 0)
	{
		char **dirlist = malloc(sizeof(char*)*1);
		dirlist[0] = ".";
		printDir(dirlist,opt,1);
	}
	exit(0);
}
int getNumOfFiles(int argc, char**argv)
{
	int count = 0;
	DIR *dp;
	struct stat st;
	for(int i = 1; i<argc; i++)
	{
		for(int j = 0; j<strlen(argv[i]); j++)
		{
			if(argv[i][0] == '-')
				break;
			//if it is file can open with opendir()fuction
			if((dp = opendir(argv[i])) != NULL){
				closedir(dp);
				break;
			}
			if(lstat(argv[i],&st) != -1){
				count++;
				break;
			}else{
				break;
			}
		}
	}
	return count;
}
int getNumOfDir(int argc,char **argv)
{
	int count = 0;
	DIR *dp;
	for(int i = 1; i<argc; i++)
	{
		for(int j = 0; j<strlen(argv[i]); j++)
		{
			if(argv[i][0] == '-')
				break;
			if((dp = opendir(argv[i])) != NULL){
				count++;
				closedir(dp);
				break;
			}else{
				break;
			}
		}
	}
	return count;
}
	
void getAllDir(int argc, char **argv,char **dirlist)
{
	int count = 0;
	DIR *dp;
	for(int i = 1; i<argc; i++)
	{
		for(int j = 0; j<strlen(argv[i]); j++)
		{
			if(argv[i][0] == '-')
				break;
			if((dp = opendir(argv[i])) != NULL)
			{
				//dynamic allocate for copy argv[i]'s data
				dirlist[count] = malloc(sizeof(char)*strlen(argv[i]));
				strcpy(dirlist[count++],argv[i]);
				closedir(dp);
				break;
			}
		}
	}
	//count == 0 means youser want to see only current directory's information
	if(count == 0){
		dirlist[count] = malloc(sizeof(char)*strlen("."));
		strcpy(dirlist[count],".");
	}
}

int getOption(int argc, char **argv)
{
	int tmpopt = 0;
	for(int i = 1; i<argc; i++)
	{
		for(int j = 0; j<strlen(argv[i]); j++)
		{
			if(argv[i][0] != '-'){
				continue;
			}
			switch(argv[i][j]){
				case 't':
					tmpopt |= FLAG_T; break;
				case 'l':
					tmpopt |= FLAG_L; break;
				case 'a':
					tmpopt |= FLAG_A; break;
				case 'i':
					tmpopt |= FLAG_I; break;
			}
		}
	}
	return tmpopt;
}
//get num of files in directory for dynamic allocate
int getNumOfFilesInDir(char*dir)
{
	DIR *dp;
	struct dirent *d;
	struct stat st;
	int count = 0;
	char path[BUFSIZE];
	if((dp = opendir(dir)) ==  NULL)
		perror(dir);
	while((d = readdir(dp)) != NULL)
	{
		sprintf(path,"%s/%s",dir,d->d_name);
		if(lstat(path,&st) != -1)
			count++;
	}
	closedir(dp);
	return count;
}

//get all file's stat struct and save mystruct  
void getAllFiles(struct files farr[],char *dir)
{
	DIR *dp;
	struct stat st;
	struct dirent *d;
	char path[BUFSIZE];

	if((dp = opendir(dir)) == NULL)
		perror(dir);

	seekdir(dp,0);
	int i = 0;
	while((d = readdir(dp)) != NULL){
		sprintf(path,"%s/%s",dir,d->d_name);
		if(lstat(path,&st) != -1){
				strcpy(farr[i].filename , d->d_name);
				farr[i].inode = st.st_ino;
				farr[i].mode = st.st_mode;
				farr[i].nlink = st.st_nlink;
				farr[i].uid = st.st_uid;
				farr[i].gid = st.st_gid;
				farr[i].size = st.st_size;
				farr[i].mtime = st.st_mtime;
				i++;
		}
	}
	closedir(dp);

}
//get All file's(in argument) stat sturct and save mystruct
void getAllFiles_ar(struct files farr[],int n,int argc,char** argv)
{
	DIR *dp;
	struct stat st;
	int k=0;
	for(int i = 1; i<argc; i++)
	{
		for(int j = 0; j<strlen(argv[i]); j++)
		{
			//for segmentation falt
			if(k==n)
				break;

			if(argv[i][0] == '-')
				break;
			if((dp = opendir(argv[i])) != NULL)
				break;
			if(lstat(argv[i],&st) != -1){
				strcpy(farr[k].filename , argv[i]);
				farr[k].inode = st.st_ino;
				farr[k].mode = st.st_mode;
				farr[k].nlink = st.st_nlink;
				farr[k].uid = st.st_uid;
				farr[k].gid = st.st_gid;
				farr[k].size = st.st_size;
				farr[k].mtime = st.st_mtime;
				k++;
				break;
			}
		}
	}
}
	

void swap(struct files *a, struct files *b)
{
	struct files tmp = *a;
	*a = *b;
	*b = tmp;
}

void sortByName(struct files farr[],int p, int r)
{
	if(p<r)
	{
		int pv = partitionByName(farr,p,r);
		sortByName(farr,p,pv-1);
		sortByName(farr,pv+1,r);
	}
}
int partitionByName(struct files farr[], int p, int r)
{
	char *pivot = malloc(sizeof(char)*strlen(farr[r].filename));
	strcpy(pivot,farr[r].filename);
	int count = p;
	for(int i = p; i < r; i++){
		if(strcmp(farr[i].filename,pivot) < 0) swap(&farr[count++],&farr[i]);
	}
	swap(&farr[count],&farr[r]);
	free(pivot);
	return count;
}

void sortByTime(struct files farr[],int p, int r)
{
	if(p<r)
	{
		int pv = partitionByTime(farr,p,r);
		sortByTime(farr,p,pv-1);
		sortByTime(farr,pv+1,r);
	}
}
int partitionByTime(struct files farr[], int p, int r)
{
	time_t pivot = farr[r].mtime;
	int count = p;
	for(int i = p; i < r; i++){
		if(farr[i].mtime > pivot) swap(&farr[count++],&farr[i]);
	}
	swap(&farr[count],&farr[r]);
	return count;
}



void printFiles(struct files _farr[],int flag, int num_files_in_argu)
{
	if(flag & FLAG_T)
		sortByTime(_farr,0,num_files_in_argu-1);
	else
		sortByTime(_farr,0,num_files_in_argu-1);

	if(flag & FLAG_L)
		print_l(_farr,flag,num_files_in_argu);
	else
		print_n(_farr,flag,num_files_in_argu);
}

void printDir(char **dirlist,int flag,int num_of_dir)
{
	for(int i = 0; i < num_of_dir; i++)
	{
		if(num_of_dir > 1) printf("%s :\n",dirlist[i]);
		//dynamic allocate file struct array
		int num_files_in_dir = getNumOfFilesInDir(dirlist[i]);
		struct files *farr = malloc(sizeof(struct files)*num_files_in_dir);
		getAllFiles(farr,dirlist[i]);

		if(flag & FLAG_T)
			sortByTime(farr,0,num_files_in_dir-1);
		else
			sortByName(farr,0,num_files_in_dir-1);
	
		if(flag & FLAG_L){
			printf("sum : %d\n",getSumOfFileSize(farr,flag,num_files_in_dir));
			print_l(farr,flag,num_files_in_dir);
		}
		else
			print_n(farr,flag,num_files_in_dir);
		
		free(farr);
	}

}

char type(mode_t mode){
	if(S_ISREG(mode))
		return('-');
	if(S_ISDIR(mode))
		return('d');
	if(S_ISCHR(mode))
		return('c');
	if(S_ISBLK(mode))
		return('b');
	if(S_ISLNK(mode))
		return('I');
	if(S_ISFIFO(mode))
		return('p');
	if(S_ISSOCK(mode))
		return('s');
}

char* perm(mode_t mode)
{
	int i;
	static char perms[10] = "---------";
	for(int j = 0 ; j<9; j++)
		perms[j] = '-';
	perms[9] = '\0';
	for(i = 0; i<3; i++){
		if(mode & (S_IRUSR >> i*3))
			perms[i*3] = 'r';
		if(mode & (S_IWUSR >> i*3))
			perms[i*3+1] = 'w';
		if(mode & (S_IXUSR >> i*3))
		{
			//perms[i*3+2] = 'x';
			
			if(i < 2){
				if(mode & (S_ISUID >> i))
					perms[i*3+2] = 's';
				else
					perms[i*3+2] = 'x';
			}else{
				if(mode & (S_ISUID >> i))
					perms[i*3+2] = 't';
				else
					perms[i*3+2] = 'x';
			}
		}else{
			if(i < 2){
				if(mode & (S_ISUID >> i))
					perms[i*3+2] = 'S';
			}else{
				if(mode & (S_ISUID >> i))
					perms[i*3+2] = 'T';
			}
		
		}	
	}	
	return(perms);
}
//print all information of stat struct With opton -l
void print_l(struct files farr[],int flag,int n)
{

	for(int i = 0; i<n;i++){
		//for option -a
		if(!(strcmp(farr[i].filename,"."))){
			if(~flag & FLAG_A)
				continue;
		}
		if(!(strcmp(farr[i].filename,".."))){
			if(~flag & FLAG_A)
				continue;
		}
		//for option -i
		if(flag & FLAG_I)
		{
			printf("%9ld ",farr[i].inode);
		}
		printf("%c%s ",type(farr[i].mode),perm(farr[i].mode));
		printf("%3ld ",farr[i].nlink);
		printf("%s %s ",getpwuid(farr[i].uid)->pw_name, getgrgid(farr[i].gid)->gr_name);
		printf("%9ld ",farr[i].size);
		printf("%.12s ",ctime(&farr[i].mtime)+4);
		printf("%s\n",farr[i].filename);
	}
}
//print just name
void print_n(struct files farr[], int flag, int n)
{
	for(int i = 0; i<n;i++){
		//for option -a
		if(!(strcmp(farr[i].filename,"."))){
			if(~flag & FLAG_A)
				continue;
		}
		if(!(strcmp(farr[i].filename,".."))){
			if(~flag & FLAG_A)
				continue;
		}
		//for option -i
		if(flag & FLAG_I)
		{
			printf("%9ld ",farr[i].inode);
		}
		printf("%s ",farr[i].filename);
	}
	printf("\n");
}
int getSumOfFileSize(struct files farr[],int flag, int n)
{
	int sum=0;
	for(int i=0; i<n; i++){
		if(!(strcmp(farr[i].filename,"."))){
			if(~flag & FLAG_A)
				continue;
		}
		if(!(strcmp(farr[i].filename,".."))){
			if(~flag & FLAG_A)
				continue;
		}
		if(farr[i].size >= 4096)
			sum += (int)((double)(farr[i].size/4096)*4);
		else
			sum += 4;
	}
	return sum;
}

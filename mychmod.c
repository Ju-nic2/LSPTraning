#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

//argv parsing functions
void argvParsing_c(char *options,mode_t *fmode);
int checkArgv(char *argv);
int optionParsing(char *argv);

//get number permition functions
mode_t argvParsing(char *mode);

//get character permition functions
void modeParsing(char *buf,mode_t *fmode);
char whatIsOperation(char *buf,int *position);
void setUserPermition(char op, char* permition, mode_t* fmode);
void setGruopPermition(char op, char* permition, mode_t* fmode);
void setOtherPermition(char op, char* permition, mode_t* fmode);
void setAllPermition(char op, char* permition, mode_t* fmode);

//change permition functions
void changePermition(char **argv, int start, int end,int flag,int check);
char* printPermition(mode_t mode);

//print permition function option '-c'
void resetPermition(mode_t *origin);

int main(int argc, char **argv)
{
	if(argc == 2 && !strcmp(argv[1],"--help"))
	{
		printf("mycmod [option] <mode> <filename>\n");
		printf("you must keep order [option],mode,file\n");
		printf("if you want to set permition with many file \n");
		printf("mycmod [option] <mode> <filename1> <filename2> ,,\n");
		printf("[option] -c : print filename orginOption changedOption\n");
		exit(1);
	}

	int opt = 0;
	opt = optionParsing(argv[1]);
	if(opt)
		changePermition(argv,3,argc,opt,2);
	else
		changePermition(argv,2,argc,opt,1);
}

int checkArgv(char *argv)
{
	if(argv[0] >= '0' && argv[0] <= '7')
		return 1;
	else
		return 0;
}

int optionParsing(char *argv)
{
	if(argv[0] == '-')
	{
		if(argv[1] == 'c')
			return 1;
		else{
			fprintf(stderr,"no Option %c\n",argv[1]);
			exit(1);
		}
	}else
		return 0;
	return 0;
}

mode_t argvParsing(char *mode)
{
	mode_t mymode = 0;
	if(strlen(mode) == 4 && (mode[0] >= '0' && mode[0] <= '9')){
		int in = 3;
		for(int i=0;i<4; i++)
		{
			int n = mode[in] - '0';
			if(n >=0 && n <=7){
				mymode |= (n << i*3);
			}
			in--;
		
		}
		return mymode;
	}else if(strlen(mode) == 3 && (mode[0] >= '0' && mode[0] <= '9')){
		int in = 2;
		for(int i=0;i<3; i++)
		{
			int n = mode[in] - '0';
			if(n >=0 && n <=7){
				mymode |= (n << i*3);
			}
			in--;
		
		}
		return mymode;
	}
	else{
		fprintf(stderr,"please write mode 0777,7777\n");
		exit(1);
	}


}

void argvParsing_c(char *options, mode_t* fmode)
{
	//parse options ex) o+x,r+x -> 'o+x' 'r+x'
	char* tmpoption = strtok(options,",");
	while(tmpoption != NULL)
	{
		modeParsing(tmpoption,fmode);
		tmpoption = strtok(NULL,",");
	}
}

//for two return value effect, use pointer
char whatIsOperation(char *buf,int *position)
{
	char operator;
	for(int i =0; i<strlen(buf); i++)
	{
		if(buf[i] == '+' || buf[i] == '-' || buf[i] == '=')
		{
			operator = buf[i];
			*position = i;
			break;
		}
	}
	return operator;
}
void modeParsing(char *buf,mode_t* fmode)
{
	int position = 0;
	//get operation character
	char op = whatIsOperation(buf,&position);
	//get permition ex) o+x permiton save 'x'
	char *permition;
	switch(op){
		case '+':
			strtok_r(buf,"+",&permition);break;
		case '-':
			strtok_r(buf,"-",&permition);break;
		case '=':
			strtok_r(buf,"=",&permition);break;
		default:
			fprintf(stderr,"can't operate %c",op); exit(1);break;
	}
	//'potition == 0' means no sepcific target
	if(position == 0)
		fprintf(stderr,"you must set u/g/o\n");

	for(int i=0; i<position; i++)
	{
		switch(buf[i]){
			case 'u':
				setUserPermition(op,permition,fmode);break;
			case 'U':
				setUserPermition(op,permition,fmode);break;
			case 'g':
				setGruopPermition(op,permition,fmode);break;
			case 'G':
				setGruopPermition(op,permition,fmode);break;
			case 'o':
				setOtherPermition(op,permition,fmode);break;
			case 'O':
				setOtherPermition(op,permition,fmode);break;
			case 'a':
				setAllPermition(op,permition,fmode);break;
			case 'A':
				setAllPermition(op,permition,fmode);break;
			default:
				printf("can set permition to %c",buf[i]);break;
		}
	}
}
void setUserPermition(char op, char* permition, mode_t* fmode)
{
	//assign operation
	if(op == '='){
		mode_t tmp = 0;
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					tmp |= S_IRUSR;break;
				case 'w':
					tmp |= S_IWUSR;break;
				case 'x':
					tmp |= S_IXUSR;break;
				case 's':
					tmp |= S_ISUID; break;
			}
		}
		//initiation origin permition bit
		//for get new setting about rwx/set-user-id
		*fmode &= ~S_ISUID;
		*fmode &= ~S_IRUSR;
		*fmode &= ~S_IWUSR;
		*fmode &= ~S_IXUSR;
		//assig new permition
		*fmode |= tmp;
	}else if(op == '+'){
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					*fmode |= S_IRUSR;break;
				case 'w':
					*fmode |= S_IWUSR;break;
				case 'x':
					*fmode |= S_IXUSR;break;
				case 's':
					*fmode |= S_ISUID; break;
			}
		}
	}else if(op == '-')	{
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					*fmode &= ~S_IRUSR;break;
				case 'w':
					*fmode &= ~S_IWUSR;break;
				case 'x':
					*fmode &= ~S_IXUSR;break;
				case 's':
					*fmode &= ~S_ISUID; break;
			}
		}
	}


}
void setGruopPermition(char op, char* permition, mode_t* fmode)
{
	if(op = '='){
		mode_t tmp = 0;
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					tmp |= S_IRGRP;break;
				case 'w':
					tmp |= S_IWGRP;break;
				case 'x':
					tmp |= S_IXGRP;break;
				case 's':
					tmp |= S_ISGID; break;
			}
		}
		*fmode &= ~S_ISGID;
		*fmode &= ~S_IRGRP;
		*fmode &= ~S_IWGRP;
		*fmode &= ~S_IXGRP;
		*fmode |= tmp;
	}else if(op == '+'){
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					*fmode |= S_IRGRP;break;
				case 'w':
					*fmode |= S_IWGRP;break;
				case 'x':
					*fmode |= S_IXGRP;break;
				case 's':
					*fmode |= S_ISGID; break;
			}
		}
	}else if(op == '-')	{
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					*fmode &= ~S_IRGRP;break;
				case 'w':
					*fmode &= ~S_IWGRP;break;
				case 'x':
					*fmode &= ~S_IXGRP;break;
				case 's':
					*fmode &= ~S_ISGID; break;
			}
		}
	}
}
		
void setOtherPermition(char op, char* permition, mode_t* fmode)
{
	if(op == '='){
		mode_t tmp = 0;
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					tmp |= S_IROTH;break;
				case 'w':
					tmp |= S_IWOTH;break;
				case 'x':
					tmp |= S_IXOTH;break;
				case 's':
					tmp |= S_ISVTX; break;
			}
		}
		*fmode &= ~S_ISVTX;
		*fmode &= ~S_IROTH;
		*fmode &= ~S_IWOTH;
		*fmode &= ~S_IXOTH;

		*fmode |= tmp;
	}else if(op == '+'){
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					*fmode |= S_IROTH;break;
				case 'w':
					*fmode |= S_IWOTH;break;
				case 'x':
					*fmode |= S_IXOTH;break;
				case 's':
					*fmode |= S_ISVTX; break;
			}
		}
	}else if(op == '-')	{
		for(int i = 0; i<strlen(permition); i++)
		{
			switch(permition[i]){
				case 'r':
					*fmode &= ~S_IROTH;break;
				case 'w':
					*fmode &= ~S_IWOTH;break;
				case 'x':
					*fmode &= ~S_IXOTH;break;
				case 's':
					*fmode &= ~S_ISVTX;break;
			}
		}
	}
}
 
void setAllPermition(char op, char* permition, mode_t* fmode)
{
	setUserPermition(op,permition,fmode);
	setGruopPermition(op,permition,fmode);
	setOtherPermition(op,permition,fmode);
}
void resetPermition(mode_t *origin)
{
		*origin &= 0b1111000000000000;
}
void changePermition(char **argv, int start, int end,int flag,int check)
{
	if(checkArgv(argv[check])){
		for(int i=start; i<end; i++){
			struct stat st;
			if(lstat(argv[i],&st) == -1){
				fprintf(stderr,"can't open %s ",argv[i]);
			}
			mode_t mode_tmp = st.st_mode;

			//reset origin permitions for save file type bits
			resetPermition(&mode_tmp);

	 		mode_tmp |= argvParsing(argv[check]);
			if(chmod(argv[i],mode_tmp) < 0)
			{	
				fprintf(stderr,"can't chmod %s \n",argv[i]);
			}
			if(flag){
				printf("mode of '%s' changed (%s)",argv[i],printPermition(st.st_mode));
				printf(" to (%s)\n",printPermition(mode_tmp));
			}
		}
	}else{
		for(int i = start; i<end; i++){
			struct stat st;

			//save mode temporary in tmp storage because of strtok fuction
			char *tmpoption = malloc(sizeof(char)*strlen(argv[check]));
			strcpy(tmpoption,argv[check]);
			if(lstat(argv[i],&st) == -1){
				fprintf(stderr,"can't open %s ",argv[i]);
			}

			mode_t mode_tmp = st.st_mode;
			argvParsing_c(tmpoption,&mode_tmp);
			if(chmod(argv[i],mode_tmp) < 0)
			{
				fprintf(stderr,"can't chmod %s",argv[i]);
			}
			if(flag){
				printf("mode of '%s' changed (%s)",argv[i],printPermition(st.st_mode));
				printf(" to (%s)\n",printPermition(mode_tmp));
			}
			free(tmpoption);
		}
	}
}
char* printPermition(mode_t mode)
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

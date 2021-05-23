#include "operation.h"

void operation(char **current, char **next, int m, int n, int startline,int endline);
char nextGenerationCell(char **current, struct LocationInfo *now,int flag);
void rowdistribution(int *arr,int arrsize,int rows);

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

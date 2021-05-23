#include <stdlib.h>
#include <string.h>

unsigned int sizeof_Matrix(int rows, int cols, size_t sizeElement);
void create_index(void **Matrix,int rows, int cols, size_t sizeElement);
void initializeSharedMemory(char **originMatrix, char **emptyMatrix,int rows, int cols);
void deleteMatrix(char **matrix, int rows);
void mymemcpy(char **copyed, char **origin,int m, int n);

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


void mymemcpy(char **copyed, char **origin,int m, int n)
{
	for(int i=0; i<m; i++)
	{
		memcpy(copyed[i],origin[i],sizeof(char)*n);
	}
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

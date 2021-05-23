#define NOWMATRIXKEY 1111
#define NEXTMATRIXKEY 2222
#define NAMEBUFSIZE 512
struct ThreadArgvs{
	int startline;
	int endline;
	int rows;
	int cols;
	char **nowMatrix;
	char **nextMatrix;
};



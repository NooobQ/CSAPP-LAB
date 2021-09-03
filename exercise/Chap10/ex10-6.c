#include "csapp.h"
int main(){
	int fd1, fd2, fd3;
	fd1 =  Open("foo.txt", O_RDONLY, 0);
	fd2 =  Open("bar.txt", O_RDONLY, 0);
	Close(fd2);
	fd2 =  Open("bar.txt", O_RDONLY, 0);
	printf("fd2 = %d\n", fd2);
	Close(fd2);
	fd3 = Open("foobar.txt", O_RDONLY, 0);
	printf("fd3 = %d\n", fd3);
	exit(0);
}

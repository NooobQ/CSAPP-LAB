#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

int main()
{    
	int fd = open("hello.txt",O_TRUNC | O_RDWR);
	if(fd == -1)
		assert("open");

	char *s= "Jello, world!\n";
	size_t len = strlen(s);
	char *t = (char *)malloc((len + 1)*sizeof(char));
	strcpy(t,s);
	printf("%s\n",t);

	struct stat sb;
	if(fstat(fd, &sb) == -1)
		assert("fstat");

	printf("size = %ld\n", sb.st_size);

	ftruncate(fd,(len + 1) * sizeof(char));

	if(fstat(fd, &sb) == -1)
		assert("fstat");
	printf("size = %ld\n", sb.st_size);

	char* addr = mmap(NULL, (len + 1) * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(addr == MAP_FAILED)
		assert("mmap");

	if(close(fd) == -1)
		assert("close");

	memset(addr, 0, sizeof(addr));
	memcpy(addr, t, len);
	//printf("0x%x\n",(int)addr);
	return 0;
}

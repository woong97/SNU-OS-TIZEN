#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>


#define	FILE_PATH		"integer"
#define	SYSCALL_ROTLOCK_WRITE	400
#define SYSCALL_ROTUNLOCK_WRITE	402


int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Input should be ./selector <integer>\n");
		exit(EXIT_FAILURE);
	}

	int number = atoi(argv[1]);
	// FILE *fp = fopen(FILE_PATH, "w");

	// TEST WRITE
	// TODO - WRITELOCK
	while(1) {
		if (syscall(SYSCALL_ROTLOCK_WRITE, 90, 90) < 0) {
			printf("write lock failed\n");
			return EXIT_FAILURE;
		}
		FILE *fp = fopen(FILE_PATH, "w");
	
		fprintf(fp, "%d", number++);
		fclose(fp);
		if (syscall(SYSCALL_ROTUNLOCK_WRITE, 90, 90) < 0) {
			printf("write unlock failed\n");
			return EXIT_FAILURE;
		}
		printf("write succeed: %d\n", number);
		sleep(1);
	}
	// printf("Write Succeed\n");
	return EXIT_SUCCESS;
}

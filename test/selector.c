#include <stdio.h>
#include <stdlib.h>

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
	FILE *fp = fopen(FILE_PATH, "w");

	// TEST WRITE
	// TODO - WRITELOCK
	fprintf(fp, "%d", number);
	fclose(fp);
	printf("Write Succeed\n");
	return 0;
}

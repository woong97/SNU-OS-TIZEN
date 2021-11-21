#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>

#define	FILE_PATH		"integer"
#define	SYSCALL_ROTLOCK_WRITE	400
#define SYSCALL_ROTUNLOCK_WRITE	402
#define SLOW_MODE		1
#define DEFAULT_MODE		0


int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Input should be './selector <integer>' or './selector <integer> slow' \n");
		exit(EXIT_FAILURE);
	}

	int mode;
	if (argc == 3 && (strcmp(argv[2], "slow") == 0)){
		mode = SLOW_MODE;
	} else {
		mode = DEFAULT_MODE;
	}

	printf("Running mode is %s\n", mode ? "'slow mode'" : "'default mode'"); 

	int number = atoi(argv[1]);

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
		if (mode == SLOW_MODE)
			sleep(1);
	}
	return EXIT_SUCCESS;
}

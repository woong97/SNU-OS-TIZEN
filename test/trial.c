#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#define FILE_PATH		"integer"
#define SYSCALL_ROTLOCK_READ	399
#define SYSCALL_ROTUNLOCK_READ	401


int naive_factorization(int number, int idx)
{
	if (number <= 0) return -EINVAL;
	if (number == 1) {
		printf("1 = 1\n");
		return EXIT_SUCCESS;
	}
	int original_number = number;
	char *output = "";
	for (int i = 2; i < original_number; i++) {
		if ((number % i) == 0) {
			number /= i;
			if (strlen(output) == 0) {
				if(asprintf(&output, "%d", i) == -1) return -ENOMEM;
			} else {
				if(asprintf(&output, "%s * %d", output, i) == -1) return -ENOMEM;
			}
			i--;
		}
	}
	
	if (original_number == number)
		if(asprintf(&output, "%d", number) == -1) return -ENOMEM;
	printf("trial-%d: %d = %s\n", idx, original_number, output);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	// TODO
	if (argc != 2) {
		printf("Input should be ./integer <process index>\n");
		exit(EXIT_FAILURE);
	}
	int idx = atoi(argv[1]);
	int number;
	
	// TEST READ!
	// TODO - READLOCK
	while (1) {
		if (syscall(SYSCALL_ROTLOCK_READ, 90, 90) < 0) {
			printf("read lock failed\n");
		}

		FILE *fp = fopen(FILE_PATH, "r");
		fscanf(fp, "%d", &number);
		fclose(fp);
		if (naive_factorization(number, idx) < 0) {
			perror("Factoraization Failed");
			return EXIT_FAILURE;
		}
		sleep(2);
		
		if (syscall(SYSCALL_ROTUNLOCK_READ, 90, 90) < 0) {
			printf("read unlock failed\n");
		}
	}
	return EXIT_SUCCESS;
}

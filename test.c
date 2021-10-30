#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sched.h>

#define SET_SCHEDULER_SYS_NUM		156
#define SCHED_SETWEIGHT_SYS_NUM		398
#define SCHED_GETWEIGHT_SYS_NUM		399
#define SCHED_WRR			7


int naive_factorization(int number)
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
	printf("%d = %s\n", original_number, output);
	return EXIT_SUCCESS;
}

int child(int num) {
	double t_start = clock();
	if (naive_factorization(num) < 0) {
		printf("factorization failed,  errno = %d\n", errno);
		return EXIT_FAILURE;
	}
	double t_end = clock();
	double spend_time = (double)(t_end - t_start)/CLOCKS_PER_SEC;
	printf("Factorization takes %f\n", spend_time);
	return EXIT_SUCCESS;
}


int main(int argc, char* argv[]) {
	int ret;
	pid_t pid;
	struct sched_param param;
	param.sched_priority = 0;

	if (argc < 2) {
		printf("Insufficient input arguments [ ex) ./test 24 ]\n");
		return EXIT_FAILURE;
	}
	
	if (syscall(SET_SCHEDULER_SYS_NUM, getpid(), SCHED_WRR, &param)) {
		perror("set scheduler failed!");
		return EXIT_FAILURE;
	}

	pid = fork();
	if (pid == 0) {
		child(atoi(argv[1]));
		return EXIT_SUCCESS;
	} else if (pid > 0) {
		int status;
		waitpid(pid, &status, 0);
		if WIFEXITED(status) {
			printf("Child terminated normally\n");
		} else {
			printf("Child terminated abnormally\n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

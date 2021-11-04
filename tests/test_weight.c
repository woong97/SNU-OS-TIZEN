#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sched.h>

#define SET_SCHEDULER_SYS_NUM		156
#define SCHED_SETWEIGHT_SYS_NUM		398
#define SCHED_GETWEIGHT_SYS_NUM		399
#define SCHED_WRR			7
#define MAX_WEIGHT			20

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

int main(int argc, char* argv[]) {
	int ret;
	struct sched_param param;
	param.sched_priority = 0;
	struct timeval t_start, t_end;
	long spend_time;

	if (argc < 3) {
		printf("Insufficient input arguments [ ex) ./test (weight) (number) ]\n");
		return EXIT_FAILURE;
	}
	int weight = atoi(argv[1]);
	int num = atoi(argv[2]);

	if (syscall(SET_SCHEDULER_SYS_NUM, getpid(), SCHED_WRR, &param)) {
		perror("set scheduler failed!");
		exit(EXIT_FAILURE);
	}
	if (syscall(SCHED_SETWEIGHT_SYS_NUM, getpid(), weight) < 0) {
		perror("setweight failed");
		exit(EXIT_FAILURE);
	}
	
	gettimeofday(&t_start, NULL);
	if (naive_factorization(num) < 0) {
		printf("[%d] - Factorization failed,  errno = %d\n", getpid(), errno);
		exit(EXIT_FAILURE);
	}
	gettimeofday(&t_end, NULL);
	spend_time = (t_end.tv_sec - t_start.tv_sec) * 1000 + (t_end.tv_usec - t_start.tv_usec)/1000.0;

	printf("weight = %d -> Factorization takes %ldms\n", weight, spend_time);
	return EXIT_SUCCESS;
}

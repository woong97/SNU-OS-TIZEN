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

int naive_factorization(long long number)
{
	if (number <= 0) return -EINVAL;
	if (number == 1) {
		printf("1 = 1\n");
		return EXIT_SUCCESS;
	}
	
	long long original_number = number;
	char *output = "";
	for (long long i = 2; i < original_number; i++) {
		if ((number % i) == 0) {
			number /= i; 
			if (strlen(output) == 0) {
				if(asprintf(&output, "%lld", i) == -1) return -ENOMEM;
			} else {
		       		if(asprintf(&output, "%s * %lld", output, i) == -1) return -ENOMEM;
			}
			i--;
		}
	}

	if (original_number == number)
		if(asprintf(&output, "%lld", number) == -1) return -ENOMEM;
	printf("%lld = %s\n", original_number, output);
	return EXIT_SUCCESS;
}

long child(long long num) {
	struct timeval t_start, t_end;
	long spend_time;

	gettimeofday(&t_start, NULL);
	if (naive_factorization(num) < 0) {
		printf("Child %d - Factorization failed,  errno = %d\n", getpid(), errno);
		exit(EXIT_FAILURE);
	}
	gettimeofday(&t_end, NULL);
	spend_time = (t_end.tv_sec - t_start.tv_sec) * 1000 + (t_end.tv_usec - t_start.tv_usec)/1000.0;
	return spend_time;
}


int main(int argc, char* argv[]) {
	int ret;
	struct sched_param param;
	param.sched_priority = 0;

	if (argc < 3) {
		printf("Insufficient input arguments [ ex) ./test (nprocess) (number) ]\n");
		return EXIT_FAILURE;
	}
	int nproc = atoi(argv[1]);
	pid_t pid[nproc];
	nproc = 5;
	int weights[nproc];
	srand(time(NULL));
	/*for (int i = 0; i < nproc; i ++) {
		weights[i] = rand() % MAX_WEIGHT + 1;
		printf("===weight: %d\n", weights[i]);
	}*/
	weights[0] = 2;
	weights[1] = 7;
	weights[2] = 20;
	weights[3] = 6;
	weights[4] = 11;
	for (int i = 0; i < nproc; i++) {
		pid[i] = fork();
		if (pid[i] == 0) {
			if (syscall(SET_SCHEDULER_SYS_NUM, getpid(), SCHED_WRR, &param)) {
				perror("set scheduler failed!");
				exit(EXIT_FAILURE);
			}
			if (syscall(SCHED_SETWEIGHT_SYS_NUM, getpid(), weights[i]) < 0) {
				perror("Child %d setweight failed");
				exit(EXIT_FAILURE);
			}
			long spend_time = child(atoll(argv[2]));
			printf("Child %d (weight=%d) -> Factorization takes %ld\n", getpid(), weights[i], spend_time);
			exit(EXIT_SUCCESS);
		}
	}
	
	for (int i = 0; i < nproc; i++) {
		int status;
		waitpid(pid[i], &status, 0); 
		if WIFEXITED(status) {
			printf("Child %d terminated normally\n", pid[i]);
		} else {
			printf("Child %d terminated abnormally\n", pid[i]);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

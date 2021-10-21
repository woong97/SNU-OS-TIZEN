#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define SCHED_SETWEIGHT_SYS_NUM		398
#define SCHED_GETWEIGHT_SYS_NUM		399

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
	
	if (argc < 2) {
		printf("Insufficient input arguments [ ex) ./test 24 ]\n");
		return EXIT_FAILURE;
	}
	// system call test
	ret = syscall(SCHED_SETWEIGHT_SYS_NUM, 0, 0);
	ret = syscall(SCHED_GETWEIGHT_SYS_NUM, 0);
	
	double t_start = clock();
	if (naive_factorization(atoi(argv[1])) < 0) {
		printf("factorization failed,  errno = %d\n", errno);
		return EXIT_FAILURE;
	}
	double t_end = clock();
	double spend_time = (double)(t_end - t_start)/CLOCKS_PER_SEC;
	printf("Factorization takes %f\n", spend_time);
	return EXIT_SUCCESS;
}

#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include "include/linux/prinfo.h"
#include <stdlib.h>
#include <errno.h>

#define PTREE_SYS_CALL_NUM	398
#define DEFAULT_NR		100


int main(int argc, char* argv[]){
	int nr;
	int num_process;
	if (argc > 2) {
		perror("Too many input");
		exit(1);
	} else if (argc == 2) {
		nr = atoi(argv[1]);
	} else {
		nr = DEFAULT_NR;
	}

	struct prinfo *buf = (struct prinfo *)malloc(sizeof(struct prinfo) * nr);
	num_process = syscall(PTREE_SYS_CALL_NUM, buf, &nr);
	if (num_process < 0){
		if (errno == EFAULT){
			perror("Fail access User memory address");
		} else if (errno == EINVAL) {
			perror("Pointer input is null or nr is lower than 1");
		} else {
			fprintf(stderr, "Value of errno: %d\n", errno);
			perror("syscall() fail");
		}
		exit(1);	
	};

	printf("Total number of process: %d\n", num_process);
	free(buf);
	return 0;
}


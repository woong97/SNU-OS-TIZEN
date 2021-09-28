#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include "include/linux/prinfo.h"
#include <stdlib.h>

int main(){
	int nr = 5;
	struct prinfo *buf = (struct prinfo *)malloc(sizeof(struct prinfo) * nr);
	int num_process = syscall(398, t, &nr);
	printf("Total number of process: %d\n", num_process);
	free(buf);
}


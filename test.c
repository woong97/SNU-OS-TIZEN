#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include "include/linux/prinfo.h"
#include <stdlib.h>

int main(){
	int nr = 5;
	struct prinfo *t = (struct prinfo *)malloc(sizeof(struct prinfo)*nr);
	int total = syscall(398, t, &nr);
	printf("Test succeed: %d\n", total);
}


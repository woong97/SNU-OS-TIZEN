#include <stdio.h>
#include <unistd.h>


#define SCHED_SETWEIGHT_SYS_NUM		398
#define SCHED_GETWEIGHT_SYS_NUM		399

int main(int argc, char* argv[]) {
	int ret;
	// system call test
	ret = syscall(SCHED_SETWEIGHT_SYS_NUM, 0, 0);
	ret = syscall(SCHED_GETWEIGHT_SYS_NUM, 0);
	return 0;
}

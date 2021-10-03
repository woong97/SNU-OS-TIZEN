#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include "linux/prinfo.h"
#include <stdlib.h>
#include <errno.h>

#define PTREE_SYS_CALL_NUM	398
#define DEFAULT_NR		150

void prinfo_print(struct prinfo *buf, int nr); // pre-order printing of ptree

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
	prinfo_print(buf, nr);
	free(buf);
	return 0;
}

void prinfo_print(struct prinfo *buf, int nr) {
	// pid_list and level_tracker for indentation
	int *level_list = (int *)malloc(sizeof(int)*nr);
	int current_level = 0;

	int i = 0, j = 0; // indexing variable
	struct prinfo p = buf[i];
	int prev = -1;
	printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid,
		p.first_child_pid, p.next_sibling_pid, p.uid);
	level_list[i] = current_level;
	for(i = 1; i < nr; i++) {
		p = buf[i];
		prev = i-1;						// used for finding current_level
		if(buf[prev].first_child_pid == p.pid)	// p is child of buf[prev]
			current_level++;					// increment level
		else if(buf[prev].next_sibling_pid == p.pid)	// p is sibling of buf[prev]
			current_level;								// do nothing
		else {									// case 3
			while(prev >= 0 &&
				p.parent_pid != buf[prev].pid) prev--;	// find parent
			if(prev < 0) current_level = 0;				// no parent => level = 0
			else current_level = level_list[prev]+1;	// level = level[parent]+1
		}
		for(j = 0; j < current_level; j++) printf("  ");
		printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state,
			p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
		level_list[i] = current_level;
	}
}

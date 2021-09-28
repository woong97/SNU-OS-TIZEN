//#include "stdio.h"
#include <linux/kernel.h>
#include "linux/prinfo.h"
asmlinkage int sys_ptree(struct prinfo *buf, int *nr){
	int a;
	a = *nr + 1;
	printk("Hello ptree.c\n");
	return a;
}

#include <linux/kernel.h>
#include <linux/prinfo.h>


asmlinkage int sys_ptree(struct prinfo *buf, int *nr){
	int a;
	a = *nr + 1;
	return a;

}

/*
 * asmlinkabe: for i386 architecture
 * printk: <linux/printk.h>
 * why not <linux/prinfo.h>?
 */

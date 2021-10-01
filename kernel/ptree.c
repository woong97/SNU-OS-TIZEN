#include <linux/kernel.h>
#include <linux/prinfo.h>
#include <linux/unistd.h>
//#include <uapi/asm-generic/errno-base.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/types.h>
/*
struct prinfo {
    int64_t state;          // current state of process
    pid_t pid;              // pid
    pid_t parent_pid        // pid of parent
    pid_t first_child_pid;  // pid of oldest child
    pid_t next_sibling_pid  // pid of younger sibling
    int64_t uid             // user id of process owner
    char comm[64];          // name of process (ex: systemd)
};
*/

struct tasklist
{
	struct task_struct *task;
	struct list_head list;
};

static LIST_HEAD(tasklist_head);		// making new list starting with tasklist_head?

void init_tasklist(void){
	struct tasklist *root;
	root = (struct tasklist *)kmalloc(sizeof(struct tasklist), GFP_KERNEL);
	// init_task : swapper
	root->task = &init_task;
	INIT_LIST_HEAD(&(root->list));				// added parentheses
	list_add(&(root->list), &tasklist_head);	// same as above
}

void preorderSearch(struct prinfo *buf, int *process_count){
	// Test print: I'll erase under two lines 
	struct tasklist *swapper = list_entry(tasklist_head.next, struct tasklist, list);
	printk("pid is %d, comm: %s\n", swapper->task->pid, swapper->task->comm);
}


asmlinkage int sys_ptree(struct prinfo *buf, int *nr){
	int tmp_nr;
	struct prinfo tmp_prinfo;
	int copy_failed_byte;
	int process_count = 0;
	struct prinfo * kernel_buf;

	if (buf == NULL || nr == NULL)
		return -EINVAL;

	// need to check: access_ok just check availabilty of scope of address
	// So we will use copy_from_user
	// need check: copy_from_user is the most powerful among
	// copy_from user, get_user, access_ok
	if (copy_from_user(&tmp_nr, nr, sizeof(tmp_nr)))
		return -EFAULT;
	if (copy_from_user(&tmp_prinfo, buf, sizeof(tmp_prinfo)))
		return -EFAULT;
	if (tmp_nr < 1) {
		return -EINVAL;
	}

	kernel_buf = (struct prinfo *)kmalloc(sizeof(struct prinfo) * tmp_nr, GFP_KERNEL);
	if (kernel_buf == NULL)
		return ENOMEM;

	read_lock(&tasklist_lock);
	init_tasklist();
	preorderSearch(kernel_buf, &process_count);
	read_unlock(&tasklist_lock);
	
	if ((copy_failed_byte = copy_to_user(buf, kernel_buf, sizeof(struct prinfo) * tmp_nr)) != 0) {
		printk("Could not copy %d bytes of prinfo struct from kernel to user\n", copy_failed_byte);
	}
	if ((copy_failed_byte = copy_to_user(nr, &process_count, sizeof(int))) != 0) {
		printk("Could not copy %d bytes of process count from kernel to user\n", copy_failed_byte);
	}

	kfree(kernel_buf);
	return *nr;

}

/*
 * asmlinkabe: for i386 architecture
 * printk: <linux/printk.h>
 * why not <linux/prinfo.h>?
 */

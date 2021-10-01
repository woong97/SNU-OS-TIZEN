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

extern struct task_struct init_task;

void tasklist_traversal(struct prinfo *buf, int *nr) {
	int i = 0;
	struct task_struct *task_p = &init_task;
	struct prinfo *prinfo_p = buf;
	for(i = 0; i < *nr; i++) {
		printk("%d %s\n", task_p->pid, task_p->comm);
		prinfo_p->state = task_p->state;
		prinfo_p->pid = task_p->pid;
		prinfo_p->parent_pid = task_p->real_parent->pid;
		prinfo_p->first_child_pid = list_entry((&task_p->children)->next, struct task_struct, children)->pid;
		prinfo_p->next_sibling_pid = list_entry((&task_p->sibling)->next, struct task_struct, sibling)->pid;
		prinfo_p->uid;	//
		strncpy(prinfo_p->comm, task_p->comm, 16);
		prinfo_p++;
		task_p = list_entry((&task_p->children)->next, struct task_struct, children);
	}
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
	tasklist_traversal(kernel_buf, nr);
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

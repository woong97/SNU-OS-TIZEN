#include <linux/kernel.h>
#include <linux/prinfo.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/types.h>
#include <linux/cred.h>
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

void preorderSearch(struct task_struct *sub_root, struct prinfo *buf, int *process_count){
	struct task_struct *first_children;
	struct task_struct *next_sibling;

	first_children = list_entry((&(sub_root->children))->next, struct task_struct, sibling);
	next_sibling = list_entry((&(sub_root->sibling))->next, struct task_struct, sibling);

	(&buf[*process_count])->state = sub_root->state;
	(&buf[*process_count])->pid = sub_root->pid;
	(&buf[*process_count])->parent_pid = sub_root->parent->pid;
	(&buf[*process_count])->first_child_pid = first_children->pid;
	(&buf[*process_count])->next_sibling_pid = next_sibling->pid;
	(&buf[*process_count])->uid = (sub_root->cred->uid).val;
	strcpy((&buf[*process_count])->comm, sub_root->comm);
	*process_count += 1;
	if (first_children->pid > sub_root->pid) {
		preorderSearch(first_children, buf, process_count);
	}
	if (next_sibling->pid > sub_root->pid){
		preorderSearch(next_sibling, buf, process_count);
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
	preorderSearch(&init_task, kernel_buf, &process_count);
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

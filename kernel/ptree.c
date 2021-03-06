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

/* 
 * void preorderSearch(sub_root, buf, process_count_p, max_process_count)
 * Preorder traversal implemented by recursive call
 * A func call recursively call itself on its children, and then on its sibling
 * This correctly implements preorder traversal
 */
void preorderSearch(struct task_struct *sub_root, struct prinfo *buf, int *process_count_p, int max_process_count){
	struct task_struct *first_children;
	struct task_struct *next_sibling;

	first_children = list_entry((&(sub_root->children))->next, struct task_struct, sibling);
	next_sibling = list_entry((&(sub_root->sibling))->next, struct task_struct, sibling);
	
	if (*process_count_p < max_process_count) {
		(&buf[*process_count_p])->state = sub_root->state;
		(&buf[*process_count_p])->pid = sub_root->pid;
		(&buf[*process_count_p])->parent_pid = sub_root->parent->pid;
		(&buf[*process_count_p])->first_child_pid = first_children->pid;
		(&buf[*process_count_p])->next_sibling_pid = next_sibling->pid;
		(&buf[*process_count_p])->uid = (sub_root->cred->uid).val;
		strcpy((&buf[*process_count_p])->comm, sub_root->comm);
		*process_count_p += 1;
		if (first_children->pid > sub_root->pid) {
			preorderSearch(first_children, buf, process_count_p, max_process_count);
		}
		if (next_sibling->pid > sub_root->pid){
			preorderSearch(next_sibling, buf, process_count_p, max_process_count);
		}
	}

}

/* 
 * int sys_ptree(struct prinfo *buf, int *nr)
 * buf: buffer destination for copying process informations in preorder
 * nr: pointer of integer, which informs size of buf
 */
asmlinkage int sys_ptree(struct prinfo *buf, int *nr){
	int max_process_count;			// this will hold the value copied from nr
	int copy_failed_byte;
	int process_count = 0;			// variable to keep tracking # of processes copied
	struct prinfo * kernel_buf;		// this will be copied to buf using copy_to_user

	if (buf == NULL || nr == NULL)
		return -EINVAL;

	if (copy_from_user(&max_process_count, nr, sizeof(max_process_count)))	// max_process_count = *nr
		return -EFAULT;
	if (max_process_count < 1) {
		return -EINVAL;
	}
	if(!access_ok(VERIFY_WRITE, buf, sizeof(struct prinfo) * max_process_count)){
	        return -EFAULT;
	}
	
	kernel_buf = (struct prinfo *)kmalloc(sizeof(struct prinfo) * max_process_count, GFP_KERNEL);
	if (kernel_buf == NULL) {
		return -ENOMEM;
	}
	memset(kernel_buf, 0, sizeof(struct prinfo) * max_process_count);

	read_lock(&tasklist_lock);
	preorderSearch(&init_task, kernel_buf, &process_count, max_process_count);
	read_unlock(&tasklist_lock);
	
	if ((copy_failed_byte = copy_to_user(buf, kernel_buf, sizeof(struct prinfo) * process_count)) != 0) {
		printk("Could not copy %d bytes of prinfo struct from kernel to user\n", copy_failed_byte);
	}
	if ((copy_failed_byte = copy_to_user(nr, &process_count, sizeof(int))) != 0) {
		printk("Could not copy %d bytes of process count from kernel to user\n", copy_failed_byte);
	}

	kfree(kernel_buf);
	return *nr;

}

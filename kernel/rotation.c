#include <linux/kernel.h>
#include <linux/rotation.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/sched.h>

/*
 typedef struct struct __rotation_rane_descriptr {
 	pid_t pid;
	int degree;
	int range;
	int rw_type;	// RT_READ or RT_WRITE
	struct list_head list;
 } rotation_rd;
 */

#define	GET_BEGIN(x, y)		(((x) >= (y)) ? ((x) - (y)) : ((x) - (y) + END_DEGREE))
#define GET_END(x, y)		(((x) + (y)) % END_DEGREE)


DEFINE_MUTEX(base_lock);
DECLARE_WAIT_QUEUE_HEAD(wait_queue);
LIST_HEAD(rotation_rd_list);
int curr_degree = 0;


int is_valid_degree(int begin, int degree, int end) {
	if (degree >= begin && degree <= end) {
		return 1;
	} else {
		if ((degree >= MIN_DEGREE && degree <= begin) || (degree >= begin && degree <= MAX_DEGREE)) {
			return 1;
		}
	}
	return 0;
}

int find_awoken_count(int degree) {
	rotation_rd *rd, *tmp;
	int awoken_cnt = 0;
	int begin;
	int end;
	list_for_each_entry_safe(rd, tmp, &rotation_rd_list, list) {
		begin = GET_BEGIN(rd->degree, rd->range);
		end = GET_END(rd->degree, rd->range);
		if (is_valid_degree(begin, degree, end) == 1) {
			if (rd->rw_type == RT_WRITE) {
				return 1;
			} else if (rd->rw_type == RT_READ) {
				awoken_cnt += 1;
			}
		}
	}
	return awoken_cnt;
}

asmlinkage long sys_set_rotation(int degree)
{
	int process_awoken = 0;
	if (degree > MAX_DEGREE || degree < MIN_DEGREE)
	       return -1;	
	curr_degree = degree;
	mutex_lock(&base_lock);
	wake_up_all(&wait_queue);
	process_awoken = find_awoken_count(degree);
	// printk("===process awoken: %d\n", process_awoken);
	mutex_unlock(&base_lock);
	return process_awoken;
}

asmlinkage long sys_rotlock_read(int degree, int range)
{
	// TODO
	return 0;
}

asmlinkage long sys_rotlock_write(int degree, int range)
{
	// TODO
	return 0;
}

asmlinkage long sys_rotunlock_read(int degree, int range)
{
	// TODO
	return 0;
}

asmlinkage long sys_rotunlock_write(int degree, int rage)
{
	// TODO
	return 0;
}


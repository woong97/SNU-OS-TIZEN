#include <linux/kernel.h>
#include <linux/rotation.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>

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
DECLARE_WAIT_QUEUE_HEAD(wait_queue_cv);

LIST_HEAD(rot_rd_list);
DEFINE_MUTEX(rot_rd_list_mutex);

DEFINE_MUTEX(write_lock);

int curr_degree = 0;
atomic_t read_cnt = ATOMIC_INIT(0);
atomic_t write_cnt = ATOMIC_INIT(0);

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
	rot_rd *rd, *tmp;
	int awoken_cnt = 0;
	int begin;
	int end;
	list_for_each_entry_safe(rd, tmp, &rot_rd_list, list) {
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

static void enqueue_rd(rot_rd *rd) {
	mutex_lock(&rot_rd_list_mutex);
	list_add_tail(&rd->list, &rot_rd_list);
	mutex_unlock(&rot_rd_list_mutex);
}

static rot_rd *create_rd(pid_t pid, int degree, int range, int rw_type) {
	rot_rd *rd = kmalloc(sizeof(rot_rd), GFP_KERNEL);
	rd->pid = pid;
	rd->degree = degree;
	rd->range = range;
	rd->rw_type = rw_type;
	enqueue_rd(rd);
	return rd;
}

void remove_rd(pid_t pid, int degree, int range, int rw_type) {
	rot_rd *rd, *tmp;
	mutex_lock(&rot_rd_list_mutex);
	list_for_each_entry_safe(rd, tmp, &rot_rd_list, list) {
		if(rd->pid == pid && rd->rw_type == rw_type) {
			list_del(&rd->list);
			kfree(rd);
			// printk("hi remove!\n");
			if (rw_type == RT_READ) {
				atomic_sub(1, &read_cnt);
			} else if (rw_type == RT_WRITE) {
				atomic_sub(1, &write_cnt);
			}
			// printk("read_cnt: %d\n", atomic_read(&read_cnt));
			mutex_unlock(&rot_rd_list_mutex);
			return;
		}
	}
	mutex_unlock(&rot_rd_list_mutex);
}

void rot_cv_wait(wait_queue_head_t *wait_queue, struct mutex *mtx) {
	// TODO!!!
	DEFINE_WAIT(wait);
	prepare_to_wait(wait_queue, &wait, TASK_INTERRUPTIBLE);
	mutex_unlock(mtx);
	schedule();
	mutex_lock(mtx);
	finish_wait(wait_queue, &wait);
//	mutex_unlock(mtx);
}

void rot_cv_signal(wait_queue_head_t *wait_queue, int rw_type) {
	// TODO!
	if (rw_type == RT_READ) {
		wake_up(wait_queue);
	} else if (rw_type == RT_WRITE) {
		wake_up_all(wait_queue);
	}
}

static int is_valid_input(int degree, int range) {
	if (degree > MAX_DEGREE || degree < MIN_DEGREE) {
		return -1;
	}
	if (range < 0 || range >= 180) {
		return -1;
	}
	return 0;
}

asmlinkage long sys_set_rotation(int degree)
{
	int process_awoken = 0;
	if (degree > MAX_DEGREE || degree < MIN_DEGREE)
	       return -1;	
	curr_degree = degree;
	mutex_lock(&base_lock);
	// wake_up_all(&wait_queue_cv);
	process_awoken = find_awoken_count(degree);
	wake_up_all(&wait_queue_cv);
	// printk("===process awoken: %d\n", process_awoken);
	mutex_unlock(&base_lock);
	return process_awoken;
}

asmlinkage long sys_rotlock_read(int degree, int range)
{
	rot_rd *rd;
	int valid;
	if (is_valid_input(degree, range) == -1) return -EINVAL;
	// printk("read lock try\n");	
	rd = create_rd(current->pid, degree, range, RT_READ);
	
	mutex_lock(&base_lock);
	while(1) {
		valid = is_valid_degree(GET_BEGIN(rd->degree, rd->range), degree, GET_END(rd->degree, rd->range));
		// printk("valid: %d\n", valid);
		// printk("write cnt: %d\n", atomic_read(&write_cnt));
		if (!mutex_is_locked(&write_lock) &&
			is_valid_degree(GET_BEGIN(rd->degree, rd->range), degree, GET_END(rd->degree, rd->range))) {
			break;
		} else {
			rot_cv_wait(&wait_queue_cv, &base_lock);
		}
	}
	atomic_add(1, &read_cnt);
	// printk("==== read lock occured : %d\n", atomic_read(&read_cnt));
	mutex_unlock(&base_lock);	
	return 0;
}

asmlinkage long sys_rotlock_write(int degree, int range)
{
	// TODO
	rot_rd *rd;
	int valid;
	if (is_valid_input(degree, range) == -1) return -EINVAL;
	// printk(" write lock try!\n");
	rd = create_rd(current->pid, degree, range, RT_WRITE);
	// printk("here1\n");
	mutex_lock(&base_lock);
	// printk("here2\n");
	
	while(1) {
		if (mutex_trylock(&write_lock) &&
			is_valid_degree(GET_BEGIN(rd->degree, rd->range), degree, GET_END(rd->degree, rd->range))) {
			while(atomic_read(&read_cnt) > 0) {
				rot_cv_wait(&wait_queue_cv, &base_lock);
			}
			break;
		} else {
			rot_cv_wait(&wait_queue_cv, &base_lock);
		}
	}

	// printk("here3\n");
	atomic_add(1, &write_cnt);
	mutex_unlock(&base_lock);
	// printk("===write lock occured\n");
	return 0;
}

asmlinkage long sys_rotunlock_read(int degree, int range)
{
	if (is_valid_input(degree, range) == -1) return -EINVAL;
	/* I'm not sure we need this (wait in unlock) */
	///////////////////////////////////////////////
	// printk("===hi unlock read\n");
	mutex_lock(&base_lock);
	while (is_valid_degree(GET_BEGIN(degree, range), degree, GET_END(degree, range)) != 1) {
		printk("this problem occured\n");
		rot_cv_wait(&wait_queue_cv, &base_lock);
	}
	/////////////////////////////////////////////
	
	remove_rd(current->pid, degree, range, RT_READ);
	// mutex_lock(&base_lock);
	
	// some studnet use wake_up as below, some studnet doesn't wake_up
	// Guess: some student thought that delete list is enough???
	// But I thnik we should wake up some process in wait_queue  => I'm not sure
	rot_cv_signal(&wait_queue_cv, RT_READ);
	mutex_unlock(&base_lock);
	//iprintk("unlock succeed %d\n", atomic_read(&read_cnt));
	return 0;
}

asmlinkage long sys_rotunlock_write(int degree, int range)
{
	// TODO
	
	if (is_valid_input(degree, range) == -1) return -EINVAL;
	/* I'm not sure we need this (wait in unlock) */
	///////////////////////////////////////////////
	mutex_lock(&base_lock);
	while (is_valid_degree(GET_BEGIN(degree, range), degree, GET_END(degree, range)) != 1) {
		printk("this problem occured\n");
		rot_cv_wait(&wait_queue_cv, &base_lock);
	}
	/////////////////////////////////////////////
	
	remove_rd(current->pid, degree, range, RT_WRITE);
	// mutex_lock(&base_lock);
	mutex_unlock(&write_lock);
	// some studnet use wake_up as below, some studnet doesn't wake_up
	// Guess: some student thought that delete list is enough???
	// But I thnik we should wake up some process in wait_queue  => I'm not sure
	// In write unlock, wake up all, 
	// In read unlock,  wake up
	rot_cv_signal(&wait_queue_cv, RT_WRITE);
	mutex_unlock(&base_lock);
	// printk("write unlock succeed %d\n", atomic_read(&write_cnt));
	return 0;
}


#include <linux/kernel.h>
#include <linux/rotation.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/sched.h>

DEFINE_MUTEX(base_lock);
DECLARE_WAIT_QUEUE_HEAD(wait_queue);

int curr_degree = 0;

asmlinkage long sys_set_rotation(int degree)
{
	int process_awoken = 0;
	if (degree > MAX_DEGREE || degree < MIN_DEGREE)
	       return -1;	
	mutex_lock(&base_lock);
	curr_degree = degree;
	wake_up_all(&wait_queue);
	// TODO
	// Find count of waiting process
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


/*
 * Weighted Round Robin Scehduling Class
 * */

#include "sched.h"

int sched_wrr_timeslice = WRR_TIMESLICE;
int sysctl_sched_wrr_timeslice = (MSEC_PER_SEC / HZ) * WRR_DEFAULT_TIMESLICE;
int sched_wrr_default_weight = WRR_DEFAULT_WEIGHT;
static DEFINE_PER_CPU(cpumask_var_t, local_cpu_mask);


void __init init_sched_wrr_class(void) {
	unsigned int i;

	for_each_possible_cpu(i) {
		zalloc_cpumask_var_node(&per_cpu(local_cpu_mask, i), GFP_KERNEL, cpu_to_node(i));
	}
	current->wrr.weight = WRR_DEFAULT_WEIGHT;
}


// It is possible to unncessary!
void init_wrr_rq(struct wrr_rq *wrr_rq)
{
	// TODO!!!
	INIT_LIST_HEAD(&(wrr_rq->run_list));
	wrr_rq->weight_sum = 0;
}

// copy from rt.c
static inline struct task_struct *wrr_task_of(struct sched_wrr_entity *wrr_se)
{
	return container_of(wrr_se, struct task_struct, wrr);
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = &(rq->wrr);
	// Q1. Do we need lock???
	// Q2. raw_spin_lock vs rcu_read_lock : which one is correct ?
	
	/* rcu_read_lock(); */
	wrr_se->time_slice = wrr_se->weight * WRR_TIMESLICE;
	list_add_tail(&wrr_se->run_list, &wrr_rq->run_list);
	wrr_rq->weight_sum += wrr_se->weight;
	add_nr_running(rq, 1);
	/* rcu_read_unlock(); */
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = &(rq->wrr);

	// Q1. Do we need lock??
	/* rcu_read_lock(); */
	list_del_init(&wrr_se->run_list);
	wrr_rq->weight_sum -= wrr_se->weight;
	sub_nr_running(rq, 1);
	/* rcu_read_unlock(); */
}

static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = &(rq->wrr);
	// Q1. Do wee need lock???
	/* rcu_read_lock(); */
	wrr_se->time_slice = wrr_se->weight * WRR_TIMESLICE;
	list_move_tail(&wrr_se->run_list, &wrr_rq->run_list);

	/* need check!!!!!!!!  upper is right or comment is right
	 * Maybe, just use list_move tail because it is round-robin!
	 * if (head)
	 * 	list_move(&wrr_se->run_list, &wrr_rq->run_list);
	 * else
	 * 	list_move_tail(&wrr_se->run_list, &wrr_rq->run_list);
	 */

	/* rcu_read_unlock(); */
}
static void yield_task_wrr(struct rq *rq)
{
	// copy from rt.c
	requeue_task_wrr(rq, rq->curr, 0);
}

static void check_preempt_curr_wrr (struct rq *rq, struct task_struct *p, int flags)
{
}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct sched_wrr_entity *next_se = NULL;
	struct task_struct *next_task_struct = NULL;
	struct wrr_rq *wrr_rq = &(rq->wrr);
	
	if (list_empty((&wrr_rq->run_list))) {
		return NULL;
	}

	next_se = list_first_entry(&wrr_rq->run_list, struct sched_wrr_entity, run_list);
	if (!next_se) {
		return NULL;
	}
	next_task_struct = wrr_task_of(next_se);
	next_task_struct->se.exec_start = rq->clock_task;
	next_se->time_slice = next_se->weight * WRR_TIMESLICE;
	return next_task_struct;
	
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *p)
{
}

#ifdef CONFIG_SMP
// very important function
// Some creative idea can be adapted to here
// For now, just move task to seleceted cpu which has the smallest weight sum
static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	struct rq *rq;
	int min_weight_sum;
	int target_cpu = -1;
	/* For anything but wake ups, just return the task_cpu 
	 * Let's test whethere under two lines are helpful or not 
	 */
	if (sd_flag != SD_BALANCE_WAKE && sd_flag != SD_BALANCE_FORK)
		goto out;
	rq = cpu_rq(cpu);

	// initialize min weight sum as current rq's weight_sum
	min_weight_sum = rq->wrr.weight_sum;

	rcu_read_lock();
	for_each_online_cpu(cpu) {
		if (!cpumask_test_cpu(cpu, &p->cpus_allowed))
			continue;
		rq = cpu_rq(cpu);
		if (rq->wrr.weight_sum < min_weight_sum) {
			min_weight_sum = rq->wrr.weight_sum;
			target_cpu = cpu;
		}
	}
	if (target_cpu >= 0) 
		cpu = target_cpu;
	rcu_read_unlock();
out:
	return cpu;
}

static void migrate_task_rq_wrr(struct task_struct *p)
{
}

static void task_woken_wrr(struct rq *this_rq, struct task_struct *task)
{
}

static void set_cpus_allowed_wrr(struct task_struct *p, const struct cpumask *newmask)
{
}

static void rq_online_wrr(struct rq *rq)
{
}

static void rq_offline_wrr(struct rq *rq)
{
}
#endif

static void set_curr_task_wrr(struct rq *rq)
{
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{

	struct sched_wrr_entity *wrr_se = &p->wrr;
	// copy from rt.c
	if (p == NULL)
		return;
	if (p->policy != SCHED_WRR)
		return;
	if (--p->wrr.time_slice)
		return;

	// In rt.c set time_slice again, but we don't have to do this.
	// Because we set time slice at requeue_task_rr
	// Nedd to check my assertion is right
	/* p->wrr.time_slilce = wrr_se->weight * WRR_TIMESLICE; */
	
	// copy from rt.c
	// Q. some people use set_tsk_need_resched instead of resched_curr -> why??
	// set_tsk_need_resched is called inside of resched_curr
	// Key difference is set_preempt_need_resched() which is called in resched_curr
	// I don't know the role of set_preempt_need_resched()
	if (wrr_se->run_list.prev != wrr_se->run_list.next) {
		requeue_task_wrr(rq, p, 0);
		resched_curr(rq);
		return;
	}
}

// Q. Why rt,c don't need task_fork?
static void task_fork_wrr(struct task_struct *p)
{
	// copy from fair.c
	struct sched_wrr_entity *wrr_se = &p->wrr;
	if (p == NULL)
		return;
	// Q. other student didn't use lock. Should we need lock?
	// fair.c use lock - should check this
	// if we use rq_lock in this part like fair.c -> bug occured when we use fork at test.c
	wrr_se->weight = p->parent->wrr.weight;
	wrr_se->time_slice = wrr_se->weight * WRR_TIMESLICE;
}

// Q. Why don't we need task_dead ?
static void task_dead_wrr(struct task_struct *p)
{
}

/* Maybe this is called when task change from wrr scheduler to fair scheduler
 * If task was last of qeuee, pull other wrr task in other runqueu
 * Other people doesn't implement this function(why?...)
 * I'm not sure this is correct. Need check!!!!!!!!!!! Help me!!!
 */

//static void switched_from_wrr(struct rq *rq, struct task_struct *p) {
//}


static void switched_from_wrr(struct rq *rq, struct task_struct *p)
{
	int cpu;
	int hijacked_cpu;
	int max_weight_sum = 0;
	struct rq *hijacked_rq = NULL;
	struct rq *tmp_rq = NULL;
	struct task_struct *hijacked_task;
	struct sched_wrr_entity *hijacked_wrr_se;
	bool resched = false;
	// int dequee_flags = DEQUEUE_SAVE | DEQUEUE_MOVE | DEQUEUE_NOCLOCK;

	// If some task already exsits in rq, just return
	if (!task_on_rq_queued(p) || (rq->wrr.weight_sum > 0))
		return;
	// Pull other task in other wrr runqueue
	// I'm not sure we need lock
	rcu_read_lock();
	for_each_online_cpu(cpu) {
		if (!cpumask_test_cpu(cpu, &p->cpus_allowed))
			continue;
		tmp_rq = cpu_rq(cpu);
		double_lock_balance(rq, tmp_rq);
		// I want to find runqeue which has more than 1 task
		// tmp_rq->curr->wrr.weight != tmp_rq->wrr.weight_sum : if current task's weight = runqueue's weight_sum, 
		// It means that this runqueue has only one task
		if ((tmp_rq->wrr.weight_sum > max_weight_sum) && (tmp_rq->curr->wrr.weight != tmp_rq->wrr.weight_sum)) {
			max_weight_sum = tmp_rq->wrr.weight_sum;
			hijacked_rq = tmp_rq;
			hijacked_cpu = cpu;
			resched = true;
		}
		double_unlock_balance(rq, tmp_rq);
	}
	rcu_read_unlock();

	if (resched) {
		// I want to find current task's next task in hijected runqueue.
		// The reason why I pick next (not chossing highest weight task) is that
		// I want to decrease searching cos : It's my imagination
		hijacked_wrr_se = container_of(hijacked_rq->curr->wrr.run_list.next, struct sched_wrr_entity, run_list);
		hijacked_task = wrr_task_of(hijacked_wrr_se);
		deactivate_task(hijacked_rq, hijacked_task, 0);
		set_task_cpu(hijacked_task, hijacked_cpu);
		activate_task(rq, hijacked_task, 0);
		resched_curr(rq);
	}
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	wrr_se->weight = WRR_DEFAULT_WEIGHT;
	wrr_se->time_slice = WRR_DEFAULT_TIMESLICE;
}

static void prio_changed_wrr(struct rq *this_rq, struct task_struct *task, int oldprio)
{
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task)
{
	if (task->policy == SCHED_WRR)
		return task->wrr.weight * WRR_TIMESLICE;
	else
		return 0;
}

static void update_curr_wrr(struct rq *rq)
{
}

void trigger_load_balance_wrr(struct rq *rq)
{
	// TODO!!!
}

const struct sched_class wrr_sched_class = {
	.next			= &fair_sched_class,
	.enqueue_task		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
	.yield_task		= yield_task_wrr,

	.check_preempt_curr	= check_preempt_curr_wrr,
	
	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

#ifdef	CONFIG_SMP
	.select_task_rq		= select_task_rq_wrr,
	.migrate_task_rq	= migrate_task_rq_wrr,

	.set_cpus_allowed	= set_cpus_allowed_common,
	.rq_online		= rq_online_wrr,
	.rq_offline		= rq_offline_wrr,
	.task_woken		= task_woken_wrr,
	.switched_from		= switched_from_wrr,
#endif

	.set_curr_task		= set_curr_task_wrr,
	.task_tick		= task_tick_wrr,
	.task_fork		= task_fork_wrr,
	.task_dead		= task_dead_wrr,
	.get_rr_interval	= get_rr_interval_wrr,
	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,

	.update_curr		= update_curr_wrr,
};

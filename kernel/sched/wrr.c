/*
 * Weighted Round Robin Scehduling Class
 * */

#include "sched.h"

int sched_wrr_timeslice = WRR_TIMESLICE;
int sysctl_sched_wrr_timeslice = (MSEC_PER_SEC / HZ) * WRR_DEFAULT_TIMESLICE;
int sched_wrr_default_weight = WRR_DEFAULT_WEIGHT;

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
	wrr_se->time_slice = wrr_se->weight * sched_wrr_timeslice;
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
	
	/* rcu_read_lock() */
	list_del_init(&wrr_se->run_list);
	wrr_rq->weight_sum -= wrr_se->weight;
	sub_nr_running(rq, 1);
	/* rcu_read_unlock()*/
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

	/* rcu_read_unlock() */

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
	next_task_struct = wrr_task_of(next_se);
	next_task_struct->se.exec_start = rq->clock_task;
	next_se->time_slice = next_se->weight * WRR_TIMESLICE;
	return next_task_struct;
	
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *p)
{
}

#ifdef CONFIG_SMP
static int select_task_rq_wrr(struct task_struct *p, int task_cpu, int sd_flag, int flags)
{
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
}

static void switched_from_wrr(struct rq *this_rq, struct task_struct *task)
{
}

static void switched_to_wrr(struct rq *this_rq, struct task_struct *task)
{
}

static void prio_changed_wrr(struct rq *this_rq, struct task_struct *task, int oldprio)
{
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task)
{
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
	
	.set_cpus_allowed	= set_cpus_allowed_common,
	.rq_online		= rq_online_wrr,
	.rq_offline		= rq_offline_wrr,
	.task_woken		= task_woken_wrr,
	.switched_from		= switched_from_wrr,
#endif

	.set_curr_task		= set_curr_task_wrr,
	.task_tick		= task_tick_wrr,

	.get_rr_interval	= get_rr_interval_wrr,
	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,

	.update_curr		= update_curr_wrr,
};

/*
 * Weighted Round Robin Scehduling Class
 * */

#include "sched.h"

int sched_wrr_timeslice = WRR_TIMESLICE;
int sysctl_sched_wrr_timeslice = (MSEC_PER_SEC / HZ) * WRR_TIMESLICE;
int sched_wrr_default_weight = WRR_DEFAULT_WEIGHT;

// It is possible to unncessary!
void init_wrr_rq(struct wrr_rq *wrr_rq)
{
	// TODO!!!
	INIT_LIST_HEAD(&(wrr_rq->list));
	wrr_rq->weight_sum = 0;
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static void yield_task_wrr(struct rq *rq)
{
}

static void check_preempt_curr_wrr (struct rq *rq, struct task_struct *p, int flags)
{
}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev,struct rq_flags *rf)
{
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

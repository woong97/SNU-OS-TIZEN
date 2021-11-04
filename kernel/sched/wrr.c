/*
 * Weighted Round Robin Scehduling Class
 * */

#include "sched.h"
#include <linux/jiffies.h>

#define WRR_EMPTY_CPU	0

static spinlock_t wrr_lb_lock;
static unsigned long prev_jiffies;

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
	prev_jiffies = jiffies;
}


void init_wrr_rq(struct wrr_rq *wrr_rq)
{
	// TODO!!!
	INIT_LIST_HEAD(&(wrr_rq->run_list));
	wrr_rq->weight_sum = 0;
	spin_lock_init(&wrr_lb_lock);
}
#ifdef CONFIG_SCHED_DEBUG
void print_wrr_stats(struct seq_file *m, int cpu)
{
	struct rq *rq = cpu_rq(cpu);
	struct wrr_rq *wrr_rq = &rq->wrr;
	rcu_read_lock();
	print_wrr_rq(m, cpu, wrr_rq);
	rcu_read_unlock();
}
#endif

static inline struct task_struct *wrr_task_of(struct sched_wrr_entity *wrr_se)
{
	return container_of(wrr_se, struct task_struct, wrr);
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = &(rq->wrr);
	rcu_read_lock();
	list_add_tail(&wrr_se->run_list, &wrr_rq->run_list);
	wrr_se->time_slice = wrr_se->weight * WRR_TIMESLICE;
	wrr_rq->weight_sum += wrr_se->weight;
	
	add_nr_running(rq, 1);
	rcu_read_unlock();
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = &(rq->wrr);

	list_del_init(&wrr_se->run_list);
	wrr_rq->weight_sum -= wrr_se->weight;
	sub_nr_running(rq, 1);
}

static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = &(rq->wrr);
	
	wrr_se->time_slice = wrr_se->weight * WRR_TIMESLICE;
	list_move_tail(&wrr_se->run_list, &wrr_rq->run_list);
}
static void yield_task_wrr(struct rq *rq)
{
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
static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	struct rq *rq;
	int min_weight_sum;
	int target_cpu = -1;
	if (sd_flag != SD_BALANCE_WAKE && sd_flag != SD_BALANCE_FORK)
		goto out;
	rq = cpu_rq(cpu);

	// initialize min weight sum as current rq's weight_sum
	min_weight_sum = rq->wrr.weight_sum;

	rcu_read_lock();
	for_each_online_cpu(cpu) {
		if (cpu == WRR_EMPTY_CPU)
			continue;
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
	if (p == NULL)
		return;
	if (p->policy != SCHED_WRR)
		return;
	if (--wrr_se->time_slice > 0)
		return;

	requeue_task_wrr(rq, p, 0);
	resched_curr(rq);
	return;
}

static void task_fork_wrr(struct task_struct *p)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	if (p == NULL)
		return;
	wrr_se->weight = p->parent->wrr.weight;
	wrr_se->time_slice = wrr_se->weight * WRR_TIMESLICE;
}

static void task_dead_wrr(struct task_struct *p)
{
}

/* Maybe this is called when task change from wrr scheduler to fair scheduler
 * If task was last of qeuee, pull other wrr task in other runqueu
 */
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

	if (!task_on_rq_queued(p) || (rq->wrr.weight_sum > 0))
		return;
	rcu_read_lock();
	for_each_online_cpu(cpu) {
		if (cpu == WRR_EMPTY_CPU)
			continue;
		if (!cpumask_test_cpu(cpu, &p->cpus_allowed))
			continue;
		tmp_rq = cpu_rq(cpu);
		double_lock_balance(rq, tmp_rq);
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
		set_task_cpu(hijacked_task, cpu_of(rq));
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

static void __trigger_load_balance_wrr(void)
{
	int cpu;
	struct rq *tmp_rq;
	unsigned int tmp_weight_sum;

	struct rq *src_rq = NULL;
	struct rq *trg_rq = NULL;
	int max_weight_sum = -1;
	int min_weight_sum = -1;

	int max_weight = 0;
	int tmp_weight = 0;
	struct sched_wrr_entity *tmp_wrr_se;
	struct task_struct *src_p = NULL;
	struct task_struct *tmp_p;
	
	rcu_read_lock();
	for_each_online_cpu(cpu) {
		if (cpu == WRR_EMPTY_CPU)
			continue;
		tmp_rq = cpu_rq(cpu);
		tmp_weight_sum = tmp_rq->wrr.weight_sum;
		if (max_weight_sum == -1 && min_weight_sum == -1) {
			max_weight_sum = tmp_weight_sum;
			min_weight_sum = tmp_weight_sum;
			src_rq = tmp_rq;
			trg_rq = tmp_rq;
		} else {
			if (tmp_weight_sum > max_weight_sum) {
				max_weight_sum = tmp_weight_sum;
				src_rq = tmp_rq; 
			}

			if (tmp_weight_sum < min_weight_sum) {
				min_weight_sum = tmp_weight_sum;
				trg_rq = tmp_rq;
			}
		}

	}
	rcu_read_unlock();

	if (src_rq == NULL || trg_rq == NULL) {
		return;
	}
	if (src_rq == trg_rq) {
		return;
	}
	double_rq_lock(src_rq, trg_rq);
	list_for_each_entry(tmp_wrr_se, &(src_rq->wrr.run_list), run_list) {
		tmp_p = wrr_task_of(tmp_wrr_se);
		tmp_weight = tmp_p->wrr.weight;

		if ((src_rq->curr != tmp_p) && 
			(src_rq->wrr.weight_sum - tmp_weight >= trg_rq->wrr.weight_sum + tmp_weight) && (tmp_weight > max_weight)) {
			src_p = tmp_p;
			max_weight = tmp_weight;
		}
	}
	double_rq_unlock(src_rq, trg_rq);

	if (src_p == NULL)
		return;

	deactivate_task(src_rq, src_p, 0);
	set_task_cpu(src_p, cpu_of(trg_rq));
	activate_task(trg_rq, src_p, 0);
	resched_curr(trg_rq);
}

void trigger_load_balance_wrr()
{
	unsigned long flags;
	spin_lock_irqsave(&wrr_lb_lock, flags);
	if(time_after_eq(jiffies, prev_jiffies + 2 * HZ)) {
		__trigger_load_balance_wrr();
		prev_jiffies = jiffies;
	}
	spin_unlock_irqrestore(&wrr_lb_lock, flags);
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

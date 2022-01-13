# [Project 2] Team 8 implementation document
## 1. Data Structure
### 1.1 struct sched_class
- kernel/sched/sched.h에 struct sched_class가 정의되어 있고, deadline.c에 const struct sched_class dl_sched_class가, rt.c에 const struct sched_class rt_sched_class가, fair.c에 const struct sched_class fair_sched_class가 정의되어 있다. 우리는 wrr.c에 const struct sched_class wrr_sched_class를 적절한 형태로 정의해야 한다
### 1.2 struct wrr_rq, struct sched_wrr_entity
- wrr scheduler를 만들기 위해 wrr_rq, sched_wrr_entity 자료구조를 선언해야 한다.
- wrr_rq는 wrr scheduler가 돌아갈 runqueue로 아래와 같이 선언하여 struct rq에 wrr_rq를 추가해줘야 한다.
- wrr_rq 내에서 관리될 sched_wrr_entity 자료구조를 선언해줘야 한다. 이 entity는 process 하나당 하나씩 관리되는 것으로 이 곳에서 각 process의 weight를 선언해주는 것이고, wrr_rq의 weight_sum에 더해주게 된다
- sched_wrr_entity를 struct task_struct field에 추가해주고, 이 entity들이 wrr_rq의 run_list와 list로 연결되어 관리된다.
``` C
# In include/linux/sched.h
struct sched_wrr_entity {
	struct list_head		run_list;
	unsigned int			time_slice;
	unsigned int			weight;
};

```

``` C
# In kernel/sched/sched.h
struct wrr_rq {
	struct list_head run_list;
	int weight_sum;
};
```
## 2. Implementation

### 2.1 Design
- task 마다 1~20 사이의 weight를 세팅해주고 이 범위를 넘어가는 weight를 set 하려 하면 에러를 리턴한다. default weight는 10으로, process가 fork() 되자마자 직후의 weight는 이 default값인 10으로 설정 돼 있다.
- task의 time slice는 weight * 10ms 로 설정 되고 해당 시간만큼만 run한뒤 스케줄링 된다.
- include/linux/sched/wrr.h 파일에 constant 변수에 대한 매크로 설정

``` C
#define WRR_TIMESLICE		(10 * HZ / 1000)
#define WRR_DEFAULT_WEIGHT	10
#define WRR_DEFAULT_TIMESLICE	(WRR_TIMESLICE * WRR_DEFAULT_WEIGHT)
#define WRR_MAX_WEIGHT		20
#define WRR_MIN_WEIGHT		1
``` 

### 2.2 cosnt struct sched_class wrr_sched_class
- kernel/sched/wrr.c 안에 아래와 같이 wrr_sched_class를 선언해주고, 필요한 method들을 구현해준다. 모두다 구현할 필요는 없다
- 필수 mehtod들을 구현해주어야, 예를 들어 kernel/sched/core.c에서 enqueue_task를 하면 해당 process가 돌고 있는 scheduler class가 wrr일 때 enqueue_task_wrr로 연결되게 된다
- 중요한 점은 .next = &fair_sched_class로 선언해줘야 된다는 점이다. rt.c에서 .next가 &fair_sched_class로 돼 있는데, 이를 &wrr_sched_class로 변경해줘야 우리가 원하는 순서대로(rt -> wrr -> fair) hierarchy가 갖춰진다 

``` C
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
```
- enqueue_task: task를 해당 sched class에 대응하는 runqueue로 넣어준다
- dequeue_task: task를 해당 runqueue에서 빼준다
- yield_task: 프로세스가 yield() 할 경우 task를 곧바로 run_list의 마지막으로 다시 넣어준다
- select_task_rq: task를 어떤 rq에 넣어줄지 해당 cpu를 찾아서 리턴해주는데, 현재 weight_sum이 가장 낮은 cpu를 선정해준다
- switched_from_rq: task가 wrr class에서 fair class로 전환될 때 호출되는 함수이다. 만약 그 task가 본인이 속한 runqueue에서 마지막 task 였을 때 다른 runqueue에 있는 task를 비어있는 runqueue로 옮겨준다
- task_fork: fork가 될때 호출되는 함수로, child의 weight는 parent의 weight를 물려받는다.
- switched_to: task가 rt class에서 wrr class로 전환될때 호출 되는 함수이다.
- pick_next_task: 한 프로세스가 time slice가 끝났거나, reschedule이 필요할 때 다음으로 스케줄링 될 task를 뽑는다
- task_tick: tick이 될 때마다 이 함수가 호출되고 해당 task의 time slice가 다 소진됐는지 확인한다

- 아래 구현한 코드에서 호출이 되지 않고 있지만, 본인은 switched_from_rq_wrr가 , 호출될 경우를 대비해 조금 독특하게 구현해보았다.
- 어떤 runqueue의 마지막 task가 fair로 switch 되면서, 다른 runqueue에서 task를 가져와야 하는데, weight_sum이 가장 큰 runqueue를 찾고, 해당 runqueue의 current task의 next task를 선택하였다. weight가 가장 큰 걸 옮기는 것이 best이겠지만, searching cost를 줄이기 위해 이렇게 구현하였다
``` C
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
		// I want to decrease searching cost : It's my imagination
		hijacked_wrr_se = container_of(hijacked_rq->curr->wrr.run_list.next, struct sched_wrr_entity, run_list);
		hijacked_task = wrr_task_of(hijacked_wrr_se);
		deactivate_task(hijacked_rq, hijacked_task, 0);
		set_task_cpu(hijacked_task, cpu_of(rq));
		activate_task(rq, hijacked_task, 0);
		resched_curr(rq);
	}
}
```
### 2.3 System Call(sys_sched_setweight, sys_sched_getweight)
- wrr scheduler로 돌릴 task의 weight를 set해주고, get 해주는 system call을 추가해줘야 합니다. proj1과 같은 방식으로 추가하되, 주요 routine은 kernel/sched/core.c에서 구현합니다
- proj2 documentation에서 weight를 set해줄 수 있는 조건이 있는데 이 조건에서만 weight를 변경할 수 있게 구현해 줘야 합니다.

### 2.4 Load Balance
- kernel/sched/core.c scheduler_tick() 함수에서 load balance가 trigger 됩니다. 기존의 trigger_load_balance는 fari.c에 구현 돼 있습니다. 우리는 비슷하게 trigger_load_balance_wrr()를 wrr.c에서 구현해 줍니다.
- 2000ms 마다 load balance를 시도합니다. 정확한 시간을 기록하기 위해, lock을 하나 선언해서, tirgger_load_balance 될때마다 이전 trigger 됐을 때보다 2000ms가 지났는지 확인해줍니다
- weight_sum이 가장 큰 runqueue에서 가장 작은 runqueue로 옮겨주는데, weight가 가장 크면서 옮겼을때 weight_sum의 대소관계가 바뀌지 않는 task를 찾아서 옮겨주고 없으면 옮겨주지 않는다.
``` C
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

``` 
### 2.5 Set CPU which doesn't use wrr schedulder

- cpu 중 반드시 하나는 wrr_rq의 list가 empty여야 한다. 아래와 같이 cpu0번으로 선정한다.
``` C
#define WRR_EMPTY_CPU	0
``` 
- kernel/sched/core.c "__sched_setscheduler" 함수가 호출 될 떄, sched_class가 wrr이고, rq의 cpu가 우리가 선정한 0번 cpu일 경우에는 다른 cpu로 연결된 wrr runqueue에 task가 들어갈 수 있게 migrate 해준다 
- migrate_task_to_wrr_existed_cpu라는 함수를 만들고, cpu를 순회하면서 0번 cpu가 아닌경우에는 cpumask_set_cpu를 해준다
- 그리고 아래 함수는 __sched_setscheduler 시작 부분에서 policy가 wrr인 경우에 호출해준다
- 또한 wrr.c 에서 cpu를 순회하면서 원하는 cpu를 찾아야 되는 부분에서 역시, cpu 0인 경우에는 continue를 하여 skip 해줬다.

``` C
static struct rq *migrate_task_to_wrr_existed_cpu(struct rq *rq, struct task_struct *p)
{
	int dest_cpu;
	int cpu;
	struct rq *tmp_rq;
	struct cpumask mask;
	//const struct cpumask *no_use_cpu_mask = get_cpu_mask(WRR_EMPTY_CPU);
	
	struct rq_flags rf;
	int min_weight_sum = 100000;
	rcu_read_lock();
	for_each_online_cpu(cpu) {
		if (cpu == WRR_EMPTY_CPU)
			continue;
		if (!cpumask_test_cpu(cpu, &p->cpus_allowed))
			continue;
		cpumask_set_cpu(cpu, &mask);
		tmp_rq = cpu_rq(cpu);
		if (tmp_rq->wrr.weight_sum < min_weight_sum) {
			min_weight_sum = tmp_rq->wrr.weight_sum;
			dest_cpu = cpu;
		}
	}
	rcu_read_unlock();
	if (dest_cpu == WRR_EMPTY_CPU) {
		return;
	}

	
	sched_setaffinity(p->pid, &mask);
	
	if (cpu_of(rq) == WRR_EMPTY_CPU) {
		update_rq_clock(rq);
		if (task_running(rq, p) || p->state ==TASK_WAKING) {
			struct migration_arg arg = {p, dest_cpu};
			//task_rq_unlock(rq, p, &rf);
			stop_one_cpu(cpu_of(rq), migration_cpu_stop, &arg);
			tlb_migrate_finish(p->mm);
			return rq;
		} else if (task_on_rq_queued(p)) {
			rq = move_queued_task(rq, &rf, p, dest_cpu);
		}
		
		
		// task_rq_unlock(rq, p, &rf);
		// cpumask_set_cpu(1, &mask);
		// cpumask_set_cpu(2, &mask);
		// cpumask_set_cpu(3, &mask);
		// sched_setaffinity(p->pid, &mask);
	}

	return rq;

}
.
.
.
``` 
## 3. Test
- test file은 tests 디렉토리 안에, test_weight.c, test_fork.c를 만들어 뒀다.
- tes_weight는 단일 프로세스를 돌리는 것으로, weight값과, factorization 할 숫자를 input으로 받는다
- test_fork는 wrr scheduler로 setting해줄 여러개의 child process를 만들고 실행한다. fork할 process 개수와, factoriazation 할 숫자를 input으로 받는다
``` 
#실행 형식
$ ./test_weight $(weight) %(number)
$ ./test_fork $(num_process) $(number)
``` 
- process를 wrr class로 돌리기 위해, 156(process의 policy를 wrr scheduler로 세팅), 398(우리가 구현한 sys_sched_setweight), 399(우리가 구현한 sys_sched_getweight)번의 시스템콜을 사용한다.
``` C
int weight = atoi(argv[1]);
int num = atoi(argv[2]);

if (syscall(SET_SCHEDULER_SYS_NUM, getpid(), SCHED_WRR, &param)) {
  perror("set scheduler failed!");
  exit(EXIT_FAILURE);
}
if (syscall(SCHED_SETWEIGHT_SYS_NUM, getpid(), weight) < 0) {
  perror("setweight failed");
  exit(EXIT_FAILURE);
}
``` 
- 첨부한 plot.pdf는, test_weight 프로그램을 돌린 것으로, weight를 변경함에 따라서 소요시간에 유의미한 차이가 있어보이지 않는다. input number로는 아주 큰 소수인 100000049를 집어넣었다.
- test_fork로 random한 weight으로 실험해보았을 때도, weight의 크기에 따른 소요시간에 차이가 보이지 않았다.
- 아마 코드에 문제가 있어서 소요시간에 차이가 없는 것으로 생각하는데, 어떤 문제가 있는지 파악하지 못했다.

## 4. Improvement of WRR Scheduler
- EXTRA.md에 아이디어들을 적어놓았습니다

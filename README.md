# [Project 3] Team 8 implementation document

## 1.Main Items(data sturucture and variables)
### 1.1 Rotation_range_descriptor
```C
typedef struct __rotation_range_descriptor {
	pid_t pid;
	int degree;
	int range;
	int rw_type;
	struct list_head list;
} rot_rd;		// defined in include/linux/rotation.h

LIST_HEAD(rot_rd_list); // defined in kernel/rotation.c
```
- 이번 프로젝트에서 가장 주요한 자료구조 입니다. 위를 include/linux/rotation.h 파일에 정의해줍니다
- lock을 잡으려고 시도하는 모든 process들을 하나의 queue에 넣어줘서 관리해줘야 하기 때문에, pid_t, struct list_head field가 존재합니다. 그리고 연결 리스트로 연결하기 위해 LIST_HEAD를 만들어줍니다. 해당 프로세스가 read lock을 잡으려 했는지, write lock을 잡으려 했는지 알기 위해 int rw_type이라는것도 정의해줍니다. 

### 1.2 WAIT_QUEUE
```C
DECLARE_WAIT_QUEUE_HEAD(wait_queue_cv);
```
- process들을 sleep, wake하기 위해 위처럼 wai_queue_cv를 선언합니다

### 1.3 Mutex
```C
DEFINE_MUTEX(base_lock);
DEFINE_MUTEX(rot_rd_list_mutex);
```
- Mutual exclusive가 필요한 기본적인 작업에서의 lock을 선언하기 위해 base_lock을 선언합니다.
- rot_rd_list를 list_for_each_entry로 돌때 혹은 list에 넣어줄때를 위해 rot_rd_list_mutex라는 lock을 선언합니다. 

### 1.4 Atomic Variables
```C
atomic_t read_cnt = ATOMIC_INIT(0);
atomic_t write_cnt = ATOMIC_INIT(0);
```
- read lock이 몇개 잡혔는지, write lock을 잡으려고 시도했는지 추적하기 위해 atomic 변수를 선언합니다. read_cnt와 write_cnt를 어디서 증가시켜주는지가 write lock의 starvation을 막는 핵심적인 아이디어가 됩니다.
### 1.5 
```C
struct task_struct *write_first_queued = NULL;
```
- write lock을 잡으려고 시도한 task를 추적하기 위해 위와 같은 global 변수를 선언합니다.

## 2. Implementation
### 2.1 System Call 추가
- sys_set_rotation, sys_rotlock_read, sys_rotlock_write, sys_rotunlock_read, sys_rotunlock_write 총 5개의 시스템콜을 추가해줍니다.시스템 콜을 추가하는 과정은 proj1, proj2와 동일하므로 생략하겠습니다.
### 2.2 Code 중복을 막기 위한 주요 함수들
```C
#define	GET_BEGIN(x, y)		(((x) >= (y)) ? ((x) - (y)) : ((x) - (y) + END_DEGREE))
#define GET_END(x, y)		(((x) + (y)) % END_DEGREE)

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
```
- 반복된 계산이 들어가는 부분을 매크로로 만들었습니다
- lock을 잡으려는 process의 degree와 range 범위와 sys_set_rotation에 의해 set된 degree 가 범위에 맞게 들어오는지 계속 확인해줘야 하는데 해당 함수를 위와 같이 만들었습니다.

### 2.3 sys_set_rotation
```C
asmlinkage long sys_set_rotation(int degree)
{
	int process_awoken = 0;
	if (degree > MAX_DEGREE || degree < MIN_DEGREE)
	       return -1;	
	curr_degree = degree;
	mutex_lock(&base_lock);
	process_awoken = find_awoken_count(degree);
	wake_up_all(&wait_queue_cv);
	mutex_unlock(&base_lock);
	return process_awoken;
}
```
- global 변수로 선언한 curr_degree를 새로 set할 degree로 바꿔주고, process 들을 깨우고 깨워야할 process 개수를 계산하고 그 값을 return 합니다.
```C
int find_awoken_count(int degree) {
	rot_rd *rd, *tmp;
	int awoken_cnt = 0;
	list_for_each_entry_safe(rd, tmp, &rot_rd_list, list) {
		if (is_valid_degree(GET_BEGIN(rd->degree, rd->range), degree, GET_END(rd->degree, rd->range)) == 1) {
			if (rd->rw_type == RT_WRITE) {
				return 1;
			} else if (rd->rw_type == RT_READ) {
				awoken_cnt += 1;
			}
		}
	}
	return awoken_cnt;
}
```
- 깨워야 할 process 개수를 찾을 핵심함수로, 연결리스트를 돌면서 연결 list에 write lock을 잡으려 한 process가 있다면 바로 1을 return하고 없다면 총 read lock을 잡으려 하는 process의 개수를 찾아줍니다.

### 2.4 sys_rotlock_read
```C
rd = create_rd(current->pid, degree, range, RT_READ);
mutex_lock(&base_lock);
while(1) {
  if ((atomic_read(&write_cnt) == 0) &&
    is_valid_degree(GET_BEGIN(rd->degree, rd->range), curr_degree, GET_END(rd->degree, rd->range))) {
    break;
  } else {
    rotlock_sleep(&wait_queue_cv, &base_lock);
  }
}
atomic_add(1, &read_cnt);
mutex_unlock(&base_lock);	
```
- 함수에서 중요한 부분만 가져왔습니다. "write lock을 잡으려고 시도하는 task가 없는지 (sleep 됐다가 꺠어나서도 이 조건을 계속 확인하기 때문에 write starvation을 막을 수 있습니다)", degree가 적절한 범위에 있을때만 while문을 빠져나와 read_cnt를 증가켜주고 read lock을 잡을 수 있습니다.
- create_rd라는 함수는 해당 task를 통해 rotation descriptor 구조체를 만들어주고 rot_rd_list 연결리스트에 넣어주는 함수입니다

### 2.5 sys_rotlock_write
```C
rd = create_rd(current->pid, degree, range, RT_WRITE);
mutex_lock(&base_lock);
while(1) {
  if ((atomic_read(&write_cnt) == 0) &&
    is_valid_degree(GET_BEGIN(rd->degree, rd->range), curr_degree, GET_END(rd->degree, rd->range))) {
    atomic_add(1, &write_cnt);
    write_first_queued = current;	
    while(atomic_read(&read_cnt) > 0) {
      rotlock_sleep(&wait_queue_cv, &base_lock);
    }
    break;
  } else {
    rotlock_sleep(&wait_queue_cv, &base_lock);
  }
}
mutex_unlock(&base_lock);
```
- 함수에서 중요한 부분만 가져왔습니다. read lock과 달리, write lock을 잡으려 시도하자마자 write_cnt를 1 증가시켜줍니다. 이곳에서 write_cnt를 증가시켜줘야 dead lock을 막을 수 있고, write lock의 starvation을 막습니다. 이것 덕분에, write lock을 잡으려다 실피해서 sleep된 task보다 뒤에(늦게) read lock을 잡으려 시도하는 process들은 lock을 잡지 못하고 이 write lock이 해결 될 때까지 sleep하게 됩니다.
- write_first_queued에 current task를 집어넣어줍니다. read unlock에서 write lock을 잡으려고 sleep했던 task가 있다면 이를 기억하고 이 task를 꺠워주기 위해 사용합니다.
- write lock 보다 먼저 read lock을 잡고있는 task들이 lock을 놓을때까지 while문을 돌며 wait합니다.
- create_rd라는 함수는 해당 task를 통해 rotation descriptor 구조체를 만들어주고 rot_rd_list 연결리스트에 넣어주는 함수입니다


### 2.6 sys_rotunlock_read, sys_rotunlock_write
```C
asmlinkage long sys_rotunlock_read(int degree, int range)
{
  ... 생략 ...
	remove_rd(current->pid, degree, range, RT_READ);
	if (atomic_read(&write_cnt) > 0 && atomic_read(&read_cnt) == 0){
		wake_up_process(write_first_queued);
	} else {
		wake_up_all(&wait_queue_cv);
	}
	mutex_unlock(&base_lock);
	return 0;
}

asmlinkage long sys_rotunlock_write(int degree, int range)
{
	... 생략 ...
	remove_rd(current->pid, degree, range, RT_WRITE);
	wake_up_all(&wait_queue_cv);
	mutex_unlock(&base_lock);
	return 0;
}
```
- unlock의 경우 read와 write가 비슷합니다. rot_rd_list(연결 list) 에서 해당 object를 제거해주고, 적절히 process들을 깨워주는데, write unlock의 경우 간편하게 잠들어있는 모든 process를 깨워주고, read unlock의 경우 대기중인 write lock이 있는 경우에만 아까 이 task를 기억하기위해 선언해준 write_first_queued를 특별히 꺠워줍니다.
- remove_rd함수는 해당 task로 만들어진 rotation descriptor 구조체를 rot_rd_list 연결리스트에서 제거해주고, write_cnt 또는 read_cnt를 1 감소시켜줍니다.

## 3. Test
- 실행의 편의를 위해 run.sh script 파일을 만들었습니다

```C
# run.sh
echo "mount -o rw,remount /dev/mmcblk0p2 /"
mount -o rw,remount /dev/mmcblk0p2 /
echo "./rotd &"
./rotd &
echo "./selector 7289 $1 &"
./selector 7289 $1 &
echo "./trial 1 $1 & ./trial 2 $1 & ./trial 3 $1"
./trial 1 $1 & ./trial 2 $1 & ./trial 3 $1
```

- 실행은 "bash run.sh" 또는 "bash run.sh slow" 로 실행시킵니다. slow는 read랑 write를 좀 더 천천히 수행하게끔 하는 모드입니다. sleep mode의 경우 trial.c, selector.c에서 while loop를 돌 때 수행을 끝내고 unlock 한 뒤 sleep함수를 호출합니다. sleep() 함수를 사용하지 않으면 number가 너무 빨리 증가해서, slow mode 라는 것을 만들어 shell script의 input argument에 'slow'를 추가해주면 slow mode로 실행시킵니다

```C
root:~> bash run.sh slow
./rotd &
./selector 7289 slow &
./trial 1 slow & ./trial 2 slow & ./trial 3 slow & 
Running mode is 'slow mode'
Running mode is 'slow mode'
Running mode is 'slow mode'
trial-1: 7289 = 37 * 197
trial-3: 7289 = 37 * 197
trial-2: 7289 = 37 * 197
trial-1: 7290 = 2 * 3 * 3 * 3 * 3 * 3 * 3 * 5
trial-3: 7290 = 2 * 3 * 3 * 3 * 3 * 3 * 3 * 5
trial-2: 7290 = 2 * 3 * 3 * 3 * 3 * 3 * 3 * 5
trial-2: 7291: 23 * 317
trial-1: 7291: 23 * 317
trial-3: 7291: 23 * 317
.
.
.
```

- 주의사항: tizen shell내에서 slow mode와 default mode를 두개 다 테스트 하고 싶을 때 수행 순서를 맞춰줘야 합니다. slow mode 테스트 -> slow mode로 실행시킨 process들 kill -> default mode 테스트
- 이 순서가 반대가 된다면, process들이 잘 killed되지 않습니다. 이 순서를 맞춰주지 않으면 특정 mode로 테스트 한뒤, qemu.sh를 종료하고 다시 재실행해서 테스트 해줘야 합니다. qemu.sh를 재실행해서 두번 테스트 하고 싶지 않을 때 이 순서대로 테스트대로 테스트하면 됩니다.
```C
root:~> bash run.sh slow
Ctrl+C...
root:~> bash kill.sh
root:~> bash run.sh
```

```C
# kill.sh
pkill -f "selector"
pkill -f "trial"
pkill -f "rotd"
```

## 4. Lessons learned
- 실제 개발할 때 read lock, write lock을 많이 사용하는데, 이를 직접 구현해보고 원리를 확실히 알 수 있어서 뜻 깊은 시간이었다.
- write lock의 starvation을 어떻게 막을 수 있을지 고민하는 과정이 매우 즐거웠고, 내가 한 방법 말고도 더 다양하게 시도해볼 수 있을 것 같다.  

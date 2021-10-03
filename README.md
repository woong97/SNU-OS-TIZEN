# [Project 1] Team 8 implementation document

## 1. tasklist
이하에서 tasklist는 `struct task_struct`타입의 객체들로 이루어진 트리를 지칭합니다.
### 1.1. `task_struct`
`task_struct` 구조체에는 매우 많은 수의 field가 존재합니다. 여기서는 그 중에서 이번 프로젝트에서 사용한 field들에 대해서만 간략하게 설명하고자 합니다.
``` C
struct task_struct {
  volatile long state;
  pid_t pid;
  struct task_struct __rcu *parent;
  struct list_head children;
  struct list_head sibling;
  const struct cred __rcu *cred;
  char comm[TASK_COMM_LEN];     // #define TASK_COMM_LEN 16
}
```
저희 조의 구현에서 활용한 필드만 나열한 `task_struct`의 구조입니다.  
이 중 `state`와 `pid`, `comm`은 그대로 `prinfo`의 `field`로 복사됩니다.  
`struct cred`는 `uid`를 `field`를 가지고 있으므로 `cred->uid`를 `prinfo`로 복사해줍니다.  
`parent`의 경우도 간단하게 `parent->pid`를 복사해주면 됩니다.  
남는 것은 `struct list_head` 타입의 두 필드입니다. 이에 대해서는 다음 절에서 설명하겠습니다.
### 1.2. `list_head`와 tasklist의 구조
Help documents와 reference에서 언급된 것과 같이, `list_head`는 doubly linked list를 구현합니다.  
특이하게 이 구조체는 다른 구조체 A의 내부에 들어가서 A의 요소들을 연결해주는 역할을 합니다. 따라서 `list_head`로부터 그것을 싸고 있는 구조체 A에 접근할 방법이 필요합니다. 그것을 제공하는 것이 `list_entry(ptr, type, field_name)`입니다. `ptr`에 `list_head *` 타입의 포인터를 넘겨주고 type에 `*ptr`를 field로 포함하고 있는 구조체 A의 타입을, `field_name`에 그 구조체가 `*ptr`를 어떤 이름의 field로 가지고 있는지 그 이름을 넘겨주면 `list_entry`는 `*ptr`를 필드로 가지고 있는 구조체 객체의 포인터를 반환합니다.

`struct list_head children`은 그 이름 그대로 자신의 자식 프로세스들에 대한 `task_struct` 구조체들의 리스트를 가리키고 있습니다.  
즉 `struct task_struct A0`와 `A0`가 나타내는 프로세스의 자식 프로세스들에 대한 `task_struct`구조체 `struct task_struct B0`, `struct task_struct B1`, ... , `struct task_struct Bn`이 있다고 할 때, `A0.children`, `B0.sibling`, `B1.sibling`, ... , `Bn.sibling`이 doubly-linked list를 이루고 있다는 것입니다. <br/>
이와 같이 tasklist는 process tree를 여러 개의 doubly-linked list로 구현하고 있습니다.

## 2. `sys_ptree`

### 2.1. High level design

기본적인 아이디어는 `extern struct task_struct init_task`를 전위순회(preorder traversal)하는 재귀함수를 정의하는 것입니다. 보통 노드 r에서 시작하는 전위순회는<br/>(1) r 방문<br/>(2) r의 각 자식에 대해 그 노드에서 시작하는 전위순회를 순서대로 실행<br/>의 방식으로 구현됩니다.

그러나 여기서는 약간 변형된 방식을 사용하였는데, 그 방법은 다음과 같습니다.<br/>(1) r 방문<br/>(2) r의 첫번째 자식에 대해 전위순회<br/>(3) (존재할 경우) r의 다음 형제(next sibling)에 대해 전위순회

두 방식이 실질적으로 같다는 것은 트리의 규모에 대한 귀납법으로 쉽게 증명할 수 있으니 그 증명을 직접 서술하지는 않겠습니다.

여기서 두 번째 방식을 사용한 이유는 tasklist의 구조와 관련이 되어 있습니다. 부모 노드에 바로 연결되어 있는 것은 첫번째 자식뿐이고, 그 다음 자식들은 첫번째 자식에 순서대로 연결되어 있기 때문에, 일반적으로 자주 쓰이는 첫번째 방법에 비해 두번째 방법이 더 효율적입니다.

위와 같은 방식으로 순회하면서 `task_struct` 구조체의 정보를 `prinfo` 구조체로 옮겨주고, 그것을 user space에 복사해주는 것이 `sys_ptree`의 역할입니다.

### 2.2. Implementation
1. user space와 kernel space의 메모리주소가 다르니, kernel에서 user space에 있는 데이터를 바로 접근할 수 없습니다. 따라서 user space에 있는 data를 kernel space로 copy 해줘야 합니다.
이 때 사용하는 함수가 copy_from_user이고, 그 반대 과정이 copy_to_user입니다.

2. nr 데이터의 경우 sys_ptree내에서 nr값을 kernel spaxe에서 선언된 max_count_process에 집어넣어 사용할것이기 때문에 copy_from_user를 사용한 반면, sys_ptree의 input으로 들어온 buf의 경우엔 data를 copy할 필요 없이 해당 유저 영역 주소에 access 가능한지만 확인하면 되기 때문에 access_ok 함수를 사용하였습니다.
	
3. kernel_buf에 담긴 data를 유저영역에 있는 buf에 다시 copy해주기 위해 copy_to_user를 사용합니다


``` C
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
```



## 3. Build
``` C
# krnel/ptree.c와 test.c 파일을 수정한 후 kernel 이미지를 rebuild 합니다
$ sudo ./build-rpi3-arm64.sh && sudo ./scripts/mkbootimg_rpi3.sh

# 새로 생긴 두 이미지 boot.img, modules.img를 tizen image들이 모여 있는 상위 디렉토리 tizen-image 디렉토리 안으로 옮깁니다
$ mv *.img ../tiaen-image/

# test.c 파일을 compile합니다
$ arm-linux-gnueabi-gcc -I$(pwd)/include ./test/test_ptree.c -o ./test/test

# tizen-image/rootfs.img를 mount할 directory를 생성한 뒤 mount 해줍니다.
# test 실행파일을 mount directory root 안으로 옮겨줍니다
$ sudo mount -v -o loop ${tizen-image directory}/rootfs.img ${mnt_dir}
$ sudo mv osfall2021-team8/test/test ${mnt_dir}/root/
$ sudo umount ${mnt_dir}

# Qemu를 실행하여 tiaen shell에 접속한 뒤 ./test 파일을 실행합니다
$ cd ${osfall2021-team8 directory}
$ sudo ./qemu.sh

(tizen shell)
root:~> ./test
```

## 4. Test
test 실행파일을 실행할 때 nr값을 argument로 받습니다. 인자를 넣지 않을 경우, default로 설정된 nr값으로 sys_ptree를 호출합니다
```C
root:~> ./test

Total number of process: 92
swapper/0,0,0,0,1,0,0
  systemd,1,1,0,140,2,0
    systemd-journal,140,1,1,0,164,0
    dbus-daemon,164,1,1,0,182,81
    .
    .
    .
    storaged,340,1,1,0,268,0
  kthreadd,2,1,0,3,0,0
    kworker/0:0,3,1026,2,0,4,0
    kworker/0:0H,4,1026,2,0,5,0
    .
    .
    .
    kworker/2:3,154,1026,2,0,0,0
```

```C
root:~> ./test 4

Total number of process: 4
swapper/0,0,0,0,1,0,0
  systemd,1,1,0,140,2,0
    systemd-journal,140,1,1,0,164,0
    dbus-daemon,164,1,1,0,182,81
```

## 5. Lessons learned
- system call을 어떻게 추가하고 어디에 추가가 되는지 알수 있었다.
- system call을 할 때 유저 영역과 커널 영역간의 데이터 복사가 일어나야 된다는 점을 공부하면서, 왜 file open() read()같은 system call들이 느린 작업이라는 것인지 알 수 있었다.
- 모든 프로세스들이 task_struct라는 struct로 다 연결돼 있다는 점, 부팅하는 순간 swapper/0 process가 실행되어 root 역할을 한다는 점이 신기하였다.
- 부모 프로세스의 childeren의 next가 자식 프로세스의 sibling으로 연결된다는 점이 가장 헷갈렸다. 처음에는 왜 자식 프로세스의 children으로 연결 된 것이 아니라 sibling으로 연결 되는 것인지 의아 하였다. 하지만 프로세스들이 어떻게 연결돼 있을지 그림을 그려보니 sibling으로 연결되는 것이 더 직관적이고 간편하게 구현할 수 있다는 것을 알 수 있었다.

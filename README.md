# [Project 4] Team 8 implementation document
## 1. Data Structure
### 1.1 gps_location
- inlude/linux/gps.h에 정의
- gps 위치 information을 담은 구조체로 각 위도, 경도의 정수, 소수 파트를 field로 가지고, accuracy도 field로 갖는다
```C
struct gps_location {
	int lat_integer;
	int lat_fractional;
	int lng_integer;
	int lng_fractional;
	int accuracy;
};
```
### 1.2 Customized floating point
- kernel/gps.c에 정의 : kernel에서는 floating 타입이 없기 때문에 연산을 위해 아래같이 구조체를 만들어 아래의 구조체가 float라 생각한다
- 아래 구조체에 대한 operation 들을 정의해줘야 gps_float 구조체 간에 연산이 가능해진다.
```C
struct gps_float {
	long long integer;
	long long decimal;
};
```

### 1.3 ext2_inode, ext2_inode_info 구조체
- fs/ext2/ext2.h 에 정의
- 두 구조체 다 indode에 어떤 정보들이 들어가야 하는지 정의 돼 있다. 우리는 이제 파일에 대해 파일이 만들어질 때의 gps location 정보도 추가할 것이기 때문에 이 구조체들에 추가해줘야 한다

```C
struct ext2_inode {
 ... 생략 ...
 __le32	i_lat_integer;
 __le32	i_lat_fractional;
 __le32	i_lng_integer;
 __le32	i_lng_fractional;
 __le32	i_accuracy;
};

struct ext2_inode_info {
 ... 생략 ...
 __u32	i_lat_integer;
 __u32	i_lat_fractional;
 __u32	i_lng_integer;
 __u32	i_lng_fractional;
 __u32	i_accuracy;
};
```

### 1.4 struct file_operations ext2_file_operations
- fs/ext2/file.c 에 정의
- file_operations 구조체는 각 파일시스템별로 사용할 operations 구조체이며, 파일이 생성되고 수정되고 삭제되고 등등 파일과 관련된 작업에서, 파일이 속해 있는 파일 시스템에 맞는 operation들을 호출하게 되는데 이 operation들을 이 구조체 필드에 정의해준다
- proj2 scheduler에서 했던 작업과 비슷하다
- gps 위치를 set, get 할수 있는 method, file permission에 대한 mehtod를 추가해준다. 이 method들은 fs/ext2/inode.c에서 구현한다
```C
const struct file_operations ext2_file_operations = {
 ... 생략 ...
 .set_gps_location = ext2_set_gps_location,
 .get_gps_location = ext2_get_gps_location,
 .permission	= ext2_permission,
};
```
## 2. Implementation
- 구현 순서에 맞춰 sys_set_gps_location -> ext2_set_gps_location -> ext2_get_gps_location -> sys_get_gps_location 순서로 설명한다
### 2.1 sys_set_gps_location 
- 시스템 콜 추가하는 방식은 생략한다.
- 현재 커널의 current gps locaiton을 set 하는 시스템 콜이다. 실행환경에서 가장 이 시스템콜을 호출하는 ./gpsupdate 실행파일을 가장 먼저 실행시킨다.
- shared variable curr_loc을 위해 mutex를 설정해준다.
- user 영역의 gps_location 구조체를 커널 영역으로 copy 해주고, mutex를 잡은 뒤 curr_loc에 각각의 필드의 값들을 넣어준다


### 2.2 ext2_set_gps_location, ext2_get_gps_location
- fs/ext2/inode.c에 구현

```C
int ext2_set_gps_location(struct inode *inode);
int ext2_get_gps_location(struct inode *inode, struct gps_location *loc);
```
- ext2_set_gps_location은 input inode에 current gps location 정보를 copy 해준다. 이때도 mutex를 사용해준다. 파일이 생성되고, 수정될 때마다 이 함수가 호출되어야 한다.
- 파일이 생성될 때를 위해, fs/ext2/namei.c ext2_create 함수 안에서, 파일이 수정될 때를 위해 fs/attr.c notify_change 함수 안에서 호출해준다. 각각의 함수 내에서 아래와 같이 호출해주면 된다.

```C
if (inode->i_op->set_gps_location)
		error = inode->i_op->set_gps_location(inode);
```
- ext2_get_gps_locationd은 parameter gps_location 구조체 bufffer에 원하는 파일(inode)에서 gps location 정보를 copy 해준다. 이 함수는 sys_get_gps_location 시스템콜 함수에서 호출될 예정이다.

- 추가로 중요한 부분은, inode object를 가져와주는 ext2_iget 메소드와 inode를 write해주는 __ext2_write_inode 수정해줘야 된다는 점이다. big endian, little endian 어느 환경에서든 데이터가 제대로 읽힐 수 있게 처리해주는게 중요하다
- inode를 읽을 떄는 le32_to_cpu 함수를 호출해주고, indoe를 write 할 때는 cpu_to_le32를 사용해준다. 우리가 ext_inode 구조체에서 location 관련 field를 le32로 정의해줬기 떄문이다
```C
ex) ei->i_lat_integer = le32_to_cpu(raw_inode->i_lat_integer); // In ext2_iget
ex) raw_inode->i_lat_integer = cpu_to_le32(ei->i_lat_integer); // In __ext2_write_inode
```

### 2.3 ext2_permission, sys_get_gps_location, 
- 파일의 gps location을 get 하는 시스템 콜을 구현한다. 이 함수 안에서 파일에 접근할 수 있는지 permission 체크를 해줘야 한다. 이 시스템 콜 함수에서 inode_permission 함수를 호출할 것인데 이 함수에서, 위에서 ext2_file_operations 구조체에 추가해준 .permission 필드에 해당하는 ext_permission 함수를 호출한다.
- inode_permission (fs/namei.c) -> __inode_permision (fs/namei.c) -> do_inode_permission (fs/namei.c) -> ext2_permission (fs/ext2/inode.c)
```C
static inline int do_inode_permission(struct inode *inode, int mask)
{
	if (unlikely(!(inode->i_opflags & IOP_FASTPERM))) {
		if (likely(inode->i_op->permission))
			return inode->i_op->permission(inode, mask);

		/* This gets set once for the inode lifetime */
		spin_lock(&inode->i_lock);
		inode->i_opflags |= IOP_FASTPERM;
		spin_unlock(&inode->i_lock);
	}
	return generic_permission(inode, mask);
}
```
- gps_distance_permission 함수는 현재 커널의 gps location과 파일의 gps location이 가까운지 아닌지 계산해주는 함수이다. 이 함수에 대한 자세한 설명은 뒤에서 설명한다.
```C
int ext2_permission(struct inode *inode, int mask)
{
	if(gps_distance_permission(inode) < 0)
		return -EACCES;
	return generic_permission(inode, mask);
}
```
- sys_get_gps_location 에서는 path를 통해 file inode object를 가져오고, inode_permission을 호출하여 gps location을 계산한뒤 permission을 통과하면 ext_get_gps_location을 통해 get 해온 location 정보를 user space input parameter로 copy_to_user 해준다. 
```C
long sys_get_gps_location(const char __user *pathname, struct gps_location __user *loc)
{
	... 생략 ...
 
	error = user_path_at(AT_FDCWD, pathname, lookup_flags, &path);
	if (error)
		return error;
	inode = path.dentry->d_inode;

	if (inode_permission(inode, MAY_READ)) {
		printk("Permission Denied\n");
		return -EACCES;
	}
	
	if (inode->i_op->get_gps_location) {
		inode->i_op->get_gps_location(inode, &kernel_buf);
	} else {
		return -EOPNOTSUPP;
	}
 
	if(copy_to_user(loc, &kernel_buf, sizeof(struct gps_location) != 0))
		return -EFAULT;
	return 0;
}
```
- gps_haversine_distance 함수를 통해, file location과 kernel current location의 거리를 구한뒤 이 거리가 각각의 accuracy의 합보다 작으면 permission을 통과시키고, 그렇지 않다면 permission denied가 된다.
```C
/// Permission denied: return -1, Succeed: return 0
int gps_distance_permission(struct inode *inode)
{
	struct gps_location file_loc;
	struct gps_float distance, accuracy;
	if (inode->i_op->get_gps_location) {
		inode->i_op->get_gps_location(inode, &file_loc);
	}
	distance = gps_haversine_distance(file_loc);
	set_gps_float(&accuracy, curr_loc.accuracy + file_loc.accuracy, 0);

	if (comp_gps_float(&accuracy, &distance) >= 0) {
		return 0;
	} else {
		return -1;
	}
}
```

### 2.4 Arimetric Operations & Trigonometric Functions
- add, sub, mult, div를 구현해준다. gps_float 구조체 방식의 문제점은 -1과 0 사이의 음수를 다를 수 없다는 점이다. -0 = 0 이기 때문에 -0.xxx를 표현할 방법이 없다. 따라서 이는 연산에 있어서 큰 문제가 된다. 처음에는 이 음수들을 다 고려해서 구현을 하였지만, 결국 이 함수들을 호출하는 곳에서 input이 -1에서 0 사이의 음수인지 확인을 해줘야 된다는 점 때문에, 오히려 복잡해진 다는 것을 꺠달았다. 따라서 이 기본 사칙 연산은 무조건 input 값들이 양수라고 가정하고 구현하였고, 이 함수를 호출하는 곳에서 input이 음수인지 양수인지를 handling 해주기로 하였다. 또한 sub를 호출할 때는 반드시 첫번째 paramenter가 두번째 parameter 보다 반드시 크다는 가정을 하였다.
- factorial, power, degree2rad를 구현해준다.
- cos, sin, arccos 삼각함수를 구현한다. 이 삼각함수를 쉽게 구현하기 위해, global variable로, gps_float type의 pi, pi/180, 1, 0, 1/2를 선언해주었다.
- 삼각함수들은 talyor series를 통한 근사식을 구현해준다. arccos의 경우 x가 1에 가까우면 실제 값과 근사값의 오차가 여전히 크게 나타난다. 또 이 최종 arccos 값이 지구의 반지름과 곱해지는 결과값이 두 점사이의 거리이기 때문에, 오차가 굉장히 중요하다. 그래서 최대한 많은 항까지 고려해주려 하였다. 하지만 8번째 항부터는 계산에서 overflow가 나기 떄문에 8번째 항부터는 x^(2n+1) 에 곱해줘야 하는 계수들을 직접 계산해줘서 코드에 직접 집어 넣었다. 이를 통해 48개의 항까지 계산하였다. 하지만 그럼에도 x >= 0.995 이상인 숫자들에 대해선 너무 큰 오차가 났다
 

![image](https://user-images.githubusercontent.com/60849888/144764184-6d21cda3-d074-48a4-9e84-9c229a784959.png)

![image](https://user-images.githubusercontent.com/60849888/144764067-0f8f4e3a-a5bc-4804-913a-39b7fd4574e1.png)

- 삼각함수 구현부를 보면 아래와 같은 모습들을 많이 찾을 수 있다. 삼각함수 연산에서는 -1과 0사이의 값이 계속 나타나고 위에서 설명하였듯이 이것을 handling 하는 것이 중요하다. 또한 sub 함수에서 반드시 lvalue가 rvalue보다 크다는 강제조항이 있기 때문에 아래와 같이 계속 handling 해줘야 한다. 
```C
term = div_gps_float(&power_tmp, &factorial_tmp);		// x^3/3!
if (comp_gps_float(&ret, &term) >= 0) {
		ret = sub_gps_float(&ret, &term);			// x-x^3/3!
  ... 생략 ...
} else if(comp_gps_float(&ret, &term) == -1) {
		ret = sub_gps_float(&term, &ret);
  ... 생략 ...
```


### 2.5 Haversine distance
- 구면에서 두 점의 latitude, longtitude를 통해 distance를 구하는 공식으로 아래를 참고하였다. 파이썬 라이브러리 코드를 확인할 수 있는데, 아래 로직을 그대로 kernel/gps.c에 구현했다.
- 여기서 중요한 점은, 우리는 square root가 구현돼 있지 않기 떄문에, arcsin 대신 arccos을 사용해준다. 공식의 변환은 아래의 수식을 통해 확인할 수 있다. 
- ref: https://en.wikipedia.org/wiki/Haversine_formula
- ref: python haversine package function
```C
# python logic
lat1, lng1 = point1
lat2, lng2 = point2

# convert all latitudes/longitudes from decimal degrees to radians
lat1 = radians(lat1)
lng1 = radians(lng1)
lat2 = radians(lat2)
lng2 = radians(lng2)

# calculate haversine
lat = lat2 - lat1
lng = lng2 - lng1
harv_d = sin(lat * 0.5) ** 2 + cos(lat1) * cos(lat2) * sin(lng * 0.5) ** 2

return 2 * get_avg_earth_radius(unit) * asin(sqrt(harv_d))
```
- 기본 logic은 위 python code를 따르지만 arccos을 사용하기 위해 아래와 같은 식을 kernel/gps.c gps_haversine_distance 메소드로 그대로 구현했다. 
![image](https://user-images.githubusercontent.com/60849888/144707258-2aab5d37-07ef-418e-8325-22c91e2e9846.png)

### 2.6 Mount our customized file system
- proj4.fs를 tizen kernel 안에서 mount해줘야 하는데, loop 문제로 잘 안되는 경우들이 있다. 이 문제를 해결하기 위해, qemu.sh command를 수정해주고. fstab 파일을 작성해줘서 이 파일로 원래 tizen image 의 fstab 파일을 교체해준다.
```C
% sudo cp ./my_fstab ${MNT_DIR}/etc/fstab
```

## 3. Test
- Test의 편의를 위해, 미리 서울대학교 공과대학 302동 위도 경도, 301동 위도 경도, 부산역의 위도 경도가 적힌 shell script 파일들을 준비하였다. 
- 원래는 서울대학교 미술관의 위치를 받아, 302동과 미술관의 거리가 멀어서 permission denied를 기대했지만, 삼각함수를 거치면서 같은 값으로 이르게 되어 거리차이가 0으로 계산되었다. 근사식의 오차 때문이다. 그래서 부산역의 위치를 잡고 테스트 하였다.
```C
root:~> bash setgps_snu302.sh
./gpsupdate 37 448743 126 951502 50

root:~> touch proj4/myfile

root:~> bash setgps_snu301.sh
./gpsupdate 37 450049 126 951409 50
root:~> ./file_loc proj4/myfile
File proj4/myfile location
=> latitude: 37.448743
=> longtitude: 126.951502
=> accuracy : 50

Google Maps Link: https://www.google.co.kr/maps/search/37.448743+126.951502

root:~> bash setgps_busan.sh
./gpsupdate 37 448743 126 951502 50
root:~> ./file_loc proj4/myfile
Too far distance between two points: 604951.934
Permission Denied
sys_get_gps_location failed
```
- 위 결과를 보아 알 수 있듯이, 302동에서 만든 파일을 301동으로 위치를 바꿔서 파일에 접근할때는 파일의 gps location과 gps google man link가 잘 출력되는걸 확인 할 수 있다.
- 부산으로 커널의 위치를 옮겼을 때 파일에 접근하게 되면, Permission Denied가 출력되는 것을 확인할 수 있다.
